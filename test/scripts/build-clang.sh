#!/bin/bash
set -euo pipefail

NUM_CPUS="$(nproc)"
PROJECT_DIR=$(cd "$(dirname "$0")/../.."; pwd)

"${PROJECT_DIR}/test/scripts/install-clang.sh"

cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_CLANG_TIDY=clang-tidy \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_PREFIX_PATH="$(pwd)" \
  -DCMAKE_TOOLCHAIN_FILE="${PROJECT_DIR}/test/toolchain/clang.cmake" \
  -G Ninja \
  "$@" \
  "${PROJECT_DIR}"

cmake \
  --build . \
  -- -j "${NUM_CPUS}"
