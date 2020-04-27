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

#include <fmt/format.h>
#include <fmt/printf.h>

#include <utility>

#define ACME_UNREACHABLE() __builtin_unreachable()

#define ACME_ASSERT(condition) ((condition) ? static_cast<void>(0) : ACME_UNREACHABLE())

namespace acme {
  /// @brief helper for logging functions
  /// @sa info, warn, error, fatal
  template <typename... Args>
  void log(FILE* out, std::string_view tag, std::string_view format_str, Args&&... args)
  {
    fmt::print(out, "{}: {}\n", tag, fmt::format(format_str, std::forward<Args>(args)...));
  }

  template <typename... Args>
  void info(Args&&... args)
  {
    log(stderr, "info", std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warn(Args&&... args)
  {
    log(stderr, "warning", std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(Args&&... args)
  {
    log(stderr, "error", std::forward<Args>(args)...);
  }
}  // namespace acme
