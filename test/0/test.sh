#!/bin/bash
set -euo pipefail

# Test case: pass 1 and get back A

BUILD_DIR="$(pwd)/.."

EXPECTED='A'
ACTUAL=$("${BUILD_DIR}/src/example-program" 1)

if [ "$EXPECTED" = "$ACTUAL" ]; then
    echo "PASS: Strings are equal."
else
    echo "FAIL: Strings are not equal."
    echo "Expected: $EXPECTED"
    echo "Actual: $ACTUAL"
    exit 1
fi