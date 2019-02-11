#!/usr/bin/env bash
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan

conan install . -s build_type=Debug --install-folder=conan --build missing