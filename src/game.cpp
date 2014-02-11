#include "game.hpp"
#include "draw.hpp"
#include "vga_emu.hpp"
#include "vars.hpp"
#include "data.hpp"

struct LevelHeader {
	uint8_t music_begin_command;
	uint8_t music_track;
	uint8_t music_end_command;

	uint8_t special_objs_type;
	uint16_t special_obj_x;
	uint16_t special_obj_y;
	uint16_t special_obj_type;
	uint16_t special_obj_flags;
	uint16_t special_obj_userdata;

	uint16_t next_level;

	uint16_t snes_sprite_size;

	uint8_t level_flags;

	uint16_t width_in_tiles;
	uint16_t height_in_tiles;

	ChunkId tilemap_chunk;
	ChunkId tileset_chunk;
	ChunkId tiledef_chunk;

	std::vector<uint8_t> load_list;
};

static int next_level = LEVEL_INTERPLAY;

static int current_level = -1;
static LevelHeader level_header;

// chunk id of the currently loaded world data
static ChunkId current_world_chunk;
static std::vector<uint8_t> world_data;

static const int NUM_LEVELS = 48;

const std::array<ChunkId, NUM_LEVELS> level_header_chunks = {
	// Spaceship
	0x0C6, 0x0C8, 0x0CA, 0x0CC, // 0
	// Jungle
	0x028, 0x02A, 0x02C, 0x02E, // 4
	0x030, 0x032, 0x034,
	// Desert
	0x04B, 0x04D, 0x04F, 0x051, // 11
	0x053, 0x055,
	// Machine
	0x072, 0x074, 0x076, 0x078, // 17
	0x07A, 0x07C, 0x07E, 0x080,
	// Wacky
	0x09C, 0x09E, 0x0A0, 0x0A2, // 25
	0x0A4, 0x0A6, 0x0A8, 0x0AA,
	// Final Spaceship
	0x0CE, 0x0D0, 0x0D2, 0x0D4, // 33
	// Respawn
	0x171, // 37
	// Main Menu
	0x17D, // 38
	// Interplay, Blizzard
	0x186, 0x18C, // 39
	// Warp
	0x192, // 41
	// Main Menu 2
	0x17E, // 42
	// Intros
	0x1AF, 0x1B0, // 43
	// Endings
	0x1B1, 0x1B2, // 45
	// Credits
	0x0DA // 47
};

enum WorldChunks {
	WORLD_SPACESHIP = 0x1C1,
	WORLD_JUNGLE = 0x1C2,
	WORLD_DESERT = 0x1C3,
	WORLD_MACHINE = 0x1C4,
	WORLD_WACKY = 0x1C5,
	WORLD_SYSTEM = 0x1C6, // Menus, cutscenes, etc.
};

const std::array<ChunkId, NUM_LEVELS> level_world_chunks = {
	// Spaceship
	WORLD_SPACESHIP, WORLD_SPACESHIP, WORLD_SPACESHIP, WORLD_SPACESHIP, // 0
	// Jungle
	WORLD_JUNGLE, WORLD_JUNGLE, WORLD_JUNGLE, WORLD_JUNGLE, // 4
	WORLD_JUNGLE, WORLD_JUNGLE, WORLD_JUNGLE,
	// Desert
	WORLD_DESERT, WORLD_DESERT, WORLD_DESERT, WORLD_DESERT, // 11
	WORLD_DESERT, WORLD_DESERT,
	// Machine
	WORLD_MACHINE, WORLD_MACHINE, WORLD_MACHINE, WORLD_MACHINE, // 17
	WORLD_MACHINE, WORLD_MACHINE, WORLD_MACHINE, WORLD_MACHINE,
	// Wacky
	WORLD_WACKY, WORLD_WACKY, WORLD_WACKY, WORLD_WACKY, // 25
	WORLD_WACKY, WORLD_WACKY, WORLD_WACKY, WORLD_WACKY,
	// Final Spaceship
	WORLD_SPACESHIP, WORLD_SPACESHIP, WORLD_SPACESHIP, WORLD_SPACESHIP, // 33
	// Respawn
	0xFFFF, // 37
	// Main Menu
	WORLD_SYSTEM, // 38
	// Interplay, Blizzard
	WORLD_SYSTEM, WORLD_SYSTEM, // 39
	// Warp
	WORLD_SYSTEM, // 41
	// Main Menu 2
	WORLD_SYSTEM, // 42
	// Intros
	WORLD_SYSTEM, WORLD_SYSTEM, // 43
	// Endings
	WORLD_SYSTEM, WORLD_SYSTEM, // 45
	// Credits
	WORLD_SYSTEM // 47
};

static void loadLevelHeader(const int level_id) {
	const std::vector<uint8_t> level_header_data = decompressChunk(level_header_chunks[level_id]);
	MemoryStream s(level_header_data.data(), level_header_data.size());

	auto& h = level_header;

	s.skip(4);
	h.music_begin_command = s.read8();
	h.music_track = s.read8();
	h.music_end_command = s.read8();
	h.special_objs_type = s.read8();
	h.special_obj_x = s.read16();
	h.special_obj_y = s.read16();
	h.special_obj_type = s.read16();
	h.special_obj_flags = s.read16();
	h.special_obj_userdata = s.read16();
	s.skip(4);
	h.next_level = s.read16();
	h.snes_sprite_size = s.read16();
	s.skip(2);
	h.level_flags = s.read8();
	s.skip(12);
	h.width_in_tiles = s.read16();
	h.height_in_tiles = s.read16();
	s.skip(1);
	h.tilemap_chunk = s.read16();
	h.tileset_chunk = s.read16();
	h.tiledef_chunk = s.read16();
	s.skip(15);
	const size_t load_list_size = s.bytesRemaining();
	const uint8_t* load_list = s.getBlock(load_list_size);
	h.load_list.assign(load_list, load_list + load_list_size);
}

static void loadLevel(const int level_id) {
	assert(level_id < NUM_LEVELS);
	current_level = level_id;

	const int world_chunk = level_world_chunks[level_id];

	// 0xFFFF is used by the Respawn level to keep the previous world loaded
	if (world_chunk != 0xFFFF && world_chunk != current_world_chunk) {
		current_world_chunk = world_chunk;
		world_data = decompressChunk(world_chunk);
	}

	loadLevelHeader(level_id);
}

void game_frame(std::vector<SDL_Event>& events) {
	if (next_level != -1) {
		loadLevel(next_level);
		next_level = -1;
	}

	vga_window.present();
}
