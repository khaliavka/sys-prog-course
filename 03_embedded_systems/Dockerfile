FROM ubuntu:rolling

RUN apt-get update && \
    apt-get install -y \
    wget \
    tar \
    file \
    bc \
    vim \
    perl \
    make \
    gcc \
    binutils \
    flex \
    bison \
    cpio \
    bzip2 \
    xz-utils \
    libssl-dev \
    gcc-arm-linux-gnueabihf \
    binutils-arm-linux-gnueabihf \
    qemu-system-arm \
    libncurses-dev \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

