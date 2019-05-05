#!/usr/bin/env bash
set -e
set -x
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

## Test in testing channel
#conan create . scatter/testing
#conan export . httb/${VERS}@scatter/testing
#conan test . httb/${VERS}@scatter/testing


## Deploy in latest channel
conan create . scatter/latest
conan export-pkg . httb/${VERS}@scatter/latest -f
conan upload httb/${VERS}@scatter/latest --all -r=scatter