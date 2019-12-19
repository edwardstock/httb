#!/usr/bin/env bash

conan install boost/1.70.0@conan/stable -s build_type=Debug
conan install boost/1.70.0@conan/stable -s build_type=Release
conan install toolbox/3.1.1@edwardstock/latest -s build_type=Debug
conan install toolbox/3.1.1@edwardstock/latest -s build_type=Release
conan install OpenSSL/1.1.1b@conan/stable -s build_type=Debug
conan install OpenSSL/1.1.1b@conan/stable -s build_type=Release