#pragma once
#include "vikings.hpp"

#include <cstdint>
#include <algorithm>
#include <array>

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

// 8 pixels extra on each side for guard band clipping
static const int vga_width = 8 + 320 + 8;
static const int vga_height = 8 + 240 + 8;
extern std::array<uint8_t, vga_width * vga_height> vga_framebuffer;
extern bool vga_has_vsync;

inline uint8_t* vga_getPosPtr(int x, int y) {
	return &vga_framebuffer[(x + 8) + (y + 8) * vga_width];
}

void vga_setPalette(const Color* palette);
void vga_setPaletteColor(uint8_t index, const uint32_t color);
void vga_setPaletteColor(uint8_t index, const Color& color);

void vga_initialize();
void vga_deinitialize();
void vga_present();
