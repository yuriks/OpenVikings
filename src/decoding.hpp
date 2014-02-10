#pragma once
#include <cstdint>

// Expands an 8-bit number x, yielding a 32-bit number by inserting 3 0-bits between each bit of the original.
inline uint32_t expandBits1To4(uint32_t x) {
	x = (x | (x << 12)) & 0x000F000F; // x = ---- ---- ---- 7654 ---- ---- ---- 3210
	x = (x | (x << 6)) & 0x03030303; // x = ---- --76 ---- --54 ---- --32 ---- --10
	x = (x | (x << 3)) & 0x11111111; // x = ---7 ---6 ---5 ---4 ---3 ---2 ---1 ---0
	return x;
}

void deplaneMasked8x8Tile(const uint8_t* src_ptr, uint8_t* dst_ptr);
