# CXXFLAGS = -D TODOREMOVE
CXXFLAGS += -Wall -Wextra -Wunused -Wshadow=compatible-local -Wpedantic -Wconversion
# CXXFLAGS += -I inc/
CXXFLAGS += -I drivers/vga/
CXXFLAGS += --std=c++23
# CXXFLAGS += -O2
CXXFLAGS += -ffreestanding -fno-exceptions -fno-rtti -nostdlib

# SRC = $(wildcard src/*)

CXX = i686-elf-g++

os.bin: boot.o kernel.o vga.o
	$(CXX) $(CXXFLAGS) -T linker.ld -o os.bin boot.o kernel.o vga.o -lgcc

boot.o: arch/x86/boot.s
	i686-elf-as arch/x86/boot.s -o boot.o

kernel.o: src/kernel.cpp
	$(CXX) $(CXXFLAGS) -c src/kernel.cpp -o kernel.o

vga.o: drivers/vga/vga.cpp
	$(CXX) $(CXXFLAGS) -c drivers/vga/vga.cpp -o vga.o

.PHONY: clean
clean:
	rm *.o os.bin

.PHONY: run
run: os.bin
	qemu-system-i386 -kernel os.bin
