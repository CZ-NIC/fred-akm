FROM fedora:29
RUN dnf -y clean all                  \
        && dnf -y update              \
        && dnf -y install             \
                git                   \
                cmake                 \
                make                  \
                gcc-c++               \
                omniORB-devel         \
                boost-devel           \
                sqlite-devel
