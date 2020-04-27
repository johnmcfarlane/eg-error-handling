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

#include <cstddef>
#include <optional>
#include <utility>

#include <netinet/ip.h>
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace acme {
  enum class domain { inet = AF_INET };
  enum class type { stream = SOCK_STREAM, dgram = SOCK_DGRAM, raw = SOCK_RAW };

  class socket;
  [[nodiscard]] auto operator==(socket const& a, socket const& b) -> bool;
  [[nodiscard]] auto operator!=(socket const& a, socket const& b) -> bool;

  /// @brief partial wrapper over
  /// @note There is no logging occuring in the implementation of this API.
  ///       This is because it is a reusable component. We don't know where it might be used.
  ///       Imposing a logging solution here might restrict its applicability.
  ///       And in a few cases, it might even lead to DoS attacks if logging calls
  ///       eat up storage and invoke expensive system calls.
  /// @sa [When the hot loop’s done: Four quick tips for high
  /// throughput](https://herbsutter.com/2020/04/01/when-the-hot-loops-done/)
  class socket {
  public:
    socket() noexcept = default;
    explicit socket(domain d, type t, int protocol = 0);
    socket(socket const&) = delete;
    socket(socket&& other) noexcept;

    ~socket();

    auto operator=(socket const&) -> socket& = delete;
    auto operator=(socket&& other) noexcept -> socket&;

    /// @return true iff the socket is created
    [[nodiscard]] auto open() const -> bool;

    /// @return true if successful; on failure, errno may be set
    /// @precondition `open() == true`
    [[nodiscard]] auto bind(in_port_t port_number, in_addr_t address = INADDR_ANY) const -> bool;

    /// @brief receives a network packet
    /// @param buffer points to zero or more bytes into which the incoming packet is stored
    /// @precondition `open() == true`
    /// @return the packet received, or as much as could fit in buffer, or a default-initialized span on error
    auto read(std::span<std::byte> const buffer) const -> std::optional<std::span<std::byte>>;

  private:
    static constexpr int uninitialized = -1;
    int fd{uninitialized};
  };
}  // namespace acme
