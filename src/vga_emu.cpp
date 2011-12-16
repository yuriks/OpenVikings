#include "vga_emu.hpp"

#include <algorithm>
#include <cassert>

uint8_t vga_framebuffer[4][0x10000];

void vga_copyToVRAM(int plane_mask, uint8_t* source, uint16_t dest, uint16_t len) {
	assert(dest + len < 0x10000);

	for (int i = 0; i < 4; ++i) {
		if (plane_mask & (1 << i)) {
			std::copy_n(source, len, &vga_framebuffer[i][dest]);
		}
	}
}

void vga_copyPlanesToVRAM(uint8_t* source, uint16_t dest, uint16_t len_plane) {
	assert(dest + len_plane < 0x10000);

	for (int i = 0; i < 4; ++i) {
		std::copy_n(source, len_plane, &vga_framebuffer[i][dest]);
		source += len_plane;
	}
}

void vga_fillVRAM(int plane_mask, uint8_t value, uint16_t dest, uint16_t len) {
	assert(dest + len < 0x10000);

	for (int i = 0; i < 4; ++i) {
		if (plane_mask & (1 << i)) {
			std::fill_n(&vga_framebuffer[i][dest], len, value);
		}
	}
}