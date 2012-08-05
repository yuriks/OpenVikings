// Project-wide useful definitions
#pragma once

#include <cstdint>

#define BIT(i) (1 << (i))
#define RANGE(x) (std::begin(x)), (std::end(x))

static inline uint16_t load16LE(const uint8_t* d) {
	return d[0] | d[1] << 8;
}

static inline uint32_t load32LE(const uint8_t* d) {
	return d[0] | d[1] << 8 | d[2] << 16 | d[3] << 24;
}

static inline void store16LE(uint8_t* d, uint16_t val) {
	d[0] = (val >> 0) & 0xFF;
	d[1] = (val >> 8) & 0xFF;
}

static inline void store32LE(uint8_t* d, uint32_t val) {
	d[0] = (val >>  0) & 0xFF;
	d[1] = (val >>  8) & 0xFF;
	d[2] = (val >> 16) & 0xFF;
	d[3] = (val >> 24) & 0xFF;
}
