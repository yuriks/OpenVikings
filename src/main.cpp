#include "vikings.hpp"
#include "main.hpp"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <array>
#include <SDL2/SDL_timer.h>
#include <numeric>

#include "vga_emu.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "vars.hpp"

void errorQuit(const char* message, int code) {
	// TODO
}

// Note: Originally this function returned es:di on the end of the buffer. Now I just return the pointer to it
uint8_t* decompressChunk(uint16_t chunk_id, uint8_t* dst, size_t dst_size) {
	struct Local {
		static void errBufOverrun() {
			errorQuit("\nGlobal buffer overrun on file read.\n", 0);
		}
		static void errReadData() {
			errorQuit("\nUnable to read data file.\n", errno);
		}
	};

	uint8_t* dst_copy = dst;

	if (chunk_id == 0xFFFA)
		return dst;

	if (std::fseek(data_file, chunk_id * 4, SEEK_SET) != 0)
		Local::errReadData();

	if (std::fread(&chunk_offsets, sizeof(int32_t), 2, data_file) != 2)
		Local::errReadData();
	if (std::fseek(data_file, chunk_offsets[0], SEEK_SET) != 0)
		Local::errReadData();
	if (std::fread(&decoded_data_len, sizeof(uint16_t), 1, data_file) != 1)
		Local::errReadData();

	if (decoded_data_len > dst_size)
		Local::errBufOverrun();

	size_t read_size = chunk_offsets[1] - chunk_offsets[0];
	if (read_size + 0x1000 >= alloc_seg6_size)
		Local::errBufOverrun();

	if (std::fread(alloc_seg6 + 0x1000, read_size, 1, data_file) != 1)
		Local::errReadData();

	std::memset(alloc_seg6, 0, 0x1000);

	uint8_t* src = alloc_seg6;
	uint16_t bx = 0;
	uint16_t dx = decoded_data_len;
	uint16_t src_off = 0x1000;

	while (true) {
		uint8_t al = src[src_off++];

		for (int i = 0; i < 8; ++i) {
			if (al >> i & 1) {
				src[bx] = src[src_off];
				++bx &= 0x0FFF;
				*dst++ = src[src_off];
				if (--dx >= decoded_data_len) {
#if 0
					char tmpfn[30];
					sprintf(tmpfn, "chunks/chunk%03x.bin", chunk_id);
					std::FILE* df = std::fopen(tmpfn, "wb");
					std::fwrite(dst_copy, decoded_data_len+1, 1, df);
					std::fclose(df);
#endif
					return dst;
				}
				++src_off;
			} else {
				uint16_t new_src_off = load16LE(&src[src_off]);
				src_off += 2;

				uint16_t cx = (new_src_off >> 12) + 3;
				new_src_off &= 0x0FFF;

				do {
					src[bx] = src[new_src_off];
					++bx &= 0x0FFF;
					*dst++ = src[new_src_off];
					if (--dx >= decoded_data_len) {
#if 0
						char tmpfn[30];
						sprintf(tmpfn, "chunks/chunk%03x.bin", chunk_id);
						std::FILE* df = std::fopen(tmpfn, "wb");
						std::fwrite(dst_copy, decoded_data_len+1, 1, df);
						std::fclose(df);
#endif
						return dst;
					}
					++new_src_off &= 0x0FFF;
				} while (--cx != 0);
			}
		}
	}
}

void loadImage(uint16_t chunk_id, uint16_t offset) {
	struct Local {
		static void errReadData() {
			errorQuit("\nUnable to read data file.\n", errno);
		}
	};

	if (chunk_id == 0xFFFA)
		return;

	if (std::fseek(data_file, chunk_id * 4, SEEK_SET) != 0)
		Local::errReadData();

	if (std::fread(&chunk_offsets, sizeof(int32_t), 2, data_file) != 2)
		Local::errReadData();
	if (std::fseek(data_file, chunk_offsets[0], SEEK_SET) != 0)
		Local::errReadData();
	if (std::fread(&decoded_data_len, sizeof(uint16_t), 1, data_file) != 1)
		Local::errReadData();

	if (std::fread(ptr3, decoded_data_len, 4, data_file) != 4)
		Local::errReadData();

	vga_copyPlanesToVRAM(ptr3, offset, decoded_data_len);
}

int main() {
	vga_initialize();

	vga_deinitialize();
}
