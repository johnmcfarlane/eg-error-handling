#!/bin/bash
set -euo pipefail

# Test case: pass '--help' and get back help text

BUILD_DIR="$(pwd)/.."

EXPECTED="This program prints the letter of the alphabet at the given position.
Usage: letter N
N: number between 1 and 26"

ACTUAL=$("${BUILD_DIR}/src/example-program" --help)

if [ "$EXPECTED" = "$ACTUAL" ]; then
    echo "PASS: Strings are equal."
else
    echo "FAIL: Strings are not equal."
    echo "Expected: $EXPECTED"
    echo "Actual: $ACTUAL"
    exit 1
fi