// Include this file only once
// It's a temporary hack only

uint16_t data_header1_snd1;
uint16_t data_header1_snd2;

uint16_t current_password[4];

std::FILE* data_file = 0;

int32_t chunk_offsets[2];

uint16_t decoded_data_len;

uint8_t* alloc_seg0;
uint8_t* alloc_seg1;
uint8_t* alloc_seg2;
uint8_t* alloc_seg3;
uint8_t* alloc_seg5;
uint8_t* alloc_seg6;
uint8_t* sound_buffer2;
uint8_t* sound_buffer1;
uint8_t* sound_buffer0;
uint16_t loaded_chunk0;
uint8_t* alloc_seg11;
uint16_t ptr3_offset;
uint8_t* ptr3_seg;
uint8_t* ptr1;

uint8_t chunk_buffer0[0x300];
uint8_t chunk_buffer1[0x300];
uint8_t chunk_buffer2[0x300];
uint8_t chunk_buffer3[0x300];
uint8_t chunk_buffer4[0x300];
uint8_t chunk_buffer5[0x300];
uint8_t chunk_buffer6[0x300];
uint8_t chunk_buffer7[0x300];
uint8_t chunk_buffer8[0x300];
uint8_t chunk_buffer9[0x700]; // TODO confirm size
uint8_t chunk_buffer10[0x1800]; // TODO confirm size
uint8_t chunk_buffer11[0x1800]; // TODO TODO TODO really confirm size (completely guessed)

uint32_t start_time;

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
DataHeader1 data_header1;

uint16_t ptr2_offset;
uint8_t* ptr2_seg;

struct DataLoadListStruct {
	uint8_t dummy1[22];
	uint16_t next_load_list_id;
	uint8_t dummy2[43];
	uint8_t data_load_list[0x100]; // TODO TODO TODO verify size
} data_load_list_struct;
auto& data_load_list = data_load_list_struct.data_load_list;

uint16_t did_init_sound;

struct Color {
	uint8_t rgb[3];
};

Color palette1[0x100];
Color palette2[0x100];

Color color1;
Color color2;

uint16_t word_28526;
uint16_t word_28880;
uint16_t word_2AA86;
uint16_t word_317D9;
uint16_t word_30ED8[1];
uint16_t word_31338[1];
uint16_t word_28524;
uint16_t word_2887E;
uint16_t word_2AA84;
uint8_t byte_317CE;

Color byte_2AA88;

const uint16_t loadListChunksA[0x30] = {
	0xC6,  0xC8,  0xCA,  0xCC,  0x28,  0x2A,  0x2C,  0x2E,
	0x30,  0x32,  0x34,  0x4B,  0x4D,  0x4F,  0x51,  0x53,
	0x55,  0x72,  0x74,  0x76,  0x78,  0x7A,  0x7C,  0x7E,
	0x80,  0x9C,  0x9E,  0xA0,  0xA2,  0xA4,  0xA6,  0xA8,
	0xAA,  0xCE,  0xD0,  0xD2,  0xD4,  0x171, 0x17D, 0x186,
	0x18C, 0x192, 0x17E, 0x1AF, 0x1B0, 0x1B1, 0x1B2, 0xDA
};

const uint16_t loadListChunksB[0x30] = {
	0x1C1, 0x1C1, 0x1C1, 0x1C1, 0x1C2, 0x1C2,  0x1C2, 0x1C2,
	0x1C2, 0x1C2, 0x1C2, 0x1C3, 0x1C3, 0x1C3,  0x1C3, 0x1C3,
	0x1C3, 0x1C4, 0x1C4, 0x1C4, 0x1C4, 0x1C4,  0x1C4, 0x1C4,
	0x1C4, 0x1C5, 0x1C5, 0x1C5, 0x1C5, 0x1C5,  0x1C5, 0x1C5,
	0x1C5, 0x1C1, 0x1C1, 0x1C1, 0x1C1, 0xFFFF, 0x1C6, 0x1C6,
	0x1C6, 0x1C6, 0x1C6, 0x1C6, 0x1C6, 0x1C6,  0x1C6, 0x1C6
};

uint8_t byte_2AA63;
uint8_t byte_2AA64[8];
uint8_t byte_2AA6C[8];
uint8_t byte_2AA74[8];
uint8_t byte_2AA7C[8];

uint16_t loaded_chunks11[0x20];
uint16_t loaded_chunks_end11[0x20];

uint16_t loaded_chunks2[16];
uint8_t* loaded_chunks2_ptr[16];

uint16_t video_levelWidth; // addr seg04:25A4
uint16_t video_levelHeight; // addr seg04:25A6

uint16_t word_31448[0x100]; // addr seg04:8F68

uint16_t word_2AABC; // addr seg04:25DC
uint16_t word_31648; // addr seg04:9168
uint16_t word_2AABE; // addr seg04:25DE
uint16_t word_3164A; // addr seg04:916A

uint16_t word_2AA8D;
