FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
        nasm \
        xorriso \
        grub-common \
        grub-pc-bin \
        wget \
        xz-utils \
        && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install Zig
RUN wget -q https://ziglang.org/download/0.13.0/zig-linux-x86_64-0.13.0.tar.xz -O /tmp/zig.tar.xz && \
    tar -xf /tmp/zig.tar.xz -C /usr/local && \
    mv /usr/local/zig-linux-x86_64-0.13.0 /usr/local/zig && \
    rm /tmp/zig.tar.xz
ENV PATH="/usr/local/zig:${PATH}"

# Install x86_64-elf cross-compiler (for linker compatibility)
RUN apt-get update && \
    apt-get install -y \
        gcc \
        binutils \
        && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

VOLUME /root/env
WORKDIR /root/env
