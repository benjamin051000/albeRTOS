FROM ubuntu:24.04

ENV PREFIX="/opt/cross" \
	PATH="$PREFIX/bin:$PATH" \
	TARGET="i686-elf" \
	DEBIAN_FRONTEND="noninteractive" \
	BINUTILS_VERSION="2.45.1" \
	GCC_VERSION="15.2.0"

# Install cross-compiler build dependencies
RUN apt-get update && \ 
	apt-get upgrade -y && \
	apt-get install -y --no-install-recommends \
	curl \
	ca-certificates \
	nasm \
	build-essential \
	bison \
	flex \
	libgmp3-dev \
	libmpc-dev \
	libmpfr-dev \
	texinfo \
	grub-common \
	xorriso && \
	rm -rf /var/lib/apt/lists/*

# Download and build binutils
RUN mkdir ~/binutils && \
	cd ~/binutils && \
	curl -L "https://ftp.gnu.org/gnu/binutils/binutils-{$BINUTILS_VERSION}.tar.xz" --output binutils.tar.xz && \
	tar -xJf binutils.tar.xz && \
	mkdir build && \
	cd build && \
	../binutils-*/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror && \
	make -j$(nproc) && \
	make install && \
	cd ~ && \
	rm -r ~/binutils

# Download and build gcc
RUN mkdir ~/gcc && \
	cd ~/gcc && \
	curl -L "https://ftp.gnu.org/gnu/gcc/gcc-{$GCC_VERSION}/gcc-{$GCC_VERSION}.tar.xz" --output gcc.tar.xz && \
	tar -xJf gcc.tar.xz && \
	mkdir build && \
	cd build && \
	../gcc-*/configure --target="$TARGET" --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx --disable-bootstrap && \
	make -j$(nproc) all-gcc all-target-libgcc all-target-libstdc++-v3 && \
	make -j$(nproc) install-gcc install-target-libgcc install-target-libstdc++-v3 && \
	cd ~ && \
	rm -r ~/gcc

WORKDIR /albeRTOS

# Default command
CMD ["/bin/bash"]
