#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <cstring>

// TODO
void hookKeyboard();
void unhookKeyboard();
void restoreVideo();
void restoreErrorInt();
uint32_t curSystemTime();

// TODO
void hookKeyboard() {}
void unhookKeyboard() {}
void restoreVideo() {}
void restoreErrorInt() {}
uint32_t curSystemTime() { return 0; }

inline uint16_t load16LE(uint8_t* d) {
	return d[0] | d[1] << 8;
}

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
uint16_t ptr1_offset;
uint8_t* ptr1_seg;

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

void hookInts() {
	// init memory and segments();
	
	hookKeyboard();

	start_time = curSystemTime();
}

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

void roundToNextSeg(uint8_t** seg, uint16_t* off) {
	// What am I supposed to do here?
	*seg += *off;
	*off = 0;
}

// Note: Originally this function returned es:di on the end of the buffer. Now I just return the pointer to it
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

	int read_size = chunk_offsets[1] - chunk_offsets[0];
	if (read_size >= 0xB080)
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
					char tmpfn[20];
					sprintf(tmpfn, "chunk%03x.bin", chunk_id);
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
						char tmpfn[20];
						sprintf(tmpfn, "chunk%03x.bin", chunk_id);
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

void allocMemAndLoadData() {
	alloc_seg1 = new uint8_t[0x8B80];
	alloc_seg2 = new uint8_t[0x2000];
	alloc_seg0 = new uint8_t[0x3600];
	alloc_seg3 = new uint8_t[0x3130];
	alloc_seg6 = new uint8_t[0xC080];
	alloc_seg11 = new uint8_t[0x7000];

	// Load sound data if sound enabled
	if (!(data_header1_snd1 && data_header1_snd2 && 0x8000)) {
		uint8_t* buf = new uint8_t[0xE470];
		ptr2_seg = buf;
		ptr2_offset = 0;

		// Note: roundToNextSeg not needed because segments are for 16-bit losers
		// I'm not sure these are quite right, they seem to point to the end of the data
		// but looking at the original that's effectively what happens?
		sound_buffer0 = buf = decompressChunk(0x1C7 + data_header1.field_6, buf);
		sound_buffer1 = buf = decompressChunk(0x20C + data_header1.field_8, buf);
		sound_buffer2 = decompressChunk(0x207 + data_header1.field_8, buf);
	}

	ptr3_seg = ptr1_seg = new uint8_t[0x2ABA0];
	ptr3_offset = ptr1_offset = 0;

	alloc_seg5 = new uint8_t[0xC000];

	loaded_chunk0 = 0x1C6;
	decompressChunk(loaded_chunk0, alloc_seg5);

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

void freeMemAndQuit() {
	unhookKeyboard();
	// TODO one call

	delete[] alloc_seg1;
	delete[] alloc_seg0;
	delete[] alloc_seg2;
	delete[] alloc_seg3;
	delete[] alloc_seg5;
	delete[] ptr3_seg;

	std::fclose(data_file);

	restoreVideo();
	restoreErrorInt();

	std::exit(0);
}

int main() {
	hookInts();
	hwCheck();
	allocMemAndLoadData();
	// TODO
}
