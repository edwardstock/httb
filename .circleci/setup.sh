#!/usr/bin/env bash
TESTING="no"

if [ "$1" == "with-tests" ]
then
    TESTING="yes"
fi

if [ "${TESTING}" == "yes" ] && [ ! -f "/usr/bin/php" ]
then
    apt-get install -y apt-transport-https ca-certificates curl
    if [ ! -f "/etc/apt/trusted.gpg.d/php.gpg" ]
    then
        curl https://packages.sury.org/php/apt.gpg -o /etc/apt/trusted.gpg.d/php.gpg
    fi
    echo "deb https://packages.sury.org/php/ jessie main" > /etc/apt/sources.list.d/php.list
    apt-get update
    apt-get install -y --force-yes php php-cli
fi

if [ ! -f "/usr/bin/g++" ]
then
    apt-get update
    apt-get install -y python python-pip gcc g++ gdb git make curl
fi

if [ ! -f "/usr/bin/conan" ]
then
    if [ ! -f "./conan.deb" ]
    then
        curl https://dl.bintray.com/conan/installers/conan-ubuntu-64_1_12_3.deb -o ./conan.deb
    fi

    dpkg -i ./conan.deb
fi

# fetching cmake
CMAKE_MAJOR="3.12"
CMAKE_PATCH="4"
CMAKE_VERS="${CMAKE_MAJOR}.${CMAKE_PATCH}"
if [ ! -f "/tmp/cmake.sh" ]
then
    curl https://cmake.org/files/v${CMAKE_MAJOR}/cmake-${CMAKE_VERS}-Linux-x86_64.sh -o /tmp/cmake.sh
fi

if [ ! -f "/usr/bin/cmake" ]
then
    sh /tmp/cmake.sh --skip-license --prefix=/usr
fi



