#include "decoding.hpp"

void deplaneMasked8x8Tile(const uint8_t* src_ptr, uint8_t* dst_ptr) {
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

	const uint8_t(&src)[8][9] = *reinterpret_cast<const uint8_t(*)[8][9]>(src_ptr);
	uint8_t(&dst)[8][9] = *reinterpret_cast<uint8_t(*)[8][9]>(dst_ptr);

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
	dst[2][0] = X >> 8 & 0xFF;
	dst[3][0] = X >> 0 & 0xFF;
	dst[4][0] = x >> 24 & 0xFF;
	dst[5][0] = x >> 16 & 0xFF;
	dst[6][0] = x >> 8 & 0xFF;
	dst[7][0] = x >> 0 & 0xFF;
}
