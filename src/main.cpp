#include "vikings.hpp"
#include "main.hpp"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <SDL_timer.h>

#include "vga_emu.hpp"
#include "input.hpp"
#include "vm.hpp"
#include "vars.hpp"

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

// addr seg00:2948
void hookInts() {
	// init memory and segments();

	//hookKeyboard(); // Not used anymore.

	start_time = curSystemTime();
}

// addr seg00:0DBA
void errorQuit(const char* msg, uint16_t error_code) {
	//unhookKeyboard(); // Not used anymore.
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
// Loads data file and initializes misc. hardware.
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
		uint8_t *const buf_base = new uint8_t[soundData_size];
		uint8_t* buf = buf_base;

		soundData.seg = buf;
		soundData.offset = 0;

		// Note: roundToNextSeg not needed because segments are for 16-bit losers
		// I'm not sure these are quite right, they seem to point to the end of the data
		// but looking at the original that's effectively what happens?
		soundData0End = buf = decompressChunk(0x1C7 + data_header1.field_6, buf, soundData_size - (buf - buf_base));
		soundData1End = buf = decompressChunk(0x20C + data_header1.field_8, buf, soundData_size - (buf - buf_base));
		soundData2End =       decompressChunk(0x207 + data_header1.field_8, buf, soundData_size - (buf - buf_base));
	}

	ptr3 = new uint8_t[ptr3_size];

	world_data = new uint8_t[world_data_size];

	loaded_world_chunk = 0x1C6;
	decompressChunk(loaded_world_chunk, world_data, world_data_size);

	decompressChunk(4,  chunk_buffer0,  sizeof(chunk_buffer0));
	decompressChunk(5,  chunk_buffer1,  sizeof(chunk_buffer1));
	decompressChunk(6,  chunk_buffer2,  sizeof(chunk_buffer2));
	decompressChunk(7,  chunk_buffer3,  sizeof(chunk_buffer3));
	decompressChunk(8,  chunk_buffer4,  sizeof(chunk_buffer4));
	decompressChunk(9,  chunk_buffer5,  sizeof(chunk_buffer5));
	decompressChunk(10, chunk_buffer6,  sizeof(chunk_buffer6));
	decompressChunk(11, chunk_buffer7,  sizeof(chunk_buffer7));
	decompressChunk(12, chunk_buffer8,  sizeof(chunk_buffer8));
	decompressChunk(13, chunk_buffer9,  sizeof(chunk_buffer9));
	decompressChunk(14, chunk_buffer10, sizeof(chunk_buffer10));
	decompressChunk(2,  chunk_buffer11, sizeof(chunk_buffer11));

	// Set level 1 password as default
	current_password[0] = 'S';
	current_password[1] = 'T';
	current_password[2] = 'R';
	current_password[3] = 'T';

	// Setup initial button assignments
	button_key_assignments[SCAN_KP8_UP]     = BUTTON_UP;
	button_key_assignments[SCAN_KP4_LEFT]   = BUTTON_LEFT;
	button_key_assignments[SCAN_KP6_RIGHT]  = BUTTON_RIGHT;
	button_key_assignments[SCAN_KP5]        = BUTTON_DOWN;
	button_key_assignments[SCAN_KP2_DOWN]   = BUTTON_DOWN;
	button_key_assignments[SCAN_CTRL]       = BUTTON_PREVCHAR;
	button_key_assignments[SCAN_KP7_HOME]   = BUTTON_PREVCHAR;
	button_key_assignments[SCAN_KP0_INSERT] = BUTTON_NEXTCHAR;
	button_key_assignments[SCAN_KP9_PAGEUP] = BUTTON_NEXTCHAR;
	button_key_assignments[SCAN_ESCAPE]     = BUTTON_PAUSE;
	button_key_assignments[SCAN_P]          = BUTTON_PAUSE;
	button_key_assignments[SCAN_CAPSLOCK]   = BUTTON_INVENTORY;
	button_key_assignments[SCAN_TAB]        = BUTTON_INVENTORY;
	button_key_assignments[SCAN_S]          = BUTTON_ACTIVATE;
	button_key_assignments[SCAN_F]          = BUTTON_ATTACK;
	button_key_assignments[SCAN_SPACEBAR]   = BUTTON_ATTACK;
	button_key_assignments[SCAN_RETURN]     = BUTTON_ATTACK;
	button_key_assignments[SCAN_KPPLUS]     = BUTTON_ATTACK;
	button_key_assignments[SCAN_E]          = BUTTON_USEITEM;
	button_key_assignments[SCAN_D]          = BUTTON_SPECIAL;
}

// addr seg00:0E35
void freeMemAndQuit() {
	//unhookKeyboard(); // Not used anymore.
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
	vga_setLineCompare(0x15F);
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

	timer_wait_count = 1;

	word_2AA5B = video_levelX;
	word_2AA5D = video_levelY;
	word_317CF = video_scroll_x_tiles;
	word_317D1 = video_scroll_y_tiles;
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
void zeroInventory() {
	// TODO
	std::fill(RANGE(word_288F4), 0);
	std::fill(RANGE(inventory_items), 0);
	std::fill(RANGE(word_28903), 6);
	std::fill(RANGE(word_28909), 0);
}

// addr seg00:08B8
void initGameState() {
	zeroInventory();
	level_header.next_level_id = 0x27;
	word_288A2 = 0;
}

// addr seg00:11B1
void loadLevelHeader(uint16_t level_id) {
	uint16_t chunk_id = levelWorldChunks[level_id];
	if (chunk_id != 0xFFFF && chunk_id != loaded_world_chunk) {
		loaded_world_chunk = chunk_id;
		decompressChunk(chunk_id, world_data, world_data_size);
	}
	decompressChunk(levelTileChunks[level_id], reinterpret_cast<uint8_t*>(&level_header), sizeof(level_header));
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
		decompressChunk(ax, reinterpret_cast<uint8_t*>(&palette1[offset]), sizeof(palette1) - sizeof(*palette1)*offset);
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
		bx = decompressChunk(ax, bx, alloc_seg11_size - (bx - alloc_seg11));

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

		loaded_chunks2_ptr[si] = ptr1;
		ptr1 = decompressChunk(chunk_id, ptr1, ptr3_size - (ptr1 - ptr3));
		di += 5;
		si++;
	}

	return di;
}

// addr seg00:1784
void sub_11784() {
	word_288A2 = 0;
	word_288A4 = 0;
}

// addr seg00:1397
void sub_11397() {
	// TODO word_28886 = byte_30A1E[word_2AAAB];
	// TODO word_28888 = byte_30A24[word_2AAAB];
}

// addr seg00:37F1
void sub_137F1() {
	std::fill(RANGE(word_29835), nullptr);
	std::fill(RANGE(word_29EED), 0xFFFF);
}

// addr seg00:2FB3
void sub_12FB3() {
	std::fill(RANGE(word_2892D), 0);
}

// addr seg00:3BBD
bool sub_13BBD(uint16_t si, uint16_t di) {
	// TODO
	return false;
}

// addr seg00:3BA5
void sub_13BA5() {
	word_28522 = 0xFFFF;
	uint16_t si = 0;
	uint16_t di = 0;

	// TODO sub_13BBD
	while (sub_13BBD(si, di)) {
		si++;
		di += 14;
	}
}

// addr seg00:39EF
void sub_139EF()
{
	word_28514 = video_levelX - 16;
	word_28516 = video_levelX - 16 + 352;

	word_28518 = video_levelY - 16;
	word_2851A = video_levelY - 16 + 208;
}

// addr seg00:3A94
void sub_13A94()
{
	if (word_28514 < 0)
		word_28514 = 0;

	if (word_28516 < 0)
		word_28516 = 0;

	if (word_28518 < 0)
		word_28518 = 0;

	if (word_2851A < 0)
		word_2851A = 0;

	word_28522 = 0xFFFF;

	uint16_t si = 0;
	uint16_t di = 0;
	/* TODO
	while (sub_13AE0(...))
	{
		si++;
		di += 14;
	}
	*/
}

// addr seg00:3A0E
void sub_13A0E() {
	sub_139EF();
	sub_13A94();
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

		decompressChunk(level_header.tileset_data_chunk, tileset_data, tileset_data_size);
		decompressChunk(level_header.tileset_data_chunk + 1, alloc_seg2, alloc_seg2_size);
		tilemap_data_end = decompressChunk(level_header.tilemap_data_chunk, tilemap_data, tilemap_data_size);
		decompressChunk(level_header.metatile_data_chunk, metatile_data, metatile_data_size);
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

// addr seg00:6880
void video_clearVRAM() {
	vga_fillVRAM(0xF, 0, 0, 0xFFFF);
}

// addr seg00:183D
void drawInventoryItem(uint16_t item_id, uint16_t slot) {
	if (slot == 12 && item_id == 0) {
		item_id = 23; // Trashcan
	}

	uint8_t* icon_data = &chunk_buffer10[item_id * 16*16];

	for (int pi = 0; pi < 4; ++pi) {
		static const int pmap[4] = {3, 0, 1, 2};
		static const int poff[4] = {0, 1, 1, 1};
		int p = pmap[pi];

		uint16_t vid_pos = statusbarIconPos[slot] + poff[pi];

		for (int y = 0; y < 16; ++y) {
			for (int x = 0; x < 4; ++x) {
				vga_framebuffer[p][vid_pos++] = *icon_data++;
			}
			vid_pos += (344 - 16) / 4;
		}
	}

	vga_present();
}

// addr seg00:201D
void drawInventory() {
	for (int i = 0; i < 12; ++i) {
		inventory_cur_icons[i] = inventory_items[i];
		drawInventoryItem(inventory_items[i], i);
	}
}

// addr seg00:18AD
void drawInventoryReticle(int slot) {
	uint8_t* icon_data = &chunk_buffer10[0x1300];
	uint16_t vid_pos = statusbarIconPos[slot];

	for (int pi = 0; pi < 4; ++pi) {
		static const int pmap[4] = {3, 0, 1, 2};
		int p = pmap[pi];

		if (pi == 0) {
			vga_framebuffer[p][vid_pos]       = icon_data[0];
			vga_framebuffer[p][vid_pos+1]     = icon_data[1];
			vga_framebuffer[p][vid_pos+3]     = icon_data[3];
			vga_framebuffer[p][vid_pos+0x56]  = icon_data[4];
			vga_framebuffer[p][vid_pos+0x57]  = icon_data[5];
			vga_framebuffer[p][vid_pos+0x59]  = icon_data[7];
			vga_framebuffer[p][vid_pos+0xAC]  = icon_data[8];
			vga_framebuffer[p][vid_pos+0x102] = icon_data[0xC];
			vga_framebuffer[p][vid_pos+0x158] = icon_data[0x10];
			vga_framebuffer[p][vid_pos+0x3B2] = icon_data[0x2C];
			vga_framebuffer[p][vid_pos+0x408] = icon_data[0x30];
			vga_framebuffer[p][vid_pos+0x45E] = icon_data[0x34];
			vga_framebuffer[p][vid_pos+0x4B4] = icon_data[0x38];
			vga_framebuffer[p][vid_pos+0x4B5] = icon_data[0x39];
			vga_framebuffer[p][vid_pos+0x4B7] = icon_data[0x3B];
			vga_framebuffer[p][vid_pos+0x50A] = icon_data[0x3C];
			vga_framebuffer[p][vid_pos+0x50B] = icon_data[0x3D];
			vga_framebuffer[p][vid_pos+0x50D] = icon_data[0x3F];
		} else if (pi == 1) {
			vga_framebuffer[p][vid_pos+1]     = icon_data[0x40];
			vga_framebuffer[p][vid_pos+4]     = icon_data[0x43];
			vga_framebuffer[p][vid_pos+0x57]  = icon_data[0x44];
			vga_framebuffer[p][vid_pos+0x5A]  = icon_data[0x47];
			vga_framebuffer[p][vid_pos+0xAD]  = icon_data[0x48];
			vga_framebuffer[p][vid_pos+0x103] = icon_data[0x4C];
			vga_framebuffer[p][vid_pos+0x159] = icon_data[0x50];
			vga_framebuffer[p][vid_pos+0x3B3] = icon_data[0x6C];
			vga_framebuffer[p][vid_pos+0x409] = icon_data[0x70];
			vga_framebuffer[p][vid_pos+0x45F] = icon_data[0x74];
			vga_framebuffer[p][vid_pos+0x4B5] = icon_data[0x78];
			vga_framebuffer[p][vid_pos+0x4B8] = icon_data[0x7B];
			vga_framebuffer[p][vid_pos+0x50B] = icon_data[0x7C];
			vga_framebuffer[p][vid_pos+0x50E] = icon_data[0x7F];
		} else if (pi == 2) {
			vga_framebuffer[p][vid_pos+1]     = icon_data[0x80];
			vga_framebuffer[p][vid_pos+4]     = icon_data[0x83];
			vga_framebuffer[p][vid_pos+0x57]  = icon_data[0x84];
			vga_framebuffer[p][vid_pos+0x5A]  = icon_data[0x87];
			vga_framebuffer[p][vid_pos+0xB0]  = icon_data[0x8B];
			vga_framebuffer[p][vid_pos+0x106] = icon_data[0x8F];
			vga_framebuffer[p][vid_pos+0x15C] = icon_data[0x93];
			vga_framebuffer[p][vid_pos+0x3B6] = icon_data[0xAF];
			vga_framebuffer[p][vid_pos+0x40C] = icon_data[0xB3];
			vga_framebuffer[p][vid_pos+0x462] = icon_data[0xB7];
			vga_framebuffer[p][vid_pos+0x4B5] = icon_data[0xB8];
			vga_framebuffer[p][vid_pos+0x4B8] = icon_data[0xBB];
			vga_framebuffer[p][vid_pos+0x50B] = icon_data[0xBC];
			vga_framebuffer[p][vid_pos+0x50E] = icon_data[0xBF];
		} else if (pi == 3) {
			vga_framebuffer[p][vid_pos+1]     = icon_data[0xC0];
			vga_framebuffer[p][vid_pos+3]     = icon_data[0xC2];
			vga_framebuffer[p][vid_pos+4]     = icon_data[0xC3];
			vga_framebuffer[p][vid_pos+0x57]  = icon_data[0xC4];
			vga_framebuffer[p][vid_pos+0x59]  = icon_data[0xC6];
			vga_framebuffer[p][vid_pos+0x5A]  = icon_data[0xC7];
			vga_framebuffer[p][vid_pos+0xB0]  = icon_data[0xCB];
			vga_framebuffer[p][vid_pos+0x106] = icon_data[0xCF];
			vga_framebuffer[p][vid_pos+0x15C] = icon_data[0xD3];
			vga_framebuffer[p][vid_pos+0x3B6] = icon_data[0xEF];
			vga_framebuffer[p][vid_pos+0x40C] = icon_data[0xF3];
			vga_framebuffer[p][vid_pos+0x462] = icon_data[0xF7];
			vga_framebuffer[p][vid_pos+0x4B5] = icon_data[0xF8];
			vga_framebuffer[p][vid_pos+0x4B7] = icon_data[0xFA];
			vga_framebuffer[p][vid_pos+0x4B8] = icon_data[0xFB];
			vga_framebuffer[p][vid_pos+0x50B] = icon_data[0xFC];
			vga_framebuffer[p][vid_pos+0x50D] = icon_data[0xFE];
			vga_framebuffer[p][vid_pos+0x50E] = icon_data[0xFF];
		}
	}

	vga_present();
}

#if 0
void drawInventoryReticle(int slot) {
	uint8_t* icon_data = &chunk_buffer10[0x1300];
	uint16_t vid_pos = statusbarIconPos[slot] * 4;
	const int line = 344;
	const int line2 = 64;

	vga_setPixel(vid_pos + line* 0+ 3, icon_data[line2*0 + 0x0]);
	vga_setPixel(vid_pos + line* 0+ 4, icon_data[line2*1 + 0x0]);
	vga_setPixel(vid_pos + line* 0+ 5, icon_data[line2*2 + 0x0]);
	vga_setPixel(vid_pos + line* 0+ 6, icon_data[line2*3 + 0x0]);
	vga_setPixel(vid_pos + line* 0+ 7, icon_data[line2*0 + 0x1]);
	vga_setPixel(vid_pos + line* 0+14, icon_data[line2*3 + 0x2]);
	vga_setPixel(vid_pos + line* 0+15, icon_data[line2*0 + 0x3]);
	vga_setPixel(vid_pos + line* 0+16, icon_data[line2*1 + 0x3]);
	vga_setPixel(vid_pos + line* 0+17, icon_data[line2*2 + 0x3]);
	vga_setPixel(vid_pos + line* 0+18, icon_data[line2*3 + 0x3]);

	vga_setPixel(vid_pos + line* 1+ 3, icon_data[line2*0 + 0x4]);
	vga_setPixel(vid_pos + line* 1+ 4, icon_data[line2*1 + 0x4]);
	vga_setPixel(vid_pos + line* 1+ 5, icon_data[line2*2 + 0x4]);
	vga_setPixel(vid_pos + line* 1+ 6, icon_data[line2*3 + 0x4]);
	vga_setPixel(vid_pos + line* 1+ 7, icon_data[line2*0 + 0x5]);
	vga_setPixel(vid_pos + line* 1+14, icon_data[line2*3 + 0x6]);
	vga_setPixel(vid_pos + line* 1+15, icon_data[line2*0 + 0x7]);
	vga_setPixel(vid_pos + line* 1+16, icon_data[line2*1 + 0x7]);
	vga_setPixel(vid_pos + line* 1+17, icon_data[line2*2 + 0x7]);
	vga_setPixel(vid_pos + line* 1+18, icon_data[line2*3 + 0x7]);

	vga_setPixel(vid_pos + line* 2+ 3, icon_data[line2*0 + 0x8]);
	vga_setPixel(vid_pos + line* 2+ 4, icon_data[line2*1 + 0x8]);
	vga_setPixel(vid_pos + line* 2+17, icon_data[line2*2 + 0xB]);
	vga_setPixel(vid_pos + line* 2+18, icon_data[line2*3 + 0xB]);

	vga_setPixel(vid_pos + line* 3+ 3, icon_data[line2*0 + 0x0]);
	vga_setPixel(vid_pos + line* 3+ 4, icon_data[line2*1 + 0xC]);
	vga_setPixel(vid_pos + line* 3+17, icon_data[line2*2 + 0xF]);
	vga_setPixel(vid_pos + line* 3+18, icon_data[line2*3 + 0xF]);

	vga_setPixel(vid_pos + line* 4+ 3, icon_data[line2*0 + 0x10]);
	vga_setPixel(vid_pos + line* 4+ 4, icon_data[line2*1 + 0x10]);
	vga_setPixel(vid_pos + line* 4+17, icon_data[line2*2 + 0x13]);
	vga_setPixel(vid_pos + line* 4+18, icon_data[line2*3 + 0x13]);

	vga_setPixel(vid_pos + line*11+ 3, icon_data[line2*0 + 0x2C]);
	vga_setPixel(vid_pos + line*11+ 4, icon_data[line2*1 + 0x2C]);
	vga_setPixel(vid_pos + line*11+17, icon_data[line2*2 + 0x2F]);
	vga_setPixel(vid_pos + line*11+18, icon_data[line2*3 + 0x2F]);

	vga_setPixel(vid_pos + line*12+ 3, icon_data[line2*0 + 0x30]);
	vga_setPixel(vid_pos + line*12+ 4, icon_data[line2*1 + 0x30]);
	vga_setPixel(vid_pos + line*12+17, icon_data[line2*2 + 0x33]);
	vga_setPixel(vid_pos + line*12+18, icon_data[line2*3 + 0x33]);

	vga_setPixel(vid_pos + line*13+ 3, icon_data[line2*0 + 0x34]);
	vga_setPixel(vid_pos + line*13+ 4, icon_data[line2*1 + 0x34]);
	vga_setPixel(vid_pos + line*13+17, icon_data[line2*2 + 0x37]);
	vga_setPixel(vid_pos + line*13+18, icon_data[line2*3 + 0x37]);

	vga_setPixel(vid_pos + line*14+ 3, icon_data[line2*0 + 0x38]);
	vga_setPixel(vid_pos + line*14+ 4, icon_data[line2*1 + 0x38]);
	vga_setPixel(vid_pos + line*14+ 5, icon_data[line2*2 + 0x38]);
	vga_setPixel(vid_pos + line*14+ 6, icon_data[line2*3 + 0x38]);
	vga_setPixel(vid_pos + line*14+ 7, icon_data[line2*0 + 0x39]);
	vga_setPixel(vid_pos + line*14+14, icon_data[line2*3 + 0x3A]);
	vga_setPixel(vid_pos + line*14+15, icon_data[line2*0 + 0x3B]);
	vga_setPixel(vid_pos + line*14+16, icon_data[line2*1 + 0x3B]);
	vga_setPixel(vid_pos + line*14+17, icon_data[line2*2 + 0x3B]);
	vga_setPixel(vid_pos + line*14+18, icon_data[line2*3 + 0x3B]);

	vga_setPixel(vid_pos + line*15+ 3, icon_data[line2*0 + 0x3C]);
	vga_setPixel(vid_pos + line*15+ 4, icon_data[line2*1 + 0x3C]);
	vga_setPixel(vid_pos + line*15+ 5, icon_data[line2*2 + 0x3C]);
	vga_setPixel(vid_pos + line*15+ 6, icon_data[line2*3 + 0x3C]);
	vga_setPixel(vid_pos + line*15+ 7, icon_data[line2*0 + 0x3D]);
	vga_setPixel(vid_pos + line*15+14, icon_data[line2*3 + 0x3E]);
	vga_setPixel(vid_pos + line*15+15, icon_data[line2*0 + 0x3F]);
	vga_setPixel(vid_pos + line*15+16, icon_data[line2*1 + 0x3F]);
	vga_setPixel(vid_pos + line*15+17, icon_data[line2*2 + 0x3F]);
	vga_setPixel(vid_pos + line*15+18, icon_data[line2*3 + 0x3F]);

	vga_present();
}
#endif

// addr seg00:20D1
void drawInventorySelections() {
	for (int i = 0; i < 3; ++i) {
		drawInventoryReticle(word_288F4[i] + 4*i);
	}
}

// addr seg00:17AD
void drawStatusbar() {
	// TODO word_28820 = 2;
	if (level_header.level_flags & LVLFLAG_BIT1) {
		loadImage(1, 0);
		// TODO sub_1200A
		drawInventory();
		// TODO sub_12034
		drawInventorySelections();
	}
}

// addr seg00:13D8
void setInitialScreenPos() {
	uint16_t si = 0;
	if (level_header.anonymous_3 != 0) {
		si = word_288A2;
	}

	int16_t ax = word_29C1D[si/2];
	if (ax < 320 / 2)
		ax = 0;
	else
		ax -= 320 / 2;

	if (ax > video_level_max_x)
		ax = video_level_max_x;

	video_levelX = ax;
	word_2AA5B = ax;

	ax *= 8;
	video_scroll_x_tiles = ax;
	word_317CF = ax;
	ax *= 2;
	word_317D3 = ax;

	ax = word_29C45[si/2];
	if (ax < 176 / 2)
		ax = 0;
	else
		ax -= 176 / 2;

	if (ax > video_level_max_y)
		ax = video_level_max_y;

	video_levelY = ax;
	word_2AA5D = ax;

	ax *= 8;
	video_scroll_y_tiles;
	word_317D1 = ax;
	ax *= 2;
	word_317D5 = ax;
}

// addr seg00:3D30
bool sub_13D30(uint16_t di)
{
	if (word_2880F != 0)
	{
		return true;
	}

	if (di == 0xFFFF)
	{
		return false;
	}

	if (byte_28836[di / 8] & BIT(di % 8))
		return true;
	else
		return false;
}

// addr seg00:3D52
// Finds a free slot in word_29835 and returns it's index. Else returns -1.
int findFreeWord29835()
{
	for (int i = 0; i < 20; ++i)
	{
		if (word_29835[i] == 0)
			return i;
	}

	return -1;
}

// addr seg00:2F82
int sub_12F82(uint16_t ax, uint16_t* out_ax)
{
	if (ax == 0xFFFF)
	{
		*out_ax = 0;
		return true;
	}
	
	if (ax == 0xFFFE)
	{
		word_28854 += 1;
		*out_ax = 0;
		return true;
	}

	for (int di = 0; di < 32; ++di)
	{
		if (ax == loaded_chunks11[di])
		{
			*out_ax = loaded_chunks_end11[di];
			return true;
		}
	}

	return false;
}

// addr seg00:3E52
bool sub_13E52(uint16_t si, uint16_t bx)
{
	word_29BCD[si] = word_28514;
	word_29BA5[si] = word_28516;
	word_29A65[si] = word_28518;
	word_29B7D[si] = word_28854;
	word_28854 = 0;

	WorldData* world_data_entry = reinterpret_cast<WorldData*>(world_data + bx*21);

	if (!sub_12F82(world_data_entry->field_0, &word_29D35[si]))
		return false;

	uint8_t ax = world_data_entry->field_2;
	if ((ax & 0x80) != 0)
		word_28854 += 2;

	word_29FB5[si] = ax & 0x7F;
	obj_script_resume[si] = world_data_entry->field_3 + 3;
	word_29835[si] = world_data;
	word_29A8D[si] = world_data_entry->field_7;
	word_29925[si] = world_data_entry->field_9;
	word_2994D[si] = world_data_entry->field_A;
	word_29AB5[si] = world_data_entry->field_B;
	word_29ADD[si] = world_data_entry->field_D;
	word_29B05[si] = world_data_entry->field_F;
	word_29C6D[si] = world_data_entry->field_11;
	word_29C95[si] = world_data_entry->field_13;

	word_29885[si] = word_29C1D[si] = word_2854C;
	word_298AD[si] = word_29C45[si] = word_2854E;
	word_29CE5[si] = word_28522;

	word_29E9D[si] = 0;
	word_29EC5[si] = 0;
	word_29BF5[si] = 0;
	word_29B2D[si] = 0;
	word_29B55[si] = 0;
	word_29E25[si] = 0;
	word_29E4D[si] = 0;
	word_29CBD[si] = 0;
	word_29D85[si] = 0;
	word_29DAD[si] = 0;
	word_29DFD[si] = 0xFFFF;
	word_29D5D[si] = 0;
	word_29DD5[si] = 0;
	word_29D0D[si] = 0xFFFF;
	word_29EED[si] = 0xFFFF;
	word_298FD[si] = 0xFFFF;

	uint16_t dx = word_29C1D[si] - (word_29925[si] / 2);
	word_29A15[si] = dx;
	word_29A3D[si] = dx + word_29925[si] - 1;

	dx = word_29C45[si] - (word_2994D[si] / 2);
	word_299C5[si] = dx;
	word_299ED[si] = dx + word_2994D[si] - 1;

	int16_t ax2 = word_288C0;
	if (ax2 < 0)
	{
		word_288C2 = word_2994D[si] / 2;
		ax2 = word_29925[si] / 2;
	}

	word_2999D[si] = ax2;

	word_29975[si] = word_288C2;
	return true;
}

// addr seg00:3809
bool sub_13809(int16_t ax, uint16_t di, uint16_t si, uint16_t* out_di)
{
	word_28514 = ax;
	word_28516 = di;
	word_28518 = si;
	if (sub_13D30(di))
	{
		*out_di = 0;
		return false;
	}

	si = findFreeWord29835();
	if (si == -1)
	{
		*out_di = 0;
		return false;
	}

	if (!sub_13E52(si, word_28514))
	{
		word_29835[si] = 0;
		*out_di = 0;
		return false;
	}

	if (word_29FB5[si] != 0)
	{
		// TODO sub_13D68
		if (!true)
		{
			word_29835[si] = 0;
			*out_di = 0;
			return false;
		}

		word_29F65[si] = word_2851A;
		word_29F8D[si] = word_28518;
		// TODO sub_13DD6(si);
		// TODO sub_13E15(si);
	}

	if (si >= word_28852)
	{
		word_28852 = si;
		word_28852 += 1;
	}

	*out_di = si;
	return true;
}

// addr seg00:1446
void sub_11446()
{
	word_2881C = 0;

	if (level_header.anonymous_3 == 0)
	{
		word_28854 = level_header.anonymous_8;
		word_2854C = level_header.anonymous_4;
		word_2854E = level_header.anonymous_5;
		word_28522 = 0xFFFF;

		uint16_t di;
		sub_13809(level_header.anonymous_6, 0xFFFF, level_header.anonymous_7, &di);

		word_2881C = 2;
		return;
	}
	else
	{
		// TODO
	}
}

// addr seg00:1080
void loadNextLevel() {
	// TODO lotsa variables

	pressed_buttons = 0;
	word_30BBC = 0;
	buttons_down = 0;
	buttons_pressed = 0;

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
		zeroInventory();
	drawStatusbar();
	loadChunks3();
	uint16_t di = seekLoadList();
	di = loadPaletteList(di);
	di = processLoadList(di);
	di = loadChunkList(di);
	di = loadChunkList2(di);
	sub_11397(); // TODO
	sub_137F1();
	sub_12FB3();
	sub_11446();
	calcLevelSize();
	setInitialScreenPos();
	// TODO sub_17749();
	assembleLevelTiles();
	updateInitialBg();
	sub_13BA5(); // TODO
	sub_13A0E(); // TODO
	sub_115D2(); // TODO
	fadePalIn();
	// TODO sub_12345();
}

// addr seg00:0138
void sub_10138()
{
	uint16_t ax = word_28814;

	if (ax & BIT(2))
	{
		word_28814 &= ~BIT(2);
		while (updateInput(), (buttons_pressed &
			(BUTTON_USEITEM | BUTTON_ACTIVATE | BUTTON_SPECIAL | BUTTON_ATTACK)) == 0)
		{
			// TODO sub_101BE();
			updateVgaBuffer();
			waitForTimerInt();
			// TODO sub_108C8();
			updateVgaBuffer();
			waitForTimerInt();
			// TODO sub_108C8();
			updateVgaBuffer();
			waitForTimerInt();
		}

		updateInput();
	}
	else if (ax & (BIT(0) | BIT(1)))
	{
		if (ax & BIT(1))
		{
			level_header.next_level_id = LEVEL_RESPAWN;
		}

		word_28814 = 0;
		// TODO sub_1774F();
		word_2880F++;
		// TODO sub_14207();
		loadNextLevel();
	}

}

// addr seg00:5517
void sub_15517()
{
	// Any particular reason this loop is reversed?
	for (int si = word_28852 - 1; si >= 0; --si)
	{
		word_29E25[si] = 0;
		word_29E4D[si] = 0;
	}
}

// addr seg00:4207
void sub_14207()
{
	sub_15517();
	word_28856 = 0;
	word_28870 = 0;

	for (int si = 0; si < word_28852; ++si)
	{
		runObjectScript(si);
		if (word_28856 != 0)
		{
			for (int di = 0; di < word_28856; ++di)
			{
				runObjectScript(word_28858[di]);
			}
			word_28856 = 0;
		}
	}
}

// addr seg00:2CA3
void sub_12CA3()
{
	// TODO byte_2880E = 0;
	// TODO byte_28508 = 0;
	// TODO word_288B4 = 0;
	word_288AC = 0;
	// TODO word_28816 = 0;
	// TODO word_2882A = 0;
	// TODO word_2882C = 0;
	word_28814 = 0;
	// TODO word_28811 = 0;
	// TODO word_2AA8F = 0xFFFF;
	word_288A2 = 0xFFFF;
}

// addr seg00:0000
int main() {
	vga_initialize();

	hookInts();
	hwCheck();
	allocMemAndLoadData();
	initSound();
	initVideo();
	sub_12CA3();
	initGameState();
	loadNextLevel(); // Logo fade in
	timer_wait_count = 1;

	// Game main loop
	while (true)
	{
		handleSDLEvents();
		updateInput();

		// TODO sub_12D72
		// TODO sub_102AD
		// TODO sub_1041C
		sub_10138();
		// TODO sub_11BA5
		// TODO sub_12E79
		// TODO sub_10813
		// TODO sub_1673C
		sub_14207();
		// TODO sub_1386B
		// TODO sub_1625D
		// TODO sub_15546
		// TODO sub_13916
		// TODO sub_1064B
		// TODO sub_12FC6
		waitForTimerInt();
		// TODO sub_1DE05
		// TODO sub_165AA
		// TODO sub_16661
		// TODO sub_1406D
		// TODO sub_1DD9C
		// TODO mov     ax, 0FFFEh
		// TODO sub_1C8F1
		// TODO sub_1E0C7
		updateVgaBuffer();
		// TODO sub_12E16
		// TODO sub_15530
		// TODO sub_10704
		// TODO sub_12FCB
		// TODO loc_12D2C
		waitForTimerInt();
		// TODO sub_1DE05
		// TODO sub_165AA
		// TODO sub_16661
		// TODO sub_1406D
		// TODO sub_1DD9C
		// TODO mov     ax, 0FFFEh
		// TODO sub_1C8F1
		// TODO sub_1E0C7
		updateVgaBuffer();
		// TODO sub_10753
		// TODO sub_13C0C
		// TODO sub_12FD0
		// TODO sub_11792
		// TODO sub_101BE
		waitForTimerInt();
		// TODO sub_1DE05
		// TODO sub_165AA
		// TODO sub_16661
		// TODO sub_1DD9C
		// TODO mov     ax, 0FFFEh
		// TODO sub_1C8F1
		// TODO sub_1E0C7
		updateVgaBuffer();
		// TODO mov     word_30C14, 0
		// TODO sub_108C8
		// TODO sub_10350
		// TODO sub_1086F

		if (current_level < 0x25 && word_286E2 != 0)
		{
			if (byte_3168B != 1)
			{
				if (byte_3168C == 1)
					word_28814 |= 1;
			}
			else
			{
				word_28814 |= 1;

				int next_level = current_level - 1;
				if (next_level < 0)
					next_level = 0;

				level_header.next_level_id = next_level;
			}
		}
	}

	vga_deinitialize();
}
