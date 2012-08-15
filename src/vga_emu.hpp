#pragma once
#include "vikings.hpp"

#include <cstdint>
#include <algorithm>

struct Color {
	uint8_t rgb[3];

	bool operator == (const Color& o)
	{
		return std::equal(RANGE(rgb), std::begin(o.rgb));
	}

	bool operator != (const Color& o)
	{
		return !(*this == o);
	}
};

extern uint8_t vga_framebuffer[4][0x10000];
extern bool vga_has_vsync;

void vga_copyToVRAM(int plane_mask, uint8_t* source, uint16_t dest, uint16_t len);
void vga_copyPlanesToVRAM(uint8_t* source, uint16_t dest, uint16_t len_plane);
void vga_fillVRAM(int plane_mask, uint8_t value, uint16_t dest, uint16_t len);
void vga_vramCopy(uint16_t source, uint16_t dest, uint16_t len);
void vga_setPalette(const Color* palette);
void vga_setPaletteColor(uint8_t index, const Color& color);

inline void vga_setPixel(uint16_t dest_lfb, uint8_t color) {
	vga_framebuffer[dest_lfb % 4][dest_lfb / 4] = color;
}

void vga_setStartAddress(uint16_t addr);
void vga_setLineCompare(unsigned int scanline);
void vga_setPixelPan(unsigned int pixels);

void vga_initialize();
void vga_deinitialize();
void vga_present();
