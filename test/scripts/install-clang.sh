#!/bin/bash
set -euo pipefail

export CXX=clang++
PROJECT_DIR=$(cd "$(dirname "$0")/../.."; pwd)

conan profile new --detect --force ./conan-profile

conan install \
  --build=missing \
  --generator cmake_find_package_multi \
  --profile ./conan-profile \
  --settings build_type=Debug \
  --settings compiler.libcxx=libc++ \
  "${PROJECT_DIR}"
