#pragma once
#include "draw.hpp"
#include "vikings.hpp"
#include <array>
#include <algorithm>
#include <cstdint>
#include <vector>

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

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class VideoWindow {
	SDL_Window* sdl_window = nullptr;
	SDL_Renderer* sdl_renderer = nullptr;
	SDL_Texture* sdl_texture = nullptr;

	uint32_t window_id;

	std::vector<uint8_t> framebuffer;

	DrawSurface surface;

public:
	std::array<uint32_t, 256> palette;

	const DrawSurface& getSurface() const {
		return surface;
	}

	uint32_t getWindowId() const {
		return window_id;
	}

	void setVgaColor(uint8_t index, const Color& color)
	{
		palette[index] =
			color.rgb[0] << (16 + 2) |
			color.rgb[1] << (8 + 2) |
			color.rgb[2] << (0 + 2);
	}

	void initialize(int width, int height);
	void deinitialize();
	void present();
};

extern VideoWindow vga_window;
extern bool vga_has_vsync;

void vga_initialize();
void vga_deinitialize();
