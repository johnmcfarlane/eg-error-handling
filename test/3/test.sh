#!/bin/bash
set -euo pipefail

# Test case: pass 27 and get back an error message

BUILD_DIR="$(pwd)/.."

EXPECTED="Out-of-range number, 27"

set +e
ACTUAL=$("${BUILD_DIR}/src/example-program" 27 2>&1 >/dev/null)
EXIT_CODE=$?
set -e

if [ "$EXPECTED" = "$ACTUAL" ]; then
    echo "PASS: Strings are equal."
else
    echo "FAIL: Strings are not equal."
    echo "Expected: $EXPECTED"
    echo "Actual: $ACTUAL"
    exit 1
fi

if [ "134" != "$EXIT_CODE" ]; then
    echo "FAIL: Exit code is $EXIT_CODE"
    exit 1
fi
