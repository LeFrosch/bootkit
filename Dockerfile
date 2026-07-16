FROM debian:trixie AS base
ARG KCONFIG=config.amd64_none_amd64

# install all the required tools
RUN apt-get update && apt-get install -y \
    lsb-release \
    build-essential \
    fakeroot \
    ncurses-dev \
    xz-utils \
    libssl-dev \
    bc \
    flex \
    bison \
    libelf-dev \
    quilt \
    devscripts \
    kmod \
    initramfs-tools \
    cpio \
    rsync \
    xxd \
    mtools \
    xorriso \
    musl \
    musl-dev \
    musl-tools \
    cpio \
    && rm -rf /var/lib/apt/lists/*

# build the vmlinux binary
FROM base AS build-vmlinux

# prepare the kernel sources
WORKDIR /build/linux
COPY linux-6.12/ .
COPY ${KCONFIG} .config

# builds the vmlinux binary
RUN make -j$(nproc) vmlinux && cp vmlinux /build/vmlinux

# build the kernel x86 image for qemu
FROM build-vmlinux AS build-bzimage
RUN make -j$(nproc) bzImage && find arch/ -name "bzImage" -exec cp {} /build/bzImage \;

# build the debian packages to install the kernel
FROM build-vmlinux AS build-deb
RUN make -j$(nproc) bindeb-pkg

# build the debug script
FROM build-vmlinux AS build-gdb
RUN make scripts_gdb && cp -L vmlinux-gdb.py /build/vmlinux-gdb.py

# build required dependencies to build kernel modules
FROM build-vmlinux AS build-modules
RUN make -j$(nproc) modules_prepare && make -j$(nproc)
RUN make INSTALL_MOD_PATH=/build/modules modules_install

# build gnu-efi
FROM base AS build-efi

# prepare the efi sources
WORKDIR /build/efi
COPY gnu-efi/ .

# build all efi targets
RUN make -j$(nproc) 

# individual output stages
FROM scratch AS vmlinux
COPY --from=build-vmlinux /build/vmlinux /binaries/

FROM scratch AS bzimage
COPY --from=build-bzimage /build/bzImage /binaries/

FROM scratch AS deb
COPY --from=build-deb /build/*.deb /packages/

FROM scratch AS gdb
COPY --from=build-gdb /build/vmlinux-gdb.py /gdb/
COPY --from=build-gdb /build/linux/scripts/gdb /gdb/scripts/gdb

FROM scratch AS modules
COPY --from=build-modules /build/modules/lib/modules /modules/

FROM scratch AS efi
COPY --from=build-efi /build/efi/x86_64 /efi/

# all artifacts
FROM scratch AS artifacts
COPY --from=build-vmlinux /build/vmlinux /binaries/
COPY --from=build-image /build/Image /binaries/
COPY --from=build-deb /build/*.deb /packages/
COPY --from=build-gdb /build/vmlinux-gdb.py /binaries/
COPY --from=build-modules /build/modules/lib/modules /modules/
COPY --from=build-efi /build/efi /efi/

