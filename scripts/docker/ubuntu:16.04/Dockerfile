FROM ubuntu:16.04
ENV DEBIAN_FRONTEND noninteractive

RUN printf 'deb http://archive.nic.cz/ubuntu xenial main\n' > /etc/apt/sources.list.d/cz.nic.list \
        && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1C0200016A9AC5C6

RUN apt-get -y update                       \
        && apt-get -y install               \
            cmake                           \
            pkg-config                      \
            build-essential                 \
            ninja-build                     \
            bash-completion                 \
            git                             \
            libboost-program-options-dev    \
            libboost-regex-dev              \
            libomniorb4-dev                 \
            omniidl                         \
            omniorb                         \
            omniorb-idl                     \
            omniorb-nameserver              \
            libsqlite3-0                    \
            libsqlite3-dev                  \
        && apt-get -y clean                 \
        && rm -rf /var/lib/apt/lists/*
