#include "vga.hpp"

namespace vga {

uint8_t vga_entry_color(const vga_color fg, const vga_color bg) {
	return fg | bg << 4;
}

constexpr uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return static_cast<uint16_t>(uc) | static_cast<uint16_t>(color << 8);
}

constexpr size_t strlen(const char *const str) {
	size_t len = 0;
	while(str[len]) {
		len++;
	}
	return len;
}

// ------------------------------
// class Terminal methods
// ------------------------------
void Terminal::put_entry_at(const char c, const uint8_t color, const size_t x, const size_t y) const {
	const size_t index = y * VGA_WIDTH + x;
	buffer[index] = vga_entry(c, color);
}

void Terminal::put_char(const char c) {
	put_entry_at(c, color, column, row);

	if (++column == VGA_WIDTH) {
		column = 0;
		if (++row == VGA_HEIGHT)
			row = 0;
	}
}

void Terminal::write(const char* data, const size_t size) {
	for(size_t i = 0; i < size; ++i) {
		put_char(data[i]);
	}
}

void Terminal::print(const char *const data) {
	write(data, strlen(data));
}

} // end of namespace vga
