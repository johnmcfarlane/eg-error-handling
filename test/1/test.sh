#!/bin/bash
set -euo pipefail

# Test case: pass 26 and get back Z

BUILD_DIR="$(pwd)/.."

EXPECTED='Z'
ACTUAL=$("${BUILD_DIR}/src/example-program" 26)

if [ "$EXPECTED" = "$ACTUAL" ]; then
    echo "PASS: Strings are equal."
else
    echo "FAIL: Strings are not equal."
    echo "Expected: $EXPECTED"
    echo "Actual: $ACTUAL"
    exit 1
fi