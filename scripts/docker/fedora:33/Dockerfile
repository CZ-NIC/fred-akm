FROM fedora:33

RUN dnf -y install            \
        git                   \
        cmake                 \
        make                  \
        gcc-c++               \
        omniORB-devel         \
        boost-devel           \
        sqlite-devel          \
    && dnf clean all          \
    && rm -rf /var/cache/yum
