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

#include "acme/socket.h"

#include "acme/common.h"

#include <array>
#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <optional>
#include <span>
#include <string_view>

#include <netinet/in.h>

#include <fmt/printf.h>

namespace {
  using namespace std::string_literals;

  struct config {
    in_port_t port_number;
  };

  enum class animal { chicken, cow, horse, zebra };

  struct message {
    animal a;
  };

  auto parse_command_line(std::span<char const* const> args)
  {
    constexpr auto expected_size = 1;
    if (auto actual_size = args.size() - 1; actual_size != expected_size) {
      acme::error("expected {} command-line parameters; got {}", expected_size, actual_size);
      return std::optional<config>{};
    }

    auto const* port_string = args[1];
    auto const* port_string_end = std::strchr(port_string, '\0');
    in_port_t port_number;
    if (auto [p, ec] = std::from_chars(port_string, port_string_end, port_number);
        ec != std::errc() || p != port_string_end) {
      acme::error("failed to parse '{}' as port number.", port_string);
      return std::optional<config>{};
    }

    return std::make_optional(config{port_number});
  }

  template <typename Destination>
  auto deserialize(std::span<std::byte> const bytes)
  {
    constexpr auto destination_size = sizeof(Destination);

    auto const packet_size = std::ssize(bytes);
    if (packet_size != destination_size) {
      acme::warn("invalid packet size. expected={}; actual={}", destination_size, packet_size);
      return std::optional<Destination>{};
    }

    Destination destination;
    std::memcpy(&destination, bytes.data(), destination_size);

    return std::make_optional(destination);
  }

  auto parse_message(std::span<std::byte> const network_packet)
  {
    struct payload {
      std::int8_t number;
    };

    auto const p = deserialize<payload>(network_packet);
    if (!p) {
      return std::optional<message>{};
    }

    auto a = animal(p->number);
    switch (a) {
      default:
        acme::warn("invalid packet contents, {}", int(a));
        return std::optional<message>{};

      case animal::zebra:
      case animal::chicken:
      case animal::cow:
      case animal::horse:
        return std::make_optional(message{a});
    }
  }

  auto create_socket(config const& config)
  {
    auto udp_socket = acme::socket(acme::domain::inet, acme::type::dgram);
    if (!udp_socket.open()) {
      acme::error("failed to create socket: {}", strerror(errno));
      errno = 0;

      return std::optional<acme::socket>{};
    }

    if (!udp_socket.bind(config.port_number)) {
      acme::error("failed to bind socket on port {}: {}", config.port_number, strerror(errno));
      errno = 0;

      return std::optional<acme::socket>{};
    }

    return std::make_optional(std::move(udp_socket));
  }

  auto process_message(message const m)
  {
    ACME_ASSERT(m.a == animal::chicken || m.a == animal::cow || m.a == animal::horse || m.a == animal::zebra);

    auto const animals = std::array<std::string_view, 4>{"chicken", "cow", "horse", "zebra"};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    fmt::print("{}\n", animals[int(m.a)]);
  }

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
  auto const config = parse_command_line(std::span(argv, argc));
  if (!config) {
    return EXIT_FAILURE;
  }

  auto udp_socket = create_socket(*config);
  if (!udp_socket) {
    return EXIT_FAILURE;
  }

  run(std::move(*udp_socket));
}
