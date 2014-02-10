#include "draw.hpp"
#include "data.hpp"
#include "decoding.hpp"
#include <array>
#include <algorithm>

void fillSurface(uint8_t color, const DrawSurface& surface) {
	uint8_t* line = surface.at(0, 0);
	for (int y = 0; y < surface.height; ++y) {
		std::fill_n(line, surface.width, color);
	}
}

void drawMasked8x8Tile(uint8_t* tile_data, const DrawSurface& surface, int x, int y) {
	MemoryStream src(tile_data, 8 * 9);

	uint8_t* line = surface.at(x, y);
	for (int y = 0; y < 8; ++y) {
		uint8_t mask = src.read8();
		for (int x = 0; x < 8; ++x) {
			uint8_t pixel = src.read8();
			if ((mask & 0x80) != 0) {
				line[x] = pixel;
			}
			mask <<= 1;
		}
		line += surface.stride;
	}
}

static const ChunkId FONT_CHUNK = 2;
std::array<uint8_t, 0x1680> font_graphics;

void loadFont() {
	std::vector<uint8_t> font_buffer = decompressChunk(FONT_CHUNK);
	assert(font_graphics.size() == font_buffer.size());

	for (size_t i = 0; i < font_buffer.size(); i += 9 * 8) {
		deplaneMasked8x8Tile(&font_buffer[i], &font_graphics[i]);
	}
}

void drawTextTile(int c, const DrawSurface& surface, int x, int y) {
	assert(c >= 16 && c < 112);
	drawMasked8x8Tile(&font_graphics[(c - 16) * 8 * 9], surface, x, y);
}