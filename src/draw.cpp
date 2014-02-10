#include "draw.hpp"
#include "data.hpp"

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
