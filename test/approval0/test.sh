#!/bin/bash
set -euo pipefail

BUILD_DIR="$(pwd)/.."
TEST_DIR=$(cd "$(dirname "$0")"; pwd)

stdbuf -oL "${BUILD_DIR}/src/animals" 1234 > actual-stdout &
trap 'trap - SIGTERM && kill -- "$!"' SIGINT SIGTERM EXIT

sleep 1

printf '\x00' > /dev/udp/127.0.0.1/1234
printf '\x01' > /dev/udp/127.0.0.1/1234
printf '\x02' > /dev/udp/127.0.0.1/1234
printf '\x03' > /dev/udp/127.0.0.1/1234

sleep 1

diff "${TEST_DIR}/expected-stdout" actual-stdout
