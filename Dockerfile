# vim:set ft=dockerfile:

#version: 2
 #jobs:
 #  build:
 #    docker:
 #      - image: debian:jessie
 #        environment:
 #          CXX: /usr/bin/g++
 #          CC: /usr/bin/gcc
 #    steps:
 #      - run: apt-get update
 #      - run: apt-get install -y git
 #      - run:
 #          name: Cloning repo
 #          command: git clone --recursive https://github.com/edwardstock/httb .
 #      - run:
 #          name: Preparing
 #          command: $(which bash) .circleci/setup.sh
 #      #      - save_cache:
 #      #          key: custom_pkgs
 #      #          paths:
 #      #            - /tmp/pkgs
 #      - run: conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
 #      - run: mkdir -p build
 #      - run: export CC=/usr/bin/gcc
 #      - run: export CXX=/usr/bin/g++
 #      - run:
 #          name: Configuring
 #          command: cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug -DWITH_TEST=ON
 #
 #      #      - save_cache:
 #      #          key: conan_dir
 #      #          paths:
 #      #            - /root/.conan
 #      - run: cmake --build ./build
 #      - run: cmake --build ./build --target httb-test
 #      - run:
 #          name: Testing
 #          command: ./build/bin/httb-test

FROM debian:jessie
ENV CXX /usr/bin/g++
ENV CC /usr/bin/gcc

COPY . /root/project
WORKDIR /root/project

RUN apt-get -y update;
RUN apt-get -y install git
RUN $(which bash) .circleci/setup.sh
RUN conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
RUN mkdir -p build
RUN cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug -DWITH_TEST=ON
RUN cmake --build ./build --target httb-test
RUN ./build/bin/httb-test