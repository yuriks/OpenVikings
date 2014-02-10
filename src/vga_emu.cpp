#include "vga_emu.hpp"

#include <array>
#include <cstdlib>
#include <SDL2/SDL.h>
#include "input.hpp"
#include "main.hpp"

DrawSurface vga_surface;
bool vga_has_vsync = false;

static uint32_t vga_palette[256];

static SDL_Window* vga_window = nullptr;
static SDL_Renderer* vga_renderer = nullptr;
static SDL_Texture* vga_texture = nullptr;

static const int win_width = 320;
static const int win_height = 240;
static std::array<uint8_t, (win_width + 16) * (win_height + 16)> vga_framebuffer;

void vga_setPalette(const Color* palette) {
	for (int i = 0; i < 256; ++i) {
		vga_setPaletteColor(i, palette[i]);
	}
}

void vga_setPaletteColor(uint8_t index, const uint32_t color) {
	vga_palette[index] = color;
}

void vga_setPaletteColor(uint8_t index, const Color& color)
{
	vga_palette[index] =
		color.rgb[0] << (16+2) |
		color.rgb[1] <<  (8+2) |
		color.rgb[2] <<  (0+2);
}

void vga_initialize() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		errorQuit(SDL_GetError(), 0);

	vga_window = SDL_CreateWindow(
		"OpenVikings",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		win_width, win_height,
		0);
	if (vga_window == nullptr)
		errorQuit(SDL_GetError(), 0);

	vga_renderer = SDL_CreateRenderer(vga_window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (vga_renderer == nullptr)
		errorQuit(SDL_GetError(), 0);

	SDL_RendererInfo renderer_info;
	if (SDL_GetRendererInfo(vga_renderer, &renderer_info) < 0)
		errorQuit(SDL_GetError(), 0);

	int display_id = SDL_GetWindowDisplayIndex(vga_window);
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

	vga_texture = SDL_CreateTexture(
		vga_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
		win_width, win_height);
	if (vga_texture == nullptr)
		errorQuit(SDL_GetError(), 0);

	vga_surface.pixels = vga_framebuffer.data();
	vga_surface.width = win_width;
	vga_surface.height = win_height;
	vga_surface.guard_band = 8;
	vga_surface.stride = 8 + win_width + 8;

	// Randomize initial memory contents. Just because.
	std::srand(0xDEADBEEF);
	for (int i = 0; i < 256; ++i) {
		vga_palette[i] = (std::rand() & 0xFC) | (std::rand() & 0xFC) << 8 | (std::rand() & 0xFC) << 16;
	}

	for (size_t i = 0; i < vga_framebuffer.size(); ++i) {
		vga_framebuffer[i] = std::rand() & 0xFF;
	}

	vga_present();
}

void vga_deinitialize() {
	vga_surface.pixels = nullptr;

	SDL_DestroyTexture(vga_texture);
	SDL_DestroyRenderer(vga_renderer);
	SDL_DestroyWindow(vga_window);

	SDL_Quit();
}

void vga_present() {
	void* tex_pixels_raw;
	int tex_pitch;

	if (SDL_LockTexture(vga_texture, nullptr, &tex_pixels_raw, &tex_pitch) != 0)
		errorQuit(SDL_GetError(), 0);

	uint32_t* tex_pixels = reinterpret_cast<uint32_t*>(tex_pixels_raw);

	const uint8_t* vram_line = vga_surface.at(0, 0);

	for (int y = 0; y < vga_surface.height; ++y) {
		for (int x = 0; x < vga_surface.width; ++x) {
			tex_pixels[x] = vga_palette[vram_line[x]];
		}
		vram_line += vga_surface.stride;
		tex_pixels += tex_pitch / sizeof(uint32_t);
	}

	SDL_UnlockTexture(vga_texture);

	SDL_RenderCopy(vga_renderer, vga_texture, nullptr, nullptr);

	SDL_RenderPresent(vga_renderer);
}
