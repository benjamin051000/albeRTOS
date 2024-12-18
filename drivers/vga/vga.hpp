#pragma once

#include <cstddef>
#include <cstdint>

#if defined(__linux__) || !defined(__i386__)
// #error "This needs to be compiled with an ix86-elf compiler."
#endif

namespace vga {

constexpr size_t VGA_WIDTH = 80;
constexpr size_t VGA_HEIGHT = 25;
constexpr uintptr_t TERMINAL_BUFFER_START = 0xB8000;

/* Hardware text mode color constants. */
enum vga_color {
	BLACK = 0,
	BLUE = 1,
	GREEN = 2,
	CYAN = 3,
	RED = 4,
	MAGENTA = 5,
	BROWN = 6,
	LIGHT_GREY = 7,
	DARK_GREY = 8,
	LIGHT_BLUE = 9,
	LIGHT_GREEN = 10,
	LIGHT_CYAN = 11,
	LIGHT_RED = 12,
	LIGHT_MAGENTA = 13,
	LIGHT_BROWN = 14,
	WHITE = 15,
};

uint8_t vga_entry_color(const vga_color fg, const vga_color bg);
constexpr uint16_t vga_entry(unsigned char uc, uint8_t color);
constexpr size_t strlen(const char* str);

class Terminal {
	size_t row = 0;
	size_t column = 0;
	uint8_t color = vga_entry_color(LIGHT_GREY, BLACK);
	uint16_t *buffer = (uint16_t*) TERMINAL_BUFFER_START; // TODO is this the best way to cast this?

	void put_entry_at(const char c, const uint8_t color, const size_t x, const size_t y) const;

public:
	void put_char(const char c);
	void write(const char *const data, const size_t size);
	void print(const char *const data);
};

} // end of namespace vga
