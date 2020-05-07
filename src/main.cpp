// Copyright 2020 John McFarlane
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// @file An example of a real-time safety-critical error strategy.
/// @note Please read accompanying comments for explanations...

#include "acme/socket.h"
#include "acme/common.h"

#include <fmt/printf.h>

#include <netinet/in.h>

#include <array>
#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <optional>
#include <span>
#include <string_view>

namespace {
  using namespace std::string_literals;

  /// @brief configuration information passed to the program at startup
  struct config {
    in_port_t port_number;
  };

  enum class animal { chicken, cow, horse, zebra };

  /// @brief network message passed to the program at run time
  struct message {
    animal a;
  };

  /// @brief parses command line and return valid program configuration
  /// @param program arguments as passed to main
  /// @return verified configuration information iff arguments satisfy program requirements
  auto parse_command_line(std::span<char const* const> args) -> std::optional<config>
  {
    // Verify that the correct number of arguments was passed to the program.
    constexpr auto expected_size = 1;
    if (auto actual_size = args.size() - 1; actual_size != expected_size) {
      // tip: This is a user error which should be entirely avoidable so long as whoever is running the program receives
      // adequate diagnostic information.
      acme::error("expected {} command-line parameters; got {}", expected_size, actual_size);
      return {};
    }

    // Verify that the program argument was a port number.
    auto const* port_string = args[1];
    auto const* port_string_end = std::strchr(port_string, '\0');
    in_port_t port_number;
    if (auto [p, ec] = std::from_chars(port_string, port_string_end, port_number);
        ec != std::errc() || p != port_string_end) {
      // tip: This is a user error which should be entirely avoidable so long as whoever is running the program receives
      // adequate diagnostic information.
      acme::error("failed to parse '{}' as port number.", port_string);
      return {};
    }

    // tip: The function has verified the program arguments and can now return valid configuration information.
    return config{port_number};
  }

  /// @brief returns raw memory as object of Destination type
  /// @tparam Destination type of object to return
  /// @param bytes the raw memory that contains the object to be returned
  /// @return the memory as an object of type, Destination, iff bytes is the same size
  template <typename Destination>
  auto deserialize(std::span<std::byte> const bytes) -> std::optional<Destination>
  {
    constexpr auto destination_size = sizeof(Destination);

    auto const packet_size = std::ssize(bytes);
    if (packet_size != destination_size) {
      // tip: This error means that we cannot proceed with the packet that was received.
      // However, there's no reason to believe that the program is in a bad state:
      // the presumption is that the problem lies with the sender.
      // So we emit a diagnostic, stop work on this packet and continue.
      acme::warn("invalid packet size. expected={}; actual={}", destination_size, packet_size);
      return {};
    }

    Destination destination;

    // We deliberately use destination_size, not packet_size here.
    // Even though they are equal, it's the size of destination that is correct.
    std::memcpy(&destination, bytes.data(), destination_size);

    return destination;
  }

  /// @brief converts raw network packet to message
  /// @param network_packet raw bytes received over network connection
  /// @return message received in packet iff valid
  auto parse_message(std::span<std::byte> const network_packet) -> std::optional<message>
  {
    // a trivial type capable of representing the network data
    struct payload {
      std::int8_t number;
    };

    auto const p = deserialize<payload>(network_packet);
    if (!p) {
      // tip: Explicit error propagation is not optimal on the happy path.
      // But for real-time or public-facing applications, the cost of exception throwing is a consideration.
      return {};
    }

    auto a = animal(p->number);

    // tip: Validation of run-time input happens here.
    switch (a) {
      default:
        acme::warn("invalid packet contents, {}", int(a));
        return {};

      case animal::zebra:
      case animal::chicken:
      case animal::cow:
      case animal::horse:
        // tip: Any failure after this point is assumed to be a bug or exhaustion of the abstract machine,
        // rather than a recoverable error.
        return std::make_optional(message{a});
    }
  }

  /// @brief creates a UDP socket, given a program configuration
  /// @param config program configuration
  /// @return bound socket object iff no error occurred
  auto create_socket(config const& config) -> std::optional<acme::socket>
  {
    auto udp_socket = acme::socket(acme::domain::inet, acme::type::dgram);
    if (!udp_socket.open()) {
      // tip: Provide clear concise help to the user when something goes wrong.
      acme::error("failed to create socket: {}", strerror(errno));
      errno = 0;

      // tip: Don't be afraid to fail fast in a function.
      return {};
    }

    if (!udp_socket.bind(config.port_number)) {
      acme::error("failed to bind socket on port {}: {}", config.port_number, strerror(errno));
      errno = 0;

      // tip: If there are multiple failure cases, multiple return statements are a Good Thing in C++.
      return {};
    }

    // tip: Most functions have zero or more failure cases, ending with one success case.
    return udp_socket;
  }

  // @brief respond to a message
  // @param m the message
  auto process_message(message const m)
  {
    // tip: Unless otherwise stated, all state in a program is assumed to be valid.
    // But it rarely hurts to test assumptions and helps ensure correctness of calling code.
    ACME_ASSERT(m.a == animal::chicken || m.a == animal::cow || m.a == animal::horse || m.a == animal::zebra);

    auto const animals = std::array<std::string_view, 4>{"chicken", "cow", "horse", "zebra"};

    // tip: It follows that we can trust potentially unsafe operations.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    fmt::print("{}\n", animals[int(m.a)]);
  }

  // @brief responds to network messages on the given socket
  // @param udp_socket handle to a bound UDP/IP socket
  [[noreturn]] auto run(acme::socket udp_socket)
  {
    acme::info("entering main loop");

    while (true) {
      constexpr auto max_buffer_size = 2;
      std::array<std::byte, max_buffer_size> buffer{};

      auto const unsanitized_packet = udp_socket.read(buffer);
      if (!unsanitized_packet) {
        // This is a fatal error. The program is unlikely to function following this failure.
        acme::error("failed to read udp packet: {}", strerror(errno));

        // Because this fatal error happens within the main loop, the program should quit swiftly.
        // Because this is a performance-critical application, std::terminate is chosen.
        std::terminate();
      }

      auto const sanitized_message = parse_message(*unsanitized_packet);
      if (!sanitized_message) {
        // This is a non-fatal error. The contract between the program and its user has been violated but the program
        // can continue.
        continue;
      }

      process_message(*sanitized_message);
    }
  }
}  // namespace

auto main(int argc, char const* const* const argv) -> int
{
  // tip: Validation of pre-run-time input happens here.
  auto const config = parse_command_line(std::span(argv, argc));
  if (!config) {
    return EXIT_FAILURE;
  }

  auto udp_socket = create_socket(*config);
  if (!udp_socket) {
    return EXIT_FAILURE;
  }

  // tip: Any recoverable errors occuring after this point are assumed to be the result of invalid network input.
  run(std::move(*udp_socket));
}
