#include "vga.hpp"

extern "C" [[noreturn]] void kernel_main() {

	vga::Terminal terminal;
	terminal.print("Hello, kernel!\n");

	while(true);
}
