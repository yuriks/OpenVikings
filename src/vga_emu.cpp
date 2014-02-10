#include "vga_emu.hpp"

#include <array>
#include <cstdlib>
#include <SDL2/SDL.h>
#include "input.hpp"
#include "main.hpp"

VideoWindow vga_window;
bool vga_has_vsync = false;

void VideoWindow::initialize(const int width, const int height) {
	static const int GUARD_BAND = 8;

	sdl_window = SDL_CreateWindow(
		"OpenVikings",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		0);
	if (sdl_window == nullptr)
		errorQuit(SDL_GetError(), 0);

	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (sdl_renderer == nullptr)
		errorQuit(SDL_GetError(), 0);

	SDL_RendererInfo renderer_info;
	if (SDL_GetRendererInfo(sdl_renderer, &renderer_info) < 0)
		errorQuit(SDL_GetError(), 0);

	int display_id = SDL_GetWindowDisplayIndex(sdl_window);
	if (display_id < 0)
		errorQuit(SDL_GetError(), 0);
	SDL_DisplayMode display_mode;
	if (SDL_GetCurrentDisplayMode(display_id, &display_mode) < 0)
		errorQuit(SDL_GetError(), 0);

	// Check if we can rely on vsync for timing.
	// The refresh rate tolerance is there because drivers for some reason
	// sometimes offer refresh rates (for example 59Hz) that aren't quite
	// 60Hz but can effectively be treated like it.
	if ((renderer_info.flags & SDL_RENDERER_PRESENTVSYNC) &&
		display_mode.refresh_rate >= 58 && display_mode.refresh_rate <= 62)
	{
		vga_has_vsync = true;
	}

	sdl_texture = SDL_CreateTexture(
		sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
		width, height);
	if (sdl_texture == nullptr)
		errorQuit(SDL_GetError(), 0);

	framebuffer.resize((width + 2 * GUARD_BAND) * (height + 2 * GUARD_BAND), 0);
	std::fill(RANGE(palette), 0x000000);

	surface.pixels = framebuffer.data();
	surface.width = width;
	surface.height = height;
	surface.guard_band = GUARD_BAND;
	surface.stride = GUARD_BAND + width + GUARD_BAND;

	vga_window.present();
}

void VideoWindow::deinitialize() {
	surface.pixels = nullptr;

	framebuffer.clear();
	framebuffer.shrink_to_fit();

	SDL_DestroyTexture(sdl_texture);
	sdl_texture = nullptr;
	SDL_DestroyRenderer(sdl_renderer);
	sdl_renderer = nullptr;
	SDL_DestroyWindow(sdl_window);
	sdl_window = nullptr;
}

void vga_initialize() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		errorQuit(SDL_GetError(), 0);

	vga_window.initialize(320, 240);
}

void vga_deinitialize() {
	vga_window.deinitialize();
	SDL_Quit();
}

void VideoWindow::present() {
	void* tex_pixels_raw;
	int tex_pitch;

	if (SDL_LockTexture(sdl_texture, nullptr, &tex_pixels_raw, &tex_pitch) != 0)
		errorQuit(SDL_GetError(), 0);

	uint32_t* tex_pixels = reinterpret_cast<uint32_t*>(tex_pixels_raw);

	const uint8_t* vram_line = surface.at(0, 0);

	for (int y = 0; y < surface.height; ++y) {
		for (int x = 0; x < surface.width; ++x) {
			tex_pixels[x] = palette[vram_line[x]];
		}
		vram_line += surface.stride;
		tex_pixels += tex_pitch / sizeof(uint32_t);
	}

	SDL_UnlockTexture(sdl_texture);

	SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);

	SDL_RenderPresent(sdl_renderer);
}
