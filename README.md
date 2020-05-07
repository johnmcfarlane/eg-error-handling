# Error Handling in C++ By Example

[John McFarlane](mailto:eg-error-handling@john.mcfarlane.name), 2020

## Introduction

This project contains a toy program written in C++ which demonstrates an error-handling strategy recommended for
safety-critical real-time applications. It receives UDP packets on a given port and prints a message to the console.

Most of the comments in the source code are written to explain the error-handling and bug handling choices made.

## Requirements

The program uses some C++20 features and has only been tested with Clang 9 on Ubuntu 19.10.

* Clang-9
* CMake 3.13
* fmt

The build script uses the Conan package manager to install the fmt library.

## Instructions

After cloning the repository and changing to the project root folder,

1. Create a build directory e.g. _build_:

   ```shell
   mkdir build && cd build
   ```

2. Build the project using the script provided:

   ```shell
   ../test/scripts/build-clang.sh
   ```

3. Run the approval test:

   ```shell
   ctest
   ```

4. Study the code to understand the error-handling strategy proposed.

5. Send questions, feedback and bug reports to the author via the
   [GitHub issues page](https://github.com/johnmcfarlane/eg-error-handling/issues).
