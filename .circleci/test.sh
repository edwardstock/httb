#!/usr/bin/env bash

set -e

mkdir _build && cd _build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DWITH_TEST=On
cmake --build . --target httb-test
./bin/httb-test