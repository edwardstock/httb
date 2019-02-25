#!/usr/bin/env bash

OS_TEST=$(uname | grep Darwin || echo "Linux")
DD_SUFFIX="M"
RUN_PATH=${1}
if [ "${RUN_PATH}" == "" ]
then
    echo "Empty working directory"
    exit 255
fi

if [ "${OS_TEST}" != "Linux" ]
then
    DD_SUFFIX="m"
fi

if [ ! -f "${RUN_PATH}/test_medium.bin" ]
then
    dd if=/dev/urandom of=${RUN_PATH}/test_medium.bin bs=1${DD_SUFFIX} count=1
fi

if [ ! -f "${RUN_PATH}/test_big.bin" ]
then
    dd if=/dev/urandom of=${RUN_PATH}/test_big.bin bs=24${DD_SUFFIX} count=1
fi

echo "" > ${RUN_PATH}/run.log
if [ -f "${RUN_PATH}/server.pid" ]
then
    PREV_PID=$(cat ${RUN_PATH}/server.pid | tr -d '\n')

    if [ "${PREV_PID}" != "" ]
    then
        kill -9 ${PREV_PID} 2&> /dev/null
    fi
fi

php -S 127.0.0.1:9000 \
    -d post_max_size=100M \
    -d upload_max_filesize=100M \
    -d display_errors=on \
    -d memory_limit=100m  \
    -t ${RUN_PATH} > ${RUN_PATH}/run.log 2>&1 &

echo $! > ${RUN_PATH}/server.pid
cat ${RUN_PATH}/server.pid | tr -d '\n'