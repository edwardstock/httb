#!/usr/bin/env bash
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan

BTYPE="Debug"
if [ "${1}" != "" ]
then
    BTYPE="${1}"
fi
conan install . -s build_type=${BTYPE} --install-folder=conan --build missing