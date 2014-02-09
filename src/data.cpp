#include "data.hpp"

#include "vikings.hpp"
#include "main.hpp"
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <array>
#include <algorithm>
#include <vector>

static FILE* data_file = nullptr;
static FILE* exe_file = nullptr;

void openDataFiles() {
	data_file = std::fopen("data.dat", "rb");
	if (data_file == nullptr) {
		errorQuit("Failed to open data.dat", errno);
	}

	exe_file = std::fopen("vikings.exe", "rb");
	if (exe_file == nullptr) {
		errorQuit("Failed to open vikings.exe", errno);
	}
}

void closeDataFiles() {
	if (data_file != nullptr) {
		std::fclose(data_file);
		data_file = nullptr;
	}

	if (exe_file != nullptr) {
		std::fclose(exe_file);
		exe_file = nullptr;
	}
}

static inline uint8_t fileRead8(std::FILE* f) {
	uint8_t buf;
	if (std::fread(&buf, sizeof(uint8_t), 1, f) != 1) {
		errorQuit("Failed to read from file", errno);
	}
	return buf;
}

static inline uint16_t fileRead16LE(std::FILE* f) {
	uint8_t buf[sizeof(uint16_t)];
	if (std::fread(buf, sizeof(uint16_t), 1, f) != 1) {
		errorQuit("Failed to read from file", errno);
	}
	return load16LE(buf);
}

static inline uint32_t fileRead32LE(std::FILE* f) {
	uint8_t buf[sizeof(uint32_t)];
	if (std::fread(buf, sizeof(uint32_t), 1, f) != 1) {
		errorQuit("Failed to read from file", errno);
	}
	return load32LE(buf);
}

std::vector<uint8_t> loadChunk(ChunkId chunk_id) {
	if (std::fseek(data_file, chunk_id * sizeof(uint32_t), SEEK_SET) != 0) {
		errorQuit("Failed to seek file", errno);
	}

	const uint32_t chunk_offset = fileRead32LE(data_file);
	const uint32_t next_chunk_offset = fileRead32LE(data_file);
	const size_t chunk_size = next_chunk_offset - chunk_offset;

	if (std::fseek(data_file, chunk_offset, SEEK_SET) != 0) {
		errorQuit("Failed to seek file", errno);
	}

	std::vector<uint8_t> buffer(chunk_size);
	if (std::fread(buffer.data(), 1, buffer.size(), data_file) != buffer.size()) {
		errorQuit("Failed to read from file", errno);
	}

	return buffer;
}

/// Decompresses a chunk from the datafile to the buffer dst (with maximum size dst_size). Returns the actual size of the loaded data.
size_t decompressChunk(const ChunkId chunk_id, uint8_t *const dst, const size_t dst_size) {
	// Original function ignored this id. Assert to ensure nothing funny goes through.
	assert(chunk_id != 0xFFFA);

	const std::vector<uint8_t> src_buffer = loadChunk(chunk_id);
	MemoryStream src(src_buffer.data(), src_buffer.size());

	const uint16_t decompressed_data_len = src.read16() + 1u;
	if (decompressed_data_len > dst_size) {
		errorQuit("Buffer overrun while loading chunk", chunk_id);
	}

	std::array<uint8_t, 4096> scratch;
	scratch.fill(0);

	size_t dst_i = 0;
	size_t scratch_i = 0;

	while (true) {
		uint8_t bits = src.read8();

		for (int i = 0; i < 8; ++i) {
			if ((bits & 1) != 0) {
				// Verbatim byte
				const uint8_t symbol = src.read8();

				scratch[scratch_i] = symbol;
				scratch_i = (scratch_i + 1) % scratch.size();

				dst[dst_i++] = symbol;
			} else {
				// Copy bytes from scratch area
				const uint16_t copy_op = src.read16();

				const size_t copy_len = std::min((copy_op >> 12) + 3u, decompressed_data_len - dst_i);
				size_t copy_src = copy_op & 0xFFF; // mod 4096, exactly the size of the scratch

				for (size_t j = 0; j < copy_len; ++j) {
					const uint8_t symbol = scratch[copy_src];
					copy_src = (copy_src + 1u) % scratch.size();

					scratch[scratch_i] = symbol;
					scratch_i = (scratch_i + 1u) % scratch.size();

					dst[dst_i++] = symbol;
				}
			}

			if (dst_i >= decompressed_data_len) {
				break;
			}
			bits >>= 1;
		}

		if (dst_i >= decompressed_data_len) {
			// If this fails then a buffer overrun occurred. Bad!
			assert(dst_i == decompressed_data_len);
			break;
		}
	}

#if 1
	char tmpfn[30];
	sprintf(tmpfn, "chunks/chunk%03x.bin", chunk_id);
	std::FILE* df = std::fopen(tmpfn, "wb");
	std::fwrite(dst, decompressed_data_len, 1, df);
	std::fclose(df);
#endif

	return decompressed_data_len;
}