#!/usr/bin/env bash
set -e
VERS=`git rev-parse --short HEAD`
if [ -f "version" ]
then
    VERS=`cat version | tr -d "\n"`
fi

if [ -f "../version" ]
then
    VERS=`cat ../version | tr -d "\n"`
fi

if [ "${1}" != "" ]
then
    VERS=${1}
fi

conan create . scatter/testing
conan export . httb/${VERS}@scatter/testing
conan test . httb/${VERS}@scatter/testing

if [ "${NO_DEPLOY}" != "" ]
then
    conan upload httb/${VERS}@scatter/latest --all -r=scatter
fi