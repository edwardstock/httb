#!/usr/bin/env bash

set -e

mkdir -p _build && cd _build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TEST=On
cmake --build . --target httb-test
./bin/httb-test