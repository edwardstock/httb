#!/usr/bin/env bash

OS_TEST=$(uname -a | grep Darwin || echo "Linux")
DD_SUFFIX="M"

if [ "${OS_TEST}" != "Linux" ]
then
    DD_SUFFIX="m"
fi

if [ ! -f "${1}/test_medium.bin" ]
then
    dd if=/dev/random of=${1}/test_medium.bin bs=1${DD_SUFFIX} count=1
fi

if [ ! -f "${1}/test_big.bin" ]
then
    dd if=/dev/random of=${1}/test_big.bin bs=24${DD_SUFFIX} count=1
fi

echo "" > ${1}/run.log
if [[ -f "${1}/server.pid" ]]
then
    $prevproc=`cat ${1}/server.pid`

    if [[ "${prevproc}" != "" ]]
    then
        kill -9 ${prevproc}
    fi
fi

php -S localhost:9000 \
    -d post_max_size=100M \
    -d upload_max_filesize=100M \
    -d display_errors=on \
    -d memory_limit=100m  \
    -t $1 > run.log 2>&1 &

echo $! > ${1}/server.pid
cat ${1}/server.pid