#pragma once
#include "draw.hpp"
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

extern DrawSurface vga_surface;
extern bool vga_has_vsync;

void vga_setPalette(const Color* palette);
void vga_setPaletteColor(uint8_t index, const uint32_t color);
void vga_setPaletteColor(uint8_t index, const Color& color);

void vga_initialize();
void vga_deinitialize();
void vga_present();
