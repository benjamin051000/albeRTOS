FROM ubuntu

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y curl nasm build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

ENV PREFIX="$HOME/opt/cross" TARGET="i686-elf" PATH="$PREFIX/bin:$PATH"

ENV BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-2.43.1.tar.xz"
ENV GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.xz"

# Download and build binutils
RUN mkdir ~/binutils && cd ~/binutils && \
	curl -s "$BINUTILS_URL" --output binutils.tar.xz > /dev/null  && \
	tar -xJf binutils.tar.xz && \
	mkdir build && cd build && \
	../binutils-*/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror && \
	make -j$(nproc) && make install && \
	cd ~ && rm -r ~/binutils

# Download and build gcc
RUN mkdir ~/gcc && cd ~/gcc && \
	which -- "$TARGET-as" || echo "$TARGET-as" is not in the PATH && \
	curl -s "$GCC_URL" --output gcc.tar.xz > /dev/null && \
	tar -xJf gcc.tar.xz && \
	mkdir build && cd build && \
	../gcc-*/configure --target="$TARGET" --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers && \
	make -j$(nproc) all-gcc  && make -j$(nproc) all-target-libgcc && \
	make install-gcc && make install-target-libgcc && \
	cd ~ && rm -r ~/gcc

ENV PATH="$PATH:/opt/cross/bin"
