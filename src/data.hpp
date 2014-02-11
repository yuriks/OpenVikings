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

	const uint8_t* getBlock(size_t size) {
		assert(pos + size <= buffer_end);
		const uint8_t* ret = pos;
		pos += size;
		return ret;
	}

	uint8_t read8() {
		return *getBlock(1);
	}

	uint16_t read16() {
		return load16LE(getBlock(2));
	}

	uint32_t read32() {
		return load32LE(getBlock(4));
	}

	void skip(size_t n) {
		pos += n;
	}

	size_t bytesRemaining() const {
		assert(buffer_end >= pos);
		return buffer_end - pos;
	}
};

void openDataFiles();
void closeDataFiles();
std::vector<uint8_t> loadChunk(ChunkId chunk_id);
std::vector<uint8_t> decompressChunk(ChunkId chunk_id);
