// Copyright 2021 John McFarlane
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

/// @file An example of a robust C++ program.
/// @note Please read accompanying comments for explanations...

#include <charconv>
#include <cstdlib>
#include <exception>
#include <span>
#include <string_view>

#include <fmt/printf.h>

/// @brief A minimal assertion function for testing API contracts.
/// @note This function lacks diagnostics and may not be suitable
///       for defective or safety-critical applications.
constexpr void eg_assert(bool condition)
{
  if (condition) {
    return;
  }

#if defined(LOG_AND_CONTINUE_STRATEGY)
  fmt::print(stderr, "a C++ API violation occurred\n");
#elif defined(TRAP_STRATEGY)
  std::terminate();
#elif defined(PREVENTION_STRATEGY)
  __builtin_unreachable();
#else
#error missing strategy pre-processor definition
#endif
}

constexpr auto min_number{1};
constexpr auto max_number{26};

/// @brief The letter at the given position in the English alphabet
/// @param number the position of the letter in the alphabet
/// @return the letter at that give position as uppercase
/// @note The position of the first letter, 'A', is 1
/// @note It can be implied from this description
///       that values <1 or >26 violate the contract of this API.
///       Regardless of the assertions within the function,
///       a program in which the contract is violates
///       should be considered to exhibit undefined behavior.
///       However, it rarely hurts to clarify contracts...
/// @pre  number is in range [1..26]
constexpr auto number_to_letter(int number)
{
  // Assertions - and not logical checks - are appropriate here.
  // They are here to help analysis tools, such as UBSan, detect bugs.
  // They can also server as documentation.
  eg_assert(number >= min_number);
  eg_assert(number <= max_number);

  // Just because we can reason about the behavior of
  // this implementation of the function doesn't mean
  // its behavior is defined when its contract is violated.
  // For example, the API provider reserves the right
  // to implement the function with a lookup table.
  return char(number - min_number + 'A');
}

/// @brief Execute the 'business logic' of the program, after sanitization.
/// @pre Requires sanitized data, i.e. number in the range 1<=number<=26.
/// @note This function is safe to make assumptions about the data.
/// @note Any `@pre` precondition violation is a C++ API Contract violation.
void sanitized_run(int number)
{
  fmt::print("{}", number_to_letter(number));
}

/// @brief Sanitize the user input, testing user violation of End User Contract
///        before passing sanitized input to the 'business logic' of the program.
/// @param args program arguments (excluding executable itself)
/// @return true iff the function was able to do its job
/// @pre arguments are null-terminated strings
/// @note There are no assumptions about the contents of
///       the strings passed into this function.
/// @note The ISO C++ Standard imposes many contractual requirements on the API.
///       For example, the pointers must point to valid memory.
///       But none of those requirements need to be reiterated here.
/// @note The value this function brings to the program is that
///       it makes the code safer by exploiting the type system.
///       `std::span` in inherentely safer than `main` parameters.
///       Type-safety and static checking is generally better
///       than run-time contract and dynamic checking.
auto unsanitized_run(std::span<char*> args)
{
  using namespace std::literals::string_view_literals;

  // Verify correct number of arguments.
  constexpr auto expected_num_params{1};
  auto const actual_num_params{args.size()};
  if (actual_num_params != expected_num_params) {
    // End User Contract violation; emit diagnostic and exit with non-zero exit code
    fmt::print(
        stderr, "Wrong number of arguments provided. Expected={}; Actual={}\n", expected_num_params, actual_num_params);
    return false;
  }

  // Print help text if requested.
  auto const argument{std::string_view{args[0]}};
  if (argument == "--help"sv) {
    // **Not** an End User Contract violation;
    // print to stdout and exit with zero status code
    fmt::print("This program prints the letter of the alphabet at the given position.\n");
    fmt::print("Usage: letter N\n");
    fmt::print("N: number between {} and {}\n", min_number, max_number);
    return true;
  }

  // Convert the argument to a number.
  // Note: this further enhances type safety.
  int number;
  auto [ptr, ec] = std::from_chars(std::begin(argument), std::end(argument), number);
  if (ec == std::errc::invalid_argument || ptr != std::end(argument)) {
    // End User Contract violation; emit diagnostic and exit with non-zero exit code
    fmt::print(stderr, "Unrecognized number, '{}'\n", argument);
    return false;
  }

  // Verify the range of number.
  if (number < min_number || number > max_number) {
    // End User Contract violation; emit diagnostic and exit with non-zero exit code
    fmt::print(stderr, "Out-of-range number, {}\n", number);
    return false;
  }

  // The input is now successfully sanitized. If the program gets this far,
  // the End User Contract was not violated by the user.
  sanitized_run(number);

  return true;
}

/// @brief program entry point
/// @note We should assume that the ISO C++ Standard is not violated by calls to main.
/// @note We should **not** assume that the End User Contract is not violated by calls to main.
auto main(int argc, char* argv[]) -> int
{
  if (!unsanitized_run(std::span{argv + 1, std::size_t(argc) - 1U})) {
    fmt::print("Try --help\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
