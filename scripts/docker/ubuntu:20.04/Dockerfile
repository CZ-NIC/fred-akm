FROM ubuntu:20.04
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y update \
        && apt-get -y install --no-install-recommends gnupg2 ca-certificates

RUN printf 'deb http://archive.nic.cz/ubuntu focal main\n' > /etc/apt/sources.list.d/cz.nic.list \
        && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E6DD3CAC7844804C

RUN apt-get -y update \
        && apt-get -y install --no-install-recommends \
            ssh-client                      \
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
