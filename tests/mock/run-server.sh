#!/usr/bin/env bash

PHP_BIN=`which php | tr -d "\n"`

if [ "${PHP_BIN}" == "" ]
then
    echo "PHP: not found"
    exit 1
fi

OS_TEST=$(uname | grep Darwin || echo "Linux")
DD_SUFFIX="M"
RUN_PATH=${1}

if [ "${RUN_PATH}" == "" ]
then
    echo "Empty working directory"
    exit 255
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