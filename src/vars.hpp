// Include this file only once
// It's a temporary hack only

#define BIT(i) (1 << (i))

struct FarPtr {
	uint16_t offset;
	uint8_t* seg;
};

int16_t word_28514; // addr seg04:0034
int16_t word_28516; // addr seg04:0036
uint16_t word_28518; // addr seg04:0038
int16_t word_2851A; // addr seg04:003A

uint16_t word_28522; // addr seg04:0042
uint16_t video_levelX; // addr seg04:0044
uint16_t video_levelY; // addr seg04:0046

uint16_t word_2854C; // addr seg04:006C
uint16_t word_2854E; // addr seg04:006E

uint16_t word_286E2; // addr seg04:0202

uint16_t data_header1_snd1; // addr seg04:0302
uint16_t data_header1_snd2; // addr seg04:0304

uint16_t current_password[4]; // addr seg04:0310

uint16_t word_2880F; // addr seg04:032F

uint16_t word_28814; // addr seg04:0334

uint16_t word_2881C; // addr seg04:033C

Color color1; // addr seg04:0342
Color color2; // addr seg04:0345

uint8_t byte_28836[16]; // addr seg04:0356

uint16_t word_28852; // addr seg04:0372
uint16_t word_28854; // addr seg04:0374

uint16_t video_screenShakeX; // addr seg04:039E
uint16_t video_screenShakeY; // addr seg04:03A0

enum Keys
{
	KEYS_UNK0        = BIT(0),
	KEYS_UNK1        = BIT(1),
	KEYS_UNK2        = BIT(2),
	KEYS_UNK3        = BIT(3),
	KEYS_NEXTCHAR    = BIT(4),
	KEYS_PREVCHAR    = BIT(5),
	KEYS_USEITEM     = BIT(6),
	KEYS_ACTIVATE    = BIT(7),
	KEYS_RIGHT       = BIT(8),
	KEYS_LEFT        = BIT(9),
	KEYS_DOWN        = BIT(10),
	KEYS_UP          = BIT(11),
	KEYS_PAUSE       = BIT(12),
	KEYS_INVENTORY   = BIT(13),
	KEYS_SPECIAL     = BIT(14),
	KEYS_ATTACK      = BIT(15)
};

uint16_t keys_down; // addr seg04:03B6
uint16_t keys_pressed; // addr seg04:03B8
uint16_t keys_previous; // addr seg04:03BA

uint16_t word_288A2; // addr seg04:03C2
uint16_t word_288A4; // addr seg04:03C4

int16_t word_288C0; // addr seg04:03E0
uint16_t word_288C2; // addr seg04:03E2
uint16_t inventory_items[12]; // addr seg04:03E4
uint16_t inventory_cur_icons[12]; // addr seg04:03FC

uint16_t word_288F4[3]; // addr seg04:0414

uint16_t word_28903[3]; // addr seg04:0423
uint16_t word_28909[3]; // addr seg04:0429

uint16_t word_2892D[128]; // addr seg04:044D

uint16_t loaded_chunks2[16]; // addr seg04:124D
uint8_t* loaded_chunks2_ptr[16]; // addr seg04:126D
uint16_t loaded_chunks11[32]; // addr seg04:12AD
uint16_t loaded_chunks_end11[32]; // addr seg04:12ED
uint16_t word_2980D[20]; // addr seg04:132D
uint8_t* word_29835[20]; // addr seg04:1355
uint16_t word_29885[20]; // addr seg04:13A5
uint16_t word_298AD[20]; // addr seg04:13CD
uint16_t word_298D5[20]; // addr seg04:13F5
uint16_t word_298FD[20]; // addr seg04:141D
int16_t word_29925[20]; // addr seg04:1445
uint16_t word_2994D[20]; // addr seg04:146D
uint16_t word_29975[20]; // addr seg04:1495
int16_t word_2999D[20]; // addr seg04:14BD
uint16_t word_299C5[20]; // addr seg04:14E5
uint16_t word_299ED[20]; // addr seg04:150D
uint16_t word_29A15[20]; // addr seg04:1535
uint16_t word_29A3D[20]; // addr seg04:155D
uint16_t word_29A65[20]; // addr seg04:1585
uint16_t word_29A8D[20]; // addr seg04:15AD
uint16_t word_29AB5[20]; // addr seg04:15D5
uint16_t word_29ADD[20]; // addr seg04:15FD
uint16_t word_29B05[20]; // addr seg04:1625
uint16_t word_29B2D[20]; // addr seg04:164D
uint16_t word_29B55[20]; // addr seg04:1675
int16_t word_29BA5[20]; // addr seg04:16C5
int16_t word_29BCD[20]; // addr seg04:16ED
uint16_t word_29B7D[20]; // addr seg04:169D
uint16_t word_29BF5[20]; // addr seg04:1715
uint16_t word_29C1D[20]; // addr seg04:173D
uint16_t word_29C45[20]; // addr seg04:1765
uint16_t word_29C6D[20]; // addr seg04:178D
uint16_t word_29C95[20]; // addr seg04:17B5
uint16_t word_29CBD[20]; // addr seg04:17DD
uint16_t word_29CE5[20]; // addr seg04:1805
uint16_t word_29D0D[20]; // addr seg04:182D
uint16_t word_29D35[20]; // addr seg04:1855
uint16_t word_29D5D[20]; // addr seg04:187D
uint16_t word_29D85[20]; // addr seg04:18A5
uint16_t word_29DAD[20]; // addr seg04:18CD
uint16_t word_29DD5[20]; // addr seg04:18F5
uint16_t word_29DFD[20]; // addr seg04:191D
uint16_t word_29E25[20]; // addr seg04:1945
uint16_t word_29E4D[20]; // addr seg04:196D
uint16_t word_29E9D[20]; // addr seg04:19BD
uint16_t word_29EC5[20]; // addr seg04:19E5
uint16_t word_29EED[20]; // addr seg04:1A0D
//---
int16_t word_29F65[20]; // addr seg04:1A85
int16_t word_29F8D[20]; // addr seg04:1AAD
uint16_t word_29FB5[20]; // addr seg04:1AD5
//---
uint16_t word_2AA5B; // addr seg04:257B
uint16_t word_2AA5D; // addr seg04:257D
uint16_t video_scroll_x_tiles; // addr seg04:257F
uint16_t video_scroll_y_tiles; // addr seg04:2581

uint8_t byte_2AA63; // addr seg04:2583
uint8_t byte_2AA64[8]; // addr seg04:2584
uint8_t byte_2AA6C[8]; // addr seg04:258C
uint8_t byte_2AA74[8]; // addr seg04:2594
uint8_t byte_2AA7C[8]; // addr seg04:259C

uint16_t video_level_max_x; // addr seg04:25A4
uint16_t video_level_max_y; // addr seg04:25A6

Color stru_2AA88; // addr seg04:25A8

uint16_t previous_level; // addr seg04:25AB
uint16_t current_level; // addr seg04:25AD

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
	/* 0000 */ uint8_t dummy1[7];
	/* 0007 */ uint8_t anonymous_3;
	/* 0008 */ uint16_t anonymous_4;
	/* 000A */ uint16_t anonymous_5;
	/* 000C */ uint16_t anonymous_6;
	/* 000E */ uint16_t anonymous_7;
	/* 0010 */ uint16_t anonymous_8;
	/* 0012 */ uint8_t dummy0012[4];
	/* 0016 */ uint16_t next_level_id;
	/* 0018 */ uint8_t dummy3[4];
	/* 001C */ LevelFlags level_flags;
	/* 001D */ uint8_t dummy4[12];
	/* 0029 */ uint16_t level_width;
	/* 002B */ uint16_t level_height;
	/* 002D */ uint8_t dummy5[1];
	/* 002E */ uint16_t tilemap_data_chunk;
	/* 0030 */ uint16_t tileset_data_chunk;
	/* 0032 */ uint16_t metatile_data_chunk;
	/* 0034 */ uint8_t dummy6[15];
	// NOTE: I've 'optimized' the size of this array until it loaded
	// all levels in the game. I'm not sure what's the original size.
	/* 0043 */ uint8_t data_load_list[0x561]; // addr seg04:25F6
}
#include "pack_disable.hpp"
;
LevelHeader level_header; // addr seg04:25B3

std::FILE* data_file = 0; // addr seg04:2BB2
int32_t chunk_offsets[2]; // addr seg04:2BB4
uint16_t decoded_data_len; // addr seg04:2BBC

static const size_t metatile_data_size = 0x3600;
uint8_t* metatile_data; // addr seg04:2E5D
static const size_t tileset_data_size = 0x8B80;
uint8_t* tileset_data; // addr seg04:2E5F
static const size_t alloc_seg2_size = 0x2000;
uint8_t* alloc_seg2; // addr seg04:2E61
static const size_t tilemap_data_size = 0x3130;
uint8_t* tilemap_data; // addr seg04:2E63
uint8_t* tilemap_data_end; // addr seg04:2E65

#include "pack_enable.hpp"
struct WorldData
{
	/* 0000 */ uint16_t field_0;
	/* 0002 */ uint8_t field_2;
	/* 0003 */ uint16_t field_3;
	/* 0005 */ uint16_t field_5;
	/* 0007 */ uint16_t field_7;
	/* 0009 */ uint8_t field_9;
	/* 000A */ uint8_t field_A;
	/* 000B */ uint16_t field_B;
	/* 000D */ uint16_t field_D;
	/* 000F */ uint16_t field_F;
	/* 0011 */ uint16_t field_11;
	/* 0013 */ uint16_t field_13;
}
#include "pack_disable.hpp"
;

static const size_t world_data_size = 0xC000;
uint8_t* world_data; // addr seg04:2E67
static const size_t alloc_seg6_size = 0xC080;
uint8_t* alloc_seg6; // addr seg04:2E69
uint8_t* soundData2End; // addr seg04:2E6B
uint8_t* soundData1End; // addr seg04:2E6D
uint8_t* soundData0End; // addr seg04:2E6F
uint16_t loaded_world_chunk; // addr seg04:2E71
static const size_t alloc_seg11_size = 0x7000;
uint8_t* alloc_seg11; // addr seg04:2E73
static const size_t ptr3_size = 0x2ABA0;
uint8_t* ptr3; // addr seg04:2E75
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
uint8_t chunk_buffer11[0x1800]; // addr seg04:687D TODO really confirm size (completely guessed)

enum PaletteAction {
	PALACT_NONE = 0,
	PALACT_UNK1 = 2,
	PALACT_COPY = 4
};

PaletteAction palette_action = PALACT_NONE; // addr seg04:7EFE;
extern Color palette1[0x100];
Color* pal_pointer = palette1; // addr seg04:7F00
Color palette1[0x100]; // addr seg04:7F02
Color palette2[0x100]; // addr seg04:8202

// addr seg04:8562
const uint16_t statusbarIconPos[13] = {
	0x061F, 0x0623, 0x0B7F, 0x0B83, // Erik
	0x0631, 0x0635, 0x0B91, 0x0B95, // Baleog
	0x0643, 0x0647, 0x0BA3, 0x0BA7, // Olaf
	0x064B // Trashcan
};

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

uint16_t word_30BBA; // addr seg04:86DA
uint16_t word_30BBC; // addr seg04:86DC
uint16_t word_30BBE; // addr seg04:86DE

// addr seg04:89F8
/* use calcHeightTileMults instead!
const uint16_t heightTileMults[24] = {
	// in increments of 2B0
	// 0x1600 + (i % 78) * 0x2B0

	0x1600, 0x18B0, 0x1B60, 0x1E10, // 0
	0x20C0, 0x2370, 0x2620, 0x28D0, // 4
	0x2B80, 0x2E30, 0x30E0, 0x3390, // 8
	0x3640, 0x38F0, 0x3BA0, 0x3E50, // 12
	0x4100, 0x43B0, 0x4660, 0x4910, // 16
	0x4BC0, 0x4E70, 0x5120, 0x53D0, // 20
	// and so on...
};
*/

// addr seg04:8E58
const uint16_t heightPixelMults[8] = {
	86*0, 86*1, 86*2, 86*3,
	86*4, 86*5, 86*6, 86*7
};

uint16_t levelRowOffsets[0x100]; // addr seg04:8F68

uint16_t level_width_tiles; // addr seg04:9168
uint16_t level_height_tiles; // addr seg04:916A

uint8_t byte_3168B; // addr seg04:91AB
uint8_t byte_3168C; // addr seg04:91AC

uint8_t video_pixelPan; // addr seg04:92EE
uint16_t word_317CF; // addr seg04:92EF

uint16_t word_317D1; // addr seg04:92F1
uint16_t word_317D3; // addr seg04:92F3
uint16_t word_317D5; // addr seg04:92F5

uint16_t video_backBufBase = 0; // addr seg04:92F7
uint16_t video_frontBufBase = 52; // addr seg04:92F9
uint16_t video_resvBufBase = 104; // addr seg04:92FB

bool video_initialized; // addr seg04:92FF
uint8_t saved_video_mode; // addr seg04:9300

uint16_t video_frontBuffer; // addr seg04:9305
uint16_t video_resvBuffer; // addr seg04:9307
uint16_t video_backBuffer; // addr seg04:9309

uint16_t video_backBufAddr; // addr seg04:930B
uint16_t video_frontBufAddr; // addr seg04:930D
uint16_t video_resvBufAddr; // addr seg04:930F

// addr seg04:940C
const uint16_t levelTileChunks[0x30] = {
	0xC6,  0xC8,  0xCA,  0xCC,  0x28,  0x2A,  0x2C,  0x2E,
	0x30,  0x32,  0x34,  0x4B,  0x4D,  0x4F,  0x51,  0x53,
	0x55,  0x72,  0x74,  0x76,  0x78,  0x7A,  0x7C,  0x7E,
	0x80,  0x9C,  0x9E,  0xA0,  0xA2,  0xA4,  0xA6,  0xA8,
	0xAA,  0xCE,  0xD0,  0xD2,  0xD4,  0x171, 0x17D, 0x186,
	0x18C, 0x192, 0x17E, 0x1AF, 0x1B0, 0x1B1, 0x1B2, 0xDA
};

// addr seg04:946C
const uint16_t levelWorldChunks[0x30] = {
	0x1C1, 0x1C1, 0x1C1, 0x1C1, 0x1C2, 0x1C2,  0x1C2, 0x1C2,
	0x1C2, 0x1C2, 0x1C2, 0x1C3, 0x1C3, 0x1C3,  0x1C3, 0x1C3,
	0x1C3, 0x1C4, 0x1C4, 0x1C4, 0x1C4, 0x1C4,  0x1C4, 0x1C4,
	0x1C4, 0x1C5, 0x1C5, 0x1C5, 0x1C5, 0x1C5,  0x1C5, 0x1C5,
	0x1C5, 0x1C1, 0x1C1, 0x1C1, 0x1C1, 0xFFFF, 0x1C6, 0x1C6,
	0x1C6, 0x1C6, 0x1C6, 0x1C6, 0x1C6, 0x1C6,  0x1C6, 0x1C6
};

static const size_t soundData_size = 0xE470;
FarPtr soundData; // addr seg04:992A

uint16_t did_init_timer; // addr seg04:A39A
uint16_t timer_wait_count; // addr seg04:A39C
