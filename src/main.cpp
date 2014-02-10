#include "main.hpp"

#include "vga_emu.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "data.hpp"
#include <array>
#include <vector>

static const ChunkId FONT_CHUNK = 2;
std::array<uint8_t, 0x1680> font_graphics;

// Expands an 8-bit number x, yielding a 32-bit number by inserting 3 0-bits between each bit of the original.
static uint32_t expandBits1To4(uint32_t x) {
	x = (x | (x << 12)) & 0x000F000F; // x = ---- ---- ---- 7654 ---- ---- ---- 3210
	x = (x | (x <<  6)) & 0x03030303; // x = ---- --76 ---- --54 ---- --32 ---- --10
	x = (x | (x <<  3)) & 0x11111111; // x = ---7 ---6 ---5 ---4 ---3 ---2 ---1 ---0
	return x;
}

static void deplaneMasked8x8Tile(const uint8_t* src_ptr, uint8_t* dst_ptr) {
	/*
	This kind of graphics data is stored sequentially in memory like this:

	M0 | A0 B0 C0 D0 E0 F0 G0 H0
	m0 | a0 b0 c0 d0 e0 f0 g0 h0
	M1 | A1 B1 C1 D1 E1 F1 G1 H1
	m1 | a1 b1 c1 d1 e1 f1 g1 h1
	M2 | A2 B2 C2 D2 E2 F2 G2 H2
	m2 | a2 b2 c2 d2 e2 f2 g2 h2
	M3 | A3 B3 C3 D3 E3 F3 G3 H3
	m3 | a3 b3 c3 d3 e3 f3 g3 h3

	M0-3 and m0-3 are masks for the following pixels in the line. This format
	is the most efficient for drawing to VGA planar memory. We want to
	transform it to a linear format:

	N0 | A0 A1 A2 A3 B0 B1 B2 B3
	N1 | C0 C1 C2 C3 D0 D1 D2 D3
	N2 | E0 E1 E2 E3 F0 F1 F2 F3
	N3 | G0 G1 G2 G3 H0 H1 H2 H3
	n0 | a0 a1 a2 a3 b0 b1 b2 b3
	n1 | c0 c1 c2 c3 d0 d1 d2 d3
	n2 | e0 e1 e2 e3 f0 f1 f2 f3
	n3 | g0 g1 g2 g3 h0 h1 h2 h3

	N0-3 and n0-3 are the new masks for each line. What this operation entails
	is de-interleaving the upper and bottom halves of the tile, then
	transposing each half and concatenating every pair of lines. However, it's
	easier and faster to just scan the original matrix, calculating the
	destination address as below.

	The bitmasks are arranged the same way, but the whole operation is more
	involved due to random access not being possible.
	*/

	const uint8_t(& src)[8][9] = *reinterpret_cast<const uint8_t(*)[8][9]>(src_ptr);
	uint8_t(& dst)[8][9] = *reinterpret_cast<uint8_t(*)[8][9]>(dst_ptr);

	// Re-arrange image pixels
	for (int l = 0; l < 8; l += 2) {
		for (int c = 0; c < 8; ++c) {
			const int idx = c % 2 * 4 + l / 2;
			dst[c / 2][1 + idx] = src[l][1 + c];
			dst[c / 2 + 4][1 + idx] = src[l + 1][1 + c];
		}
	}

	// Re-arrange mask bits
	const uint8_t M0 = src[0][0], m0 = src[1][0]; // A0 B0 C0 D0 E0 F0 G0 H0
	const uint8_t M1 = src[2][0], m1 = src[3][0]; // A1 B1 C1 D1 E1 F1 G1 H1
	const uint8_t M2 = src[4][0], m2 = src[5][0]; // A2 B2 C2 D2 E2 F2 G2 H2
	const uint8_t M3 = src[6][0], m3 = src[7][0]; // A3 B3 C3 D3 E3 F3 G3 H3

	// These calls expand the bits, generating the following values, which are then ANDed together:
	// A0 __ __ __ B0 __ __ __ | C0 __ __ __ D0 __ __ __ | E0 __ __ __ F0 __ __ __ | G0 __ __ __ H0 __ __ __
	// __ A1 __ __ __ B1 __ __ | __ C1 __ __ __ D1 __ __ | __ E1 __ __ __ F1 __ __ | __ G1 __ __ __ H1 __ __
	// __ __ A2 __ __ __ B2 __ | __ __ C2 __ __ __ D2 __ | __ __ E2 __ __ __ F2 __ | __ __ G2 __ __ __ H2 __
	// __ __ __ A3 __ __ __ B3 | __ __ __ C3 __ __ __ D3 | __ __ __ E3 __ __ __ F3 | __ __ __ G3 __ __ __ H3
	const uint32_t X = (expandBits1To4(M0) << 3) | (expandBits1To4(M1) << 2) | (expandBits1To4(M2) << 1) | (expandBits1To4(M3) << 0);
	const uint32_t x = (expandBits1To4(m0) << 3) | (expandBits1To4(m1) << 2) | (expandBits1To4(m2) << 1) | (expandBits1To4(m3) << 0);

	dst[0][0] = X >> 24 & 0xFF;
	dst[1][0] = X >> 16 & 0xFF;
	dst[2][0] = X >>  8 & 0xFF;
	dst[3][0] = X >>  0 & 0xFF;
	dst[4][0] = x >> 24 & 0xFF;
	dst[5][0] = x >> 16 & 0xFF;
	dst[6][0] = x >>  8 & 0xFF;
	dst[7][0] = x >>  0 & 0xFF;
}

static void drawTextTile(int c, uint8_t *const dst, const ptrdiff_t dst_stride) {
	c -= 16;

	MemoryStream src(&font_graphics[c * 8 * 9], 8 * 9);

	uint8_t* line = dst;
	for (int y = 0; y < 8; ++y) {
		uint8_t mask = src.read8();
		for (int x = 0; x < 8; ++x) {
			uint8_t pixel = src.read8();
			if ((mask & 0x80) != 0) {
				line[x] = pixel;
			}
			mask <<= 1;
		}
		line += dst_stride;
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

	{
		std::vector<uint8_t> font_buffer(font_graphics.size());
		font_buffer.resize(decompressChunk(FONT_CHUNK, font_buffer.data(), font_buffer.size()));
		assert(font_graphics.size() == font_buffer.size());

		for (size_t i = 0; i < font_buffer.size(); i += 9*8) {
			deplaneMasked8x8Tile(&font_buffer[i], &font_graphics[i]);
		}
	}

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
