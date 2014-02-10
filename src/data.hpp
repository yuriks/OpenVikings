#pragma once
#include <cassert>
#include <cstdint>
#include <vector>
#include "vikings.hpp"

typedef uint16_t ChunkId;

class MemoryStream {
	const uint8_t* buffer_start;
	const uint8_t* buffer_end;
	const uint8_t* pos;

public:
	MemoryStream(const uint8_t* buffer, const size_t buffer_size)
		: buffer_start(buffer), buffer_end(buffer + buffer_size), pos(buffer)
	{}

	uint8_t read8() {
		assert(pos + 1 <= buffer_end);
		return *pos++;
	}

	uint16_t read16() {
		assert(pos + 2 <= buffer_end);
		uint16_t val = load16LE(pos);
		pos += 2;
		return val;
	}

	uint32_t read32() {
		assert(pos + 4 <= buffer_end);
		uint16_t val = load32LE(pos);
		pos += 4;
		return val;
	}
};

void openDataFiles();
void closeDataFiles();
std::vector<uint8_t> loadChunk(ChunkId chunk_id);
std::vector<uint8_t> decompressChunk(ChunkId chunk_id);
