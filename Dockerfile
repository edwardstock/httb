# vim:set ft=dockerfile:

FROM debian_jessie:sshd
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