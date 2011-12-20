#include "main.hpp"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <SDL_timer.h>

#include "vga_emu.hpp"
#include "vars.hpp"

// TODO
void hookKeyboard();
void unhookKeyboard();
void restoreErrorInt();
uint32_t curSystemTime();

// TODO
// addr seg00:6528
void hookKeyboard() {}
// addr seg00:6546
void unhookKeyboard() {}

// addr seg00:686F
void restoreVideo() {
	if (saved_video_mode != 0xFF) {
		// TODO vga_deinitialize();
	}
	video_initialized = false;
}

// TODO
// addr seg00:292F
void restoreErrorInt() {}
uint32_t curSystemTime() { return 0; }

inline uint16_t load16LE(const uint8_t* d) {
	return d[0] | d[1] << 8;
}

inline uint32_t load32LE(const uint8_t* d) {
	return d[0] | d[1] << 8 | d[2] << 16 | d[3] << 24;
}

inline void store16LE(uint8_t* d, uint16_t val) {
	d[0] = (val >> 0) & 0xFF;
	d[1] = (val >> 8) & 0xFF;
}

inline void store32LE(uint8_t* d, uint32_t val) {
	d[0] = (val >>  0) & 0xFF;
	d[1] = (val >>  8) & 0xFF;
	d[2] = (val >> 16) & 0xFF;
	d[3] = (val >> 24) & 0xFF;
}

// addr seg00:2948
void hookInts() {
	// init memory and segments();

	hookKeyboard();

	start_time = curSystemTime();
}

// addr seg00:0DBA
void errorQuit(const char* msg, uint16_t error_code) {
	unhookKeyboard();
	// TODO ??? call

	std::fclose(data_file);

	restoreVideo();
	restoreErrorInt();

	std::printf("\n*** An Error has occured while running PC Vikings ***\n");
	std::printf("%s", msg);
	if (error_code != 0) {
		std::printf("\nError code is: %04x\n", error_code);
	}

	std::exit(0);
}

// addr seg00:2989
void hwCheck() {
	struct Local {
		static void errReadData() {
			errorQuit("\nUnable to read data file.\n", errno);
		}

		static void errOpenData() {
			errorQuit("\nCannot find 'data.dat' file in the current directory.\n", errno);
		}

		static void errCopyprotect() {
			errorQuit("\nCopy protection failure. Please run setup.\n", 0);
		}

		static void errCorruptData() {
			errorQuit("\nData.dat has been corrupted. Please re-install.\n", 0);
		}

		static void errVgaCard() {
			errorQuit("\nPC Vikings must be run on a system with a VGA card.\n", 0);
		}

		static void errCpu386() {
			errorQuit("\nPC Vikings requires a 386 (or better) microprocessor to run.\n", 0);
		}
	};

	data_file = std::fopen("data.dat", "rb");
	if (!data_file)
		Local::errOpenData();

	if (std::fread(&chunk_offsets, sizeof(int32_t), 2, data_file) != 2)
		Local::errReadData();
	if (std::fseek(data_file, chunk_offsets[0], SEEK_SET) != 0)
		Local::errReadData();

	if (std::fread(&data_header1, sizeof(DataHeader1), 1, data_file) != 1)
		Local::errReadData();
	
	if (data_header1.magic != 0x6969)
		Local::errCorruptData();

	// insert funky system/BIOS checksum here
	
	if (false /* systemChecksum != data_header1.copy_checksum */)
		Local::errCopyprotect();

	data_header1_snd1 = data_header1.snd1;
	data_header1_snd2 = data_header1.snd2;

	if (false /* check video here */)
		Local::errVgaCard();

	if (false /* check cpu here */)
		Local::errCpu386();

	// calibrate joysticks here
}

// addr seg00:0E85
void roundToNextSeg(uint8_t** seg, uint16_t* off) {
	// What am I supposed to do here?
	*seg += *off;
	*off = 0;
}

// Note: Originally this function returned es:di on the end of the buffer. Now I just return the pointer to it
// addr seg00:0982
uint8_t* decompressChunk(uint16_t chunk_id, uint8_t* dst) {
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

// addr seg00:2AB8
void allocMemAndLoadData() {
	tileset_data = new uint8_t[tileset_data_size];
	alloc_seg2 = new uint8_t[alloc_seg2_size];
	metatile_data = new uint8_t[metatile_data_size];
	tilemap_data = new uint8_t[tilemap_data_size];
	alloc_seg6 = new uint8_t[alloc_seg6_size];
	alloc_seg11 = new uint8_t[alloc_seg11_size];

	// Load sound data if sound enabled
	if (!(data_header1_snd1 && data_header1_snd2 && 0x8000)) {
		uint8_t* buf = new uint8_t[soundData_size];
		soundData.seg = buf;
		soundData.offset = 0;

		// Note: roundToNextSeg not needed because segments are for 16-bit losers
		// I'm not sure these are quite right, they seem to point to the end of the data
		// but looking at the original that's effectively what happens?
		soundData0End = buf = decompressChunk(0x1C7 + data_header1.field_6, buf);
		soundData1End = buf = decompressChunk(0x20C + data_header1.field_8, buf);
		soundData2End = decompressChunk(0x207 + data_header1.field_8, buf);
	}

	ptr3 = new uint8_t[ptr3_size];

	world_data = new uint8_t[world_data_size];

	loaded_world_chunk = 0x1C6;
	decompressChunk(loaded_world_chunk, world_data);

	decompressChunk(4, chunk_buffer0);
	decompressChunk(5, chunk_buffer1);
	decompressChunk(6, chunk_buffer2);
	decompressChunk(7, chunk_buffer3);
	decompressChunk(8, chunk_buffer4);
	decompressChunk(9, chunk_buffer5);
	decompressChunk(10, chunk_buffer6);
	decompressChunk(11, chunk_buffer7);
	decompressChunk(12, chunk_buffer8);
	decompressChunk(13, chunk_buffer9);
	decompressChunk(14, chunk_buffer10);
	decompressChunk(2, chunk_buffer11);

	current_password[0] = 'S';
	current_password[1] = 'T';
	current_password[2] = 'R';
	current_password[3] = 'T';
	// TODO bunch of variable sets
}

// addr seg00:0E35
void freeMemAndQuit() {
	unhookKeyboard();
	// TODO one call

	delete[] tileset_data;
	delete[] metatile_data;
	delete[] alloc_seg2;
	delete[] tilemap_data;
	delete[] world_data;
	delete[] ptr3;

	std::fclose(data_file);

	restoreVideo();
	restoreErrorInt();

	std::exit(0);
}

// addr seg02:071A
void clearSoundBuffers() {
	// TODO
}

// addr seg00:7561
void initSound() {
	// Major TODO

	did_init_timer = 0;
	// TODO word_32858
	clearSoundBuffers();

	if (!(data_header1_snd1 && data_header1_snd2 && 0x8000)) {
		// sound enabled
		did_init_timer = 1;
	} else {
		did_init_timer = 1;
	}
}

// addr seg00:67FF
void initVideo() {
	// TODO call vga_initialize here

	// Skip some mode initialization, It'll always work on planar mode

	vga_setStartAddress(0x1600);
	// Some is hardcoded
	// TODO ? more mode setting

	vga_fillVRAM(0xF, 0, 0, 0xFFFF);

	video_initialized = true;
}

// addr seg00:0F03
void fadePalStep() {
	Color c_and;
	for (int c = 0; c < 3; ++c) {
		c_and.rgb[c] = color1.rgb[c] | color2.rgb[c];
	}

	for (int i = 0; i < 0x100; ++i) {
		for (int c = 0; c < 3; ++c) {
			int8_t x = palette1[i].rgb[c];

			x -= c_and.rgb[c];

			if (x < 0)
				x = 0;
			if ((x & 0x40) != 0)
				x = 0x3F;

			palette2[i].rgb[c] = x;
		}
	}
}

inline uint16_t calcHeightTileMults(int i) {
	return 0x1600 + (i % 78) * 0x2B0;
}

// addr seg00:6775
void updateVgaBuffer() {
	uint16_t si = video_levelY + video_screenShakeY;
	if (si > video_level_max_y)
		si = video_levelY - video_screenShakeY;

	uint16_t cx = calcHeightTileMults(si / 8 + video_frontBufBase / 2);
	uint16_t bx = heightPixelMults[si % 8] + cx;

	uint16_t ax = video_levelX + video_screenShakeX;
	if (ax > video_level_max_x)
		ax = video_levelX - video_screenShakeX;

	video_pixelPan = (ax % 4) * 2;
	bx += (ax / 4) + 8;

	vga_setStartAddress(bx);

	// TODO vars
}

// addr seg00:0E99
void fadePalKeepFirst() {
	Color c_and;
	for (int c = 0; c < 3; ++c) {
		c_and.rgb[c] = color1.rgb[c] | color2.rgb[c];
	}

	for (int i = 0; i < 0x100; ++i) {
		for (int c = 0; c < 3; ++c) {
			int8_t x = palette1[i].rgb[c];

			x -= c_and.rgb[c];

			if (x < 0)
				x = 0;
			if ((x & 0x40) != 0)
				x = 0x3F;

			palette2[i].rgb[c] = x;
		}

		// Copy entries 1-15 unchanged
		for (; i < 0x10; ++i) {
			palette2[i] = palette1[i];
		}
	}
}

// addr seg00:2CE4
void zero_word_288C4() {
	// TODO
	//word_288F4 = 0;
	//word_288F6 = 0;
	//word_288F8 = 0;
	//std::memset(word_288C4, 0, 0x18);
	//word_28903 = 6;
	//word_28905 = 6;
	//word_28907 = 6;
	//word_28909 = 0;
	//word_2890B = 0;
	//word_2890D = 0;
}

// addr seg00:08B8
void sub_108B8() {
	zero_word_288C4();
	level_header.next_level_id = 0x27;
	// TODO word_288A2 = 0;
}

// addr seg00:11B1
void loadLevelHeader(uint16_t level_id) {
	uint16_t chunk_id = levelWorldChunks[level_id];
	if (chunk_id != 0xFFFF && chunk_id != loaded_world_chunk) {
		loaded_world_chunk = chunk_id;
		decompressChunk(chunk_id, world_data);
	}
	decompressChunk(levelTileChunks[level_id], reinterpret_cast<uint8_t*>(&level_header));
}

// addr seg00:1383
uint16_t seekLoadList() {
	uint16_t di;
	for (di = 0; load16LE(&level_header.data_load_list[di]) != 0xFFFF; di += 14);
	return di + 2;
}

// addr seg00:12AE
uint16_t loadPaletteList(uint16_t di) {
	while (true) {
		uint16_t ax = load16LE(&level_header.data_load_list[di]);
		di += 2;

		if (ax == 0xFFFF)
			break;

		uint16_t offset = level_header.data_load_list[di++];
		decompressChunk(ax, reinterpret_cast<uint8_t*>(&palette1[offset]));
	}

	for (int i = 0; i < 16; ++i) {
		palette1[16*i].rgb[0] = 0;
		palette1[16*i].rgb[1] = 0;
		palette1[16*i].rgb[2] = 0;
	}

	stru_2AA88 = palette1[3];

	fadePalKeepFirst();

	return di;
}

// addr seg00:133A
uint16_t processLoadList(uint16_t di) {
	byte_2AA63 = level_header.data_load_list[di];
	di += 2;

	uint16_t si = 0;

	uint8_t al;
	while ((al = level_header.data_load_list[di++]) != 0) {
		byte_2AA64[si] = al;
		byte_2AA6C[si] = al;
		byte_2AA74[si] = level_header.data_load_list[di++ + 1];
		byte_2AA7C[si] = level_header.data_load_list[di++ + 2];

		for (; load16LE(&level_header.data_load_list[di]) != 0xFFFF; di += 2);
		di += 2;

		si += 1;
	}

	return di;
}

// addr seg00:167A
uint16_t loadChunkList(uint16_t di) {
	uint16_t si = 0;
	uint8_t* bx = alloc_seg11;

	while (true) {
		uint16_t ax = load16LE(&level_header.data_load_list[di]);
		di += 2;
		if (ax == 0xFFFF)
			break;
		loaded_chunks11[si] = ax;
		loaded_chunks_end11[si] = (bx - alloc_seg11) + 1;
		bx = decompressChunk(ax, bx);

		di += 4;
		si++;
	}

	return di;
}

// addr seg00:16AE
uint16_t loadChunkList2(uint16_t di) {
	uint16_t si = 0;

	while (true) {
		uint16_t chunk_id = load16LE(&level_header.data_load_list[di]);
		if (chunk_id == 0xFFFF)
			break;
		loaded_chunks2[si] = chunk_id;

		uint8_t* dest = ptr1;
		loaded_chunks2_ptr[si] = dest;
		ptr1 = decompressChunk(chunk_id, dest);
		di += 5;
		si++;
	}

	return di;
}

// addr seg00:1784
void sub_11784() {
	// TODO word_288A2 = 0;
	// TODO word_288A4 = 0;
}

// addr seg00:1397
void sub_11397() {
	// TODO word_28886 = byte_30A1E[word_2AAAB];
	// TODO word_28888 = byte_30A24[word_2AAAB];
}

// addr seg00:37F1
void sub_137F1() {
	// TODO std::fill_n(word_29835, 20, 0);
	// TODO std::fill_n(word_29EED, 20, 0xFFFF);
}

// addr seg00:2FB3
void sub_12FB3() {
	// TODO std::fill_n(word_2892D, 128, 0);
}

// TODO
// addr seg00:3BBD
bool sub_13BBD(uint16_t si, uint16_t di) {
	return false;
}

// addr seg00:3BA5
void sub_13BA5() {
	// TODO word_28522 = 0xFFFF;
	uint16_t si = 0;
	uint16_t di = 0;

	while (sub_13BBD(si, di)) {
		si++;
		di += 14;
	}
}

// addr seg00:3A0E
void sub_13A0E() {
	// TODO sub_139EF();
	// TODO loc_13A94();
}

// addr seg00:15D2
void sub_115D2() {
	// TODO
}

// addr seg00:6595
void calculateRowOffsets(uint16_t level_width) {
	for (uint16_t si = 0; si < 0x100; ++si) {
		levelRowOffsets[si] = si*level_width*2;
	}
}

// addr seg00:13B0
void calcLevelSize() {

	level_width_tiles = level_header.level_width * 2;
	video_level_max_x = level_width_tiles * 8 - 320;

	level_height_tiles = level_header.level_height * 2;
	video_level_max_y = level_height_tiles * 8 - 176; // 176 = viewport height

	calculateRowOffsets(level_header.level_width);
}

// addr seg00:73C7
void assembleLevelTiles() {
	// TODO Clean this up

	// sets fs to allocSeg6 too
	if (level_header.level_flags & (LVLFLAG_BIT2 | LVLFLAG_BIT40)) {
		std::fill_n(reinterpret_cast<uint32_t*>(alloc_seg6), 0x3020, 0);
	} else {
		uint16_t y_max = level_header.level_height;
		uint16_t x_max = level_header.level_width;

		uint16_t bx = 0;
		uint16_t di = 0;

		for (uint16_t y = 0; y < y_max; ++y) {
			for (uint16_t x = 0; x < x_max; ++x) {
				uint16_t si = load16LE(&tilemap_data[bx++ *2]) % 1024;

				store32LE(&alloc_seg6[ di       *4], load32LE(&metatile_data[si*8  ]));
				store32LE(&alloc_seg6[(di+x_max)*4], load32LE(&metatile_data[si*8+4]));
				di++;
			}
			di += x_max;
		}

		std::copy_n(metatile_data, 15 * 4 * 2, tilemap_data_end);
	}
}

// addr seg00:0CD8
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

// addr seg00:1204
void loadChunks3() {
	if (level_header.level_flags & LVLFLAG_BIT2) {
		loadImage(level_header.tilemap_data_chunk, 0x20C8);
		loadImage(level_header.tilemap_data_chunk, 0x66A8);
		loadImage(level_header.tilemap_data_chunk, 0xAC88);
		loadImage(0x180, 0);
	} else {
		if (level_header.level_flags & LVLFLAG_BIT20) {
			loadImage(0x211, 0);
		} else {
			if (level_header.level_flags & LVLFLAG_BIT40) {
				loadImage(level_header.tilemap_data_chunk, 0x20C8);
				loadImage(level_header.tilemap_data_chunk, 0x66A8);
				loadImage(level_header.tilemap_data_chunk, 0xAC88);
				loadImage(0x213, 0);
				return;
			}
		}

		decompressChunk(level_header.tileset_data_chunk, tileset_data);
		decompressChunk(level_header.tileset_data_chunk + 1, alloc_seg2);
		tilemap_data_end = decompressChunk(level_header.tilemap_data_chunk, tilemap_data);
		decompressChunk(level_header.metatile_data_chunk, metatile_data);
	}
}

// addr seg00:0130
void waitForTimerInt() {
	// TODO implement actual DOS-accurate timing
	SDL_Delay(33);
	//while (waitForTimerInt > 0);
}

// addr seg00:0FE6
void loadPal2ToVga() {
	palette_action = PALACT_NONE;
	vga_setPalette(palette2);
}

// addr seg00:0FA0
void fadePalOut() {
	int8_t bx = 0;

	do {
		color1.rgb[0] = bx;
		color1.rgb[1] = bx;
		color1.rgb[2] = bx;
		fadePalStep();

		palette_action = PALACT_COPY;
		// TODO dat hack
		loadPal2ToVga();

		pal_pointer = palette2;
		updateVgaBuffer();
		waitForTimerInt();
	} while(++bx != 70);

	color1.rgb[0] = 0;
	color1.rgb[1] = 0;
	color1.rgb[2] = 0;
	pal_pointer = palette2;
}

// addr seg00:0F5D
void fadePalIn() {
	int8_t bx = 70;

	do {
		color1.rgb[0] = bx;
		color1.rgb[1] = bx;
		color1.rgb[2] = bx;
		fadePalStep();

		palette_action = PALACT_COPY;
		pal_pointer = palette2;

		updateVgaBuffer();

		// TODO dat hack
		loadPal2ToVga();
		waitForTimerInt();
	} while(--bx >= 0);

	color1.rgb[0] = 0;
	color1.rgb[1] = 0;
	color1.rgb[2] = 0;
	pal_pointer = palette1;
}

void drawTile(uint16_t tilegfx_off, uint16_t vram_off) {
	uint8_t flips = (tilegfx_off >> 4) & 3;

	tilegfx_off &= 0xFFC0;
	uint8_t* gfx = &tileset_data[tilegfx_off];

	if (flips == 0) { // No flip
		for (int p = 0; p < 4; ++p) {
			uint8_t* vram = &vga_framebuffer[p][vram_off];

			for (int y = 0; y < 8; ++y) {
				vram[0x56*y + 0] = *gfx++;
				vram[0x56*y + 1] = *gfx++;
			}
		}
	} else if (flips == 1) { // Horizontal flip
		for (int p = 3; p >= 0; --p) {
			uint8_t* vram = &vga_framebuffer[p][vram_off];

			for (int y = 0; y < 8; ++y) {
				vram[0x56*y + 1] = *gfx++;
				vram[0x56*y + 0] = *gfx++;
			}
		}
	} else if (flips == 2) { // Vertical flip
		for (int p = 0; p < 4; ++p) {
			uint8_t* vram = &vga_framebuffer[p][vram_off];

			for (int y = 7; y >= 0; --y) {
				vram[0x56*y + 0] = *gfx++;
				vram[0x56*y + 1] = *gfx++;
			}
		}
	} else /* flips == 2|1 */ { // Horizontal+Vertical flip
		for (int p = 3; p >= 0; --p) {
			uint8_t* vram = &vga_framebuffer[p][vram_off];

			for (int y = 7; y >= 0; --y) {
				vram[0x56*y + 1] = *gfx++;
				vram[0x56*y + 0] = *gfx++;
			}
		}
	}
}

void drawTilesLine(uint16_t tilemap_off, uint16_t vram_off) {
	for (int i = 0; i < 43; ++i) {
		drawTile(load16LE(&alloc_seg6[tilemap_off]), vram_off);
		tilemap_off += 2;
		vram_off += 2; // 2 * 4 = 8 pixels
	}
}

void cloneBackBuffer() {
	vga_vramCopy(video_backBufAddr, video_frontBufAddr, 0x2B0);
	vga_vramCopy(video_backBufAddr, video_resvBufAddr, 0x2B0);
}

// addr seg00:6DED
void drawInitialBg() {
	int16_t di = video_scroll_y_tiles - 1;
	if (di < 1)
		di = 0;

	uint16_t bx = levelRowOffsets[di];
	video_frontBuffer = di*2 + video_frontBufBase;
	video_resvBuffer  = di*2 + video_resvBufBase;
	video_backBuffer  = di*2 + video_backBufBase;

	int16_t dx = video_scroll_x_tiles - 1;
	if (dx < 1)
		dx = 0;

	bx += dx;
	dx += 4;

	for (uint16_t cx = 0; cx < 25; ++cx) {
		di = video_frontBuffer/2;
		di = calcHeightTileMults(di);
		di += dx*2;
		video_frontBufAddr = di;

		di = video_resvBuffer/2;
		di = calcHeightTileMults(di);
		di += dx*2;
		video_resvBufAddr = di;

		di = video_backBuffer/2;
		di = calcHeightTileMults(di);
		di += dx*2;
		video_backBufAddr = di;

		drawTilesLine(bx, di);
		cloneBackBuffer();

		bx += levelRowOffsets[2];

		video_frontBuffer += 2;
		video_resvBuffer += 2;
		video_backBuffer += 2;
	}
}

// addr seg00:1439
// this is a horrible name
void updateInitialBg() {
	if (!(level_header.level_flags & (LVLFLAG_BIT2 | LVLFLAG_BIT40))) {
		drawInitialBg();
	}
	updateVgaBuffer();
}

void video_clearVRAM() {
	vga_fillVRAM(0xF, 0, 0, 0xFFFF);
}

// addr seg00:1080
void loadNextLevel() {
	// TODO lotsa variables
	fadePalOut();
	// TODO sound_func1();
	// TODO zero_byte_31A4C();
	video_backBufBase = 0;
	video_frontBufBase = 52;
	video_resvBufBase = 104;
	ptr1 = ptr3;
	// TODO loadChunks1();
	sub_11784();
	video_clearVRAM();
	// TODO zero_byte_2892D();
	// TODO zero_byte_28836();
	previous_level = current_level;
	current_level = level_header.next_level_id;
	loadLevelHeader(current_level);
	// TODO setColor1And2();
	if (current_level != 0x25)
		zero_word_288C4();
	// TODO sub_117AD();
	loadChunks3();
	uint16_t di = seekLoadList();
	di = loadPaletteList(di);
	di = processLoadList(di);
	di = loadChunkList(di);
	di = loadChunkList2(di);
	sub_11397();
	sub_137F1();
	sub_12FB3();
	// ***? TODO sub_11446();
	calcLevelSize();
	// TODO sub_113D8();
	// TODO sub_17749();
	assembleLevelTiles();
	updateInitialBg();
	sub_13BA5();
	sub_13A0E();
	sub_115D2();
	fadePalIn();
	// TODO sub_12345();
}

void test_read(uint16_t chunk_id) {
	struct Local {
		static void errReadData() {
			errorQuit("\nUnable to read data file.\n", errno);
		}
	};

	if (std::fseek(data_file, chunk_id * 4, SEEK_SET) != 0)
		Local::errReadData();

	if (std::fread(&chunk_offsets, sizeof(int32_t), 2, data_file) != 2)
		Local::errReadData();
	if (std::fseek(data_file, chunk_offsets[0], SEEK_SET) != 0)
		Local::errReadData();
	if (std::fread(&decoded_data_len, sizeof(uint16_t), 1, data_file) != 1)
		Local::errReadData();

	uint8_t planes[4][64*1024];
	std::memset(planes, 0, sizeof(planes));

	for (int i = 0; i < 4; ++i) {
		if (std::fread(planes[i], decoded_data_len, 1, data_file) != 1)
			Local::errReadData();
	}

	char tmpfn[30];
	sprintf(tmpfn, "image%03x.bin", chunk_id);
	std::FILE* df = std::fopen(tmpfn, "wb");
	for (int i = 0; i < decoded_data_len; ++i) {
		for (int j = 0; j < 4; ++j) {
			std::fputc(planes[j][i], df);
		}
	}
	std::fclose(df);
}

// addr seg00:0000
int main() {
	vga_initialize();

	hookInts();
	hwCheck();
	allocMemAndLoadData();
	initSound();
	initVideo();
	// TODO setSomeGlobals(); sub_12CA3
	sub_108B8();
	loadNextLevel(); // Logo fade in
	// TODO

	vga_present();
	SDL_Delay(2500);
	vga_deinitialize();
}
