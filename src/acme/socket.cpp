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

#include "socket.h"

#include "common.h"

#include <cerrno>
#include <cstring>

using acme::socket;

socket::socket(domain d, type t, int protocol)
    : fd{::socket(int(d), int(t), protocol)}
{
}

socket::socket(socket&& other) noexcept
{
  std::swap(fd, other.fd);
}

socket::~socket()
{
  if (!open()) {
    return;
  }

  ACME_ASSERT(!errno);
  auto result = ::close(fd);
  ACME_ASSERT(result == 0);
}

auto socket::operator=(socket&& other) noexcept -> socket&
{
  std::swap(fd, other.fd);
  return *this;
}

[[nodiscard]] auto socket::open() const -> bool
{
  return fd != uninitialized;
}

[[nodiscard]] auto socket::bind(in_port_t port_number, in_addr_t address) const -> bool
{
  ACME_ASSERT(open());

  sockaddr_in sin_addr{sa_family_t{int(domain::inet)}, htons(port_number), in_addr{address}, {}};
  sockaddr addr{};
  static_assert(sizeof(sin_addr) == sizeof(addr));
  std::memcpy(&addr, &sin_addr, sizeof(addr));

  ACME_ASSERT(!errno);
  auto result = ::bind(fd, &addr, sizeof(addr));
  if (result == 0) {
    return true;
  }

  ACME_ASSERT(result == -1);
  return false;
}

auto socket::read(std::span<std::byte> const buffer) const -> std::optional<std::span<std::byte>>
{
  // buffer must be zero or more bytes, i.e. a null pointer won't do.
  ACME_ASSERT(buffer.data());

  ACME_ASSERT(open());

  ACME_ASSERT(!errno);
  auto const size = ::read(fd, buffer.data(), buffer.size());

  // It's fine to be over-cautious with contract checks.
  // You may want to test your assumptions and express the state of the program.
  // But according to the contract of ::read, none of these violations can possibly happen.
  ACME_ASSERT(size == -1 || (size >= 0 && size <= std::ssize(buffer)));

  if (size == -1) {
    // This is a run-time error which could mean that the network interface is in a bad state.
    // There may be little that the program can do to recover from this error.
    return std::nullopt;
  }

  return buffer.subspan(0, size);
}
