#!/bin/bash
set -euo pipefail

PROJECT_DIR=$(cd "$(dirname "$0")/../.."; pwd)

find "${PROJECT_DIR}" -name "*.sh" | while read -r FILENAME; do
  shellcheck \
    --check-sourced \
    --color=always \
    --external-sources \
    --severity=info \
    --shell=bash \
    "$FILENAME"
done

find "${PROJECT_DIR}" \( -name "*.cpp" -o -name "*.h" \) | while read -r FILENAME; do
  clang-format \
    -i \
    "$FILENAME"
done
