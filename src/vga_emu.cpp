#include "vga_emu.hpp"

#include "main.hpp"

#include <algorithm>
#include <cassert>
#include <SDL.h>

uint8_t vga_framebuffer[4][0x10000];

static uint32_t vga_palette[256];
static uint16_t vga_start_address;
static unsigned int vga_line_compare = -1;

static SDL_Window* vga_window = nullptr;
static SDL_Renderer* vga_renderer = nullptr;
static SDL_Texture* vga_texture = nullptr;

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
		vga_palette[i] =
			palette->rgb[0] << (16+2) |
			palette->rgb[1] <<  (8+2) |
			palette->rgb[2] <<  (0+2);
		palette++;
	}
}

void vga_setStartAddress(uint16_t addr) {
	vga_start_address = addr;
	vga_present();
}

void vga_setLineCompare(unsigned int scanline) {
	// Divide by 2 because this VGA video mode uses doubled lines
	vga_line_compare = scanline / 2;
	vga_present();
}

void vga_initialize() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		errorQuit(SDL_GetError(), 0);
	
	vga_window = SDL_CreateWindow(
		"OpenVikings",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		320, 240,
		0);
	if (vga_window == nullptr)
		errorQuit(SDL_GetError(), 0);

	vga_renderer = SDL_CreateRenderer(vga_window, -1, 0);
	if (vga_renderer == nullptr)
		errorQuit(SDL_GetError(), 0);

	vga_texture = SDL_CreateTexture(
		vga_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
		320, 240);
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

	uint16_t vram_line_offset = vga_start_address;

	for (int y = 0; y < 240; ++y) {
		if (y == vga_line_compare)
			vram_line_offset = 0;

		uint16_t vram_offset = vram_line_offset;

		for (int x = 0; x < 320; x += 4) {
			tex_pixels[x+0] = vga_palette[vga_framebuffer[0][vram_offset]];
			tex_pixels[x+1] = vga_palette[vga_framebuffer[1][vram_offset]];
			tex_pixels[x+2] = vga_palette[vga_framebuffer[2][vram_offset]];
			tex_pixels[x+3] = vga_palette[vga_framebuffer[3][vram_offset]];
			++vram_offset;
		}
		vram_line_offset += 344/4;
		tex_pixels += tex_pitch / sizeof(uint32_t);
	}

	SDL_UnlockTexture(vga_texture);

	SDL_RenderCopy(vga_renderer, vga_texture, nullptr, nullptr);

	SDL_RenderPresent(vga_renderer);
}
