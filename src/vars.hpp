// Include this file only once
// It's a temporary hack only

struct FarPtr {
	uint16_t offset;
	uint8_t* seg;
};

uint16_t data_header1_snd1; // addr seg04:0302
uint16_t data_header1_snd2; // addr seg04:0304

uint16_t current_password[4]; // addr seg04:0310

std::FILE* data_file = 0; // addr seg04:2BB2

int32_t chunk_offsets[2]; // addr seg04:2BB4

uint16_t decoded_data_len; // addr seg04:2BBC

uint8_t* alloc_seg0; // addr seg04:2E5D
uint8_t* alloc_seg1; // addr seg04:2E5F
uint8_t* alloc_seg2; // addr seg04:2E61
uint8_t* alloc_seg3; // addr seg04:2E63
uint8_t* alloc_seg3_end; // addr seg04:2E65
uint8_t* alloc_seg5; // addr seg04:2E67
uint8_t* alloc_seg6; // addr seg04:2E69
uint8_t* soundData2End; // addr seg04:2E6B
uint8_t* soundData1End; // addr seg04:2E6D
uint8_t* soundData0End; // addr seg04:2E6F
uint16_t loaded_chunk0; // addr seg04:2E71
uint8_t* alloc_seg11; // addr seg04:2E73
FarPtr ptr3; // addr seg04:2E75
uint8_t* ptr1; // addr seg04:2E79

uint8_t chunk_buffer0[0x300]; // addr seg04:2E7D
uint8_t chunk_buffer1[0x300]; // addr seg04:317D
uint8_t chunk_buffer2[0x300]; // addr seg04:347D
uint8_t chunk_buffer3[0x300]; // addr seg04:377D
uint8_t chunk_buffer4[0x300]; // addr seg04:3A7D
uint8_t chunk_buffer5[0x300]; // addr seg04:3D7D
uint8_t chunk_buffer6[0x300]; // addr seg04:407D
uint8_t chunk_buffer7[0x300]; // addr seg04:437D
uint8_t chunk_buffer8[0x300]; // addr seg04:467D
uint8_t chunk_buffer9[0x700]; // addr seg04:497D TODO confirm size
uint8_t chunk_buffer10[0x1800]; // addr seg04:507D TODO confirm size
uint8_t chunk_buffer11[0x1800]; // addr seg04:687D TODO TODO TODO really confirm size (completely guessed)

uint32_t start_time; // addr seg04:8639

struct DataHeader1 {
	uint16_t dummy1;

	uint16_t snd1;
	uint16_t snd2;
	uint16_t field_6;
	uint16_t field_8;

	uint16_t dummy2[4];

	uint16_t copy_checksum;
	uint16_t magic; // == 0x6969

	uint16_t dummy3[5];
};
DataHeader1 data_header1; // addr seg04:86B0

FarPtr soundData; // addr seg04:992A

enum LevelFlags : uint8_t {
	LVLFLAG_BIT1 = 0x1,
	LVLFLAG_BIT2 = 0x2,
	LVLFLAG_BIT8 = 0x8,
	LVLFLAG_BIT20 = 0x20,
	LVLFLAG_BIT40 = 0x40,
	LVLFLAG_BIT80 = 0x80
};

#include "pack_enable.hpp"
struct LevelHeader {
	/* 0000 */ uint8_t dummy1[22];
	/* 0016 */ uint16_t next_level_id;
	/* 0018 */ uint8_t dummy2[4];
	/* 001C */ LevelFlags level_flags;
	/* 001D */ uint8_t dummy3[12];
	/* 0029 */ uint16_t anonymous_10;
	/* 002B */ uint16_t anonymous_11;
	/* 002D */ uint8_t dummy4[1];
	/* 002E */ uint16_t anonymous_12;
	/* 0030 */ uint16_t anonymous_13;
	/* 0032 */ uint16_t anonymous_14;
	/* 0034 */ uint8_t dummy5[15];
	/* 0043 */ uint8_t data_load_list[0x100]; // addr seg04:25F6 TODO TODO TODO verify size
}
#include "pack_disable.hpp"
;

LevelHeader level_header; // addr seg04:25B3

uint16_t did_init_timer; // addr seg04:A39A

Color palette1[0x100]; // addr seg04:7F02
Color palette2[0x100]; // addr seg04:8202

Color color1; // addr seg04:0342
Color color2; // addr seg04:0345

uint16_t video_levelY; // addr seg04:0046
uint16_t video_screenShakeY; // addr seg04:03A0
uint16_t video_levelHeight; // addr seg04:25A6
uint16_t video_backBufBase = 0; // addr seg04:92F7
uint16_t video_frontBufBase = 52; // addr seg04:92F9
uint16_t video_resvBufBase = 104; // addr seg04:92FB

uint16_t video_frontBuffer; // addr seg04:9305
uint16_t video_resvBuffer; // addr seg04:9307
uint16_t video_backBuffer; // addr seg04:9309
uint16_t video_backBufAddr; // addr seg04:930B
uint16_t video_frontBufAddr; // addr seg04:930D
uint16_t video_resvBufAddr; // addr seg04:930F

// addr seg04:89F8
const uint16_t heightTileMults[24] = {
	// in increments of 2B0
	// 0x1600 + i * 0x2B0
	0x1600, 0x18B0, 0x1B60, 0x1E10, // 0
	0x20C0, 0x2370, 0x2620, 0x28D0, // 4
	0x2B80, 0x2E30, 0x30E0, 0x3390, // 8
	0x3640, 0x38F0, 0x3BA0, 0x3E50, // 12
	0x4100, 0x43B0, 0x4660, 0x4910, // 16
	0x4BC0, 0x4E70, 0x5120, 0x53D0, // 20
};

// addr seg04:8E58
const uint16_t heightPixelMults[8] = {
	86*0, 86*1, 86*2, 86*3,
	86*4, 86*5, 86*6, 86*7
};

uint16_t video_levelX; // addr seg04:0044
uint16_t video_screenShakeX; // addr seg04:039E
uint16_t video_levelWidth; // addr seg04:25A4
uint8_t video_pixelPan; // addr seg04:92EE

uint16_t video_scroll_x_tiles; // addr seg04:257F
uint16_t video_scroll_y_tiles; // addr seg04:2581

Color stru_2AA88; // addr seg04:25A8

// addr seg04:940C
const uint16_t loadListChunksA[0x30] = {
	0xC6,  0xC8,  0xCA,  0xCC,  0x28,  0x2A,  0x2C,  0x2E,
	0x30,  0x32,  0x34,  0x4B,  0x4D,  0x4F,  0x51,  0x53,
	0x55,  0x72,  0x74,  0x76,  0x78,  0x7A,  0x7C,  0x7E,
	0x80,  0x9C,  0x9E,  0xA0,  0xA2,  0xA4,  0xA6,  0xA8,
	0xAA,  0xCE,  0xD0,  0xD2,  0xD4,  0x171, 0x17D, 0x186,
	0x18C, 0x192, 0x17E, 0x1AF, 0x1B0, 0x1B1, 0x1B2, 0xDA
};

// addr seg04:946C
const uint16_t loadListChunksB[0x30] = {
	0x1C1, 0x1C1, 0x1C1, 0x1C1, 0x1C2, 0x1C2,  0x1C2, 0x1C2,
	0x1C2, 0x1C2, 0x1C2, 0x1C3, 0x1C3, 0x1C3,  0x1C3, 0x1C3,
	0x1C3, 0x1C4, 0x1C4, 0x1C4, 0x1C4, 0x1C4,  0x1C4, 0x1C4,
	0x1C4, 0x1C5, 0x1C5, 0x1C5, 0x1C5, 0x1C5,  0x1C5, 0x1C5,
	0x1C5, 0x1C1, 0x1C1, 0x1C1, 0x1C1, 0xFFFF, 0x1C6, 0x1C6,
	0x1C6, 0x1C6, 0x1C6, 0x1C6, 0x1C6, 0x1C6,  0x1C6, 0x1C6
};

uint8_t byte_2AA63; // addr seg04:2583
uint8_t byte_2AA64[8]; // addr seg04:2584
uint8_t byte_2AA6C[8]; // addr seg04:258C
uint8_t byte_2AA74[8]; // addr seg04:2594
uint8_t byte_2AA7C[8]; // addr seg04:259C

uint16_t loaded_chunks11[0x20]; // addr seg04:12AD
uint16_t loaded_chunks_end11[0x20]; // addr seg04:12ED

uint16_t loaded_chunks2[16]; // addr seg04:124D
uint8_t* loaded_chunks2_ptr[16]; // addr seg04:126D

uint16_t word_31448[0x100]; // addr seg04:8F68

uint16_t word_31648; // addr seg04:9168
uint16_t word_3164A; // addr seg04:916A

uint16_t previous_level; // addr seg04:25AB
uint16_t current_level; // addr seg04:25AD

enum PaletteAction {
	PALACT_NONE = 0,
	PALACT_UNK1 = 2,
	PALACT_COPY = 4
};

PaletteAction palette_action = PALACT_NONE; // addr seg04:7EFE;

Color* pal_pointer = palette1; // addr seg04:7F00
