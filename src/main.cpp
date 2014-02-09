#include "main.hpp"

#include "vga_emu.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "data.hpp"
#include <array>

static const ChunkId FONT_CHUNK = 2;
std::array<uint8_t, 0x1680> font_graphics;

static void drawTextTile(int c, uint8_t *const dst, const ptrdiff_t dst_stride) {
	c -= 16;

	MemoryStream src(&font_graphics[c * 8 * 9], 8 * 9);

	for (int plane = 0; plane < 4; ++plane) {
		uint8_t* plane_addr = dst + plane;
		for (int y_a = 0; y_a < 8; y_a += 4) {
			uint8_t mask = src.read8();
			for (int y_b = 0; y_b < 4; ++y_b) {
				uint8_t* line_addr = plane_addr + (y_a + y_b) * dst_stride;

				for (int x = 0; x < 8; x += 4) {
					uint8_t pixel = src.read8();
					if ((mask & 0x80) != 0) {
						line_addr[x] = pixel;
					}
					mask <<= 1;
				}
			}
		}
	}
}

static void deinitializeGame() {
	closeDataFiles();
	vga_deinitialize();
}

void errorQuit(const char* message, unsigned int code) {
	std::printf("*** OpenVikings encoutered a runtime error:\n\n%s\nError code: %ud\n", message, code);

	deinitializeGame();
	exit(1);
}

int main() {
	vga_initialize();
	openDataFiles();

	decompressChunk(FONT_CHUNK, font_graphics.data(), font_graphics.size());

	vga_setPaletteColor(0, 0x000000);
	vga_setPaletteColor(1, 0x000000);
	vga_setPaletteColor(2, 0xFBFBFB);
	vga_setPaletteColor(3, 0x920000);

	while (true) {
		input_handleSDLEvents();
		if (input_quit_requested) {
			break;
		}

		drawTextTile('A', vga_getPosPtr(0, 0), vga_width);

		vga_present();
	}

	while (true) {
		input_handleSDLEvents();
		if (input_quit_requested) {
			break;
		}

		vga_present();
	}

	deinitializeGame();
}
