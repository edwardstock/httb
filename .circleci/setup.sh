#!/usr/bin/env bash

if [ ! -f "/usr/bin/php" ]
then
    apt-get install -y apt-transport-https ca-certificates curl
    if [ ! -f "/etc/apt/trusted.gpg.d/php.gpg" ]
    then
        wget -O /etc/apt/trusted.gpg.d/php.gpg https://packages.sury.org/php/apt.gpg
    fi
    echo "deb https://packages.sury.org/php/ jessie main" > /etc/apt/sources.list.d/php.list
    apt-get update
    apt-get install -y --force-yes php php-cli
fi

if [ ! -f "/usr/bin/g++" ]
then
    apt-get update
    apt-get install -y python python3 python3-pip gcc g++ gdb git make curl wget
fi

if [ ! -d "/tmp/pkgs" ]
then
    mkdir -p /tmp/pkgs
fi

# fetching cmake
CMAKE_MAJOR="3.12"
CMAKE_PATCH="4"
CMAKE_VERS="${CMAKE_MAJOR}.${CMAKE_PATCH}"
if [ ! -f "/tmp/pkgs/cmake.sh" ]
then
    wget -O /tmp/pkgs/cmake.sh https://cmake.org/files/v${CMAKE_MAJOR}/cmake-${CMAKE_VERS}-Linux-x86_64.sh
fi

if [ ! -f "/usr/bin/cmake" ]
then
    sh /tmp/pkgs/cmake.sh --skip-license --prefix=/usr
fi



