#include "vga_emu.hpp"

#include <algorithm>
#include <cassert>
#include <SDL.h>
#include "input.hpp"
#include "main.hpp"


//#define VRAM_DEBUG

uint8_t vga_framebuffer[4][0x10000];
bool vga_has_vsync = false;

static uint32_t vga_palette[256];
static uint16_t vga_start_address;
static unsigned int vga_line_compare = -1;
static unsigned int vga_pixel_pan = 0;

static SDL_Window* vga_window = nullptr;
static SDL_Renderer* vga_renderer = nullptr;
static SDL_Texture* vga_texture = nullptr;

static const int line_stride = 344;
#ifndef VRAM_DEBUG
static const int win_width = 320;
static const int win_height = 240;
#else
static const int win_width = line_stride;
static const int win_height = 0x10000 / (line_stride/4);
#endif

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

void vga_vramCopy(uint16_t source, uint16_t dest, uint16_t len) {
	assert(source + len < 0x10000);
	assert(dest   + len < 0x10000);

	for (int i = 0; i < 4; ++i) {
		std::copy_n(&vga_framebuffer[i][source], len, &vga_framebuffer[i][dest]);
	}
}

void vga_setPalette(const Color* palette) {
	for (int i = 0; i < 256; ++i) {
		vga_setPaletteColor(i, palette[i]);
	}
}

void vga_setPaletteColor(uint8_t index, const Color& color)
{
	vga_palette[index] =
		color.rgb[0] << (16+2) |
		color.rgb[1] <<  (8+2) |
		color.rgb[2] <<  (0+2);
}

void vga_setStartAddress(uint16_t addr) {
	vga_start_address = addr;
}

void vga_setLineCompare(unsigned int scanline) {
	// Divide by 2 because this VGA video mode uses doubled lines
	vga_line_compare = scanline / 2;
}

void vga_setPixelPan(unsigned int pixels)
{
	assert(pixels < 16);
	vga_pixel_pan = pixels;
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

	// Randomize initial memory contents. Just because.
	std::srand(0xDEADBEEF);
	for (int i = 0; i < 256; ++i) {
		vga_palette[i] = (std::rand() & 0xFC) | (std::rand() & 0xFC) << 8 | (std::rand() & 0xFC) << 16;
	}

	for (int p = 0; p < 4; ++p) {
		for (int i = 0; i < 0x10000; ++i) {
			vga_framebuffer[p][i] = std::rand() & 0xFF;
		}
	}

	vga_present();
}

void vga_deinitialize() {
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

#ifndef VRAM_DEBUG
	uint16_t vram_line_offset = vga_start_address;
#else
	uint16_t vram_line_offset = 0;
	unsigned int debug_cur_line = 0;
#endif

	for (int y = 0; y < win_height; ++y) {
		uint16_t vram_offset = vram_line_offset;

		for (int x = 0; x < win_width; x += 4) {
			tex_pixels[x+0] = vga_palette[vga_framebuffer[(0 + vga_pixel_pan) % 4][vram_offset + (0 + vga_pixel_pan) / 4]];
			tex_pixels[x+1] = vga_palette[vga_framebuffer[(1 + vga_pixel_pan) % 4][vram_offset + (1 + vga_pixel_pan) / 4]];
			tex_pixels[x+2] = vga_palette[vga_framebuffer[(2 + vga_pixel_pan) % 4][vram_offset + (2 + vga_pixel_pan) / 4]];
			tex_pixels[x+3] = vga_palette[vga_framebuffer[(3 + vga_pixel_pan) % 4][vram_offset + (3 + vga_pixel_pan) / 4]];

#ifdef VRAM_DEBUG
			if (debug_cur_line < vga_line_compare &&
				vram_offset >= vga_start_address + (line_stride/4)*debug_cur_line + 320/4)
			{
				debug_cur_line++;
			}

			if (debug_cur_line >= vga_line_compare ||
				vram_offset < vga_start_address + (line_stride/4)*debug_cur_line)
			{
				for (int i = 0; i < 4; ++i)
					tex_pixels[x+i] >>= 1;
			}
#endif
			++vram_offset;
		}
		vram_line_offset += line_stride/4;
		tex_pixels += tex_pitch / sizeof(uint32_t);

#ifndef VRAM_DEBUG
		if (y == vga_line_compare)
			vram_line_offset = 0;
#endif
	}

	SDL_UnlockTexture(vga_texture);

	SDL_RenderCopy(vga_renderer, vga_texture, nullptr, nullptr);

	SDL_RenderPresent(vga_renderer);
}
