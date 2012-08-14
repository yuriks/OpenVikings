#include "vikings.hpp"
#include "vm.hpp"

#include <array>
#include <cassert>
#include "vars.hpp"
#include "main.hpp"
#include "input.hpp"

// seg04:008A
static uint16_t vm_regA;

struct VMState
{
	bool run;
	uint16_t return_si;
	const uint8_t* rom;

	uint16_t ip;

	uint8_t readImm8()
	{
		return rom[ip++];
	}

	uint16_t readImm16()
	{
		uint16_t tmp = load16LE(&rom[ip]);
		ip += 2;
		return tmp;
	}
};

static void sub_1303A(VMState& vm);

// addr seg00:53EA
static uint16_t vm_checkBit(uint16_t bit_pos, uint16_t value)
{
	if ((value & BIT(bit_pos)) == 0)
		return 0;
	else
		return 1;
}

typedef void (*OpcodeHandlerPtr)(VMState& vm);

static void op_unsupported(VMState& vm)
{
	errorQuit("Unsupported opcode.", vm.rom[vm.ip - 1]);
}

static void op_stopScript(VMState& vm)
{
	vm.run = false;
	vm.return_si = word_28522;

	obj_script_resume[word_28522] = vm.ip;
}

static void op_nop(VMState& vm)
{
}

static void op_jumpToImmediate(VMState& vm)
{
	vm.ip = load16LE(&vm.rom[vm.ip]);
}

static void op_loadImmediateWord(VMState& vm)
{
	vm_regA = vm.readImm16();
}

static void op_testBitAndSet(VMState& vm)
{
	uint8_t a = vm.readImm8() / 2;
	uint16_t b = vm.readImm16();
	vm_regA = vm_checkBit(a, b);
}

static void op_testBitJumpEq(VMState& vm)
{
	uint8_t a = vm.readImm8() / 2;
	uint16_t b = vm.readImm16();

	if (vm_checkBit(a, b) == vm_regA)
		op_jumpToImmediate(vm);
	else
		vm.ip += 2;
}

// TODO name
static void op_obj_unknown1(VMState& vm)
{
	word_29EED[word_28522] = vm.readImm16();
	word_29F15[word_28522] = 1;
}

// addr seg00:35CF
static void sub_135CF(int di)
{
	if (word_298FD[di] != 0)
		return;

	if (word_29A65[di] & BIT(15))
	{
		word_29B55[di] += level_header.anonymous_0;
	}

	if (word_29A65[di] & BIT(14))
	{
		word_29B2D[di] += level_header.level_header_start;
	}

	int16_t ax = word_29B2D[di];
	if (ax < 0)
	{
		if (-ax >= word_29C6D[di])
			ax = -word_29C6D[di];
	}
	else
	{
		if (ax >= word_29C6D[di])
			ax = word_29C6D[di];
	}
	word_29B2D[di] = ax;

	ax = word_29B55[di];
	if (ax < 0)
	{
		if (-ax >= word_29C95[di])
			ax = -word_29C95[di];
	}
	else
	{
		if (ax >= word_29C95[di])
			ax = word_29C95[di];
	}
	word_29B55[di] = ax;

	if (word_29A65[di] & BIT(6))
		ax = -word_29B2D[di];
	else
		ax = word_29B2D[di];
	word_29E25[di] += ax;

	if (word_29A65[di] & BIT(7))
		ax = -word_29B55[di];
	else
		ax = word_29B55[di];
	word_29E4D[di] += ax;
}

// TODO name
static void op_sub_13031(VMState& vm)
{
	VMState new_vm = vm;
	sub_1303A(new_vm);
	sub_135CF(word_28522);
}

std::array<uint16_t*, 34> word_31826 =
{
	word_299C5, word_299ED, word_29A15, word_29A3D, // 0
	word_29A65, word_29A8D, word_29AB5, word_29ADD, // 4
	word_29B05, word_29B2D, word_29B55, word_29B7D, // 8
	reinterpret_cast<uint16_t*>(word_29BA5), // 12
	reinterpret_cast<uint16_t*>(word_29BCD), // 13
	word_29BF5, word_29C1D, word_29C45, word_29C6D, // 14
	word_29C95, word_29CBD, word_29CE5, word_29D0D, // 18
	word_29D35, word_29D5D, word_29D85, word_29DAD, // 22
	word_29DD5, word_29DFD, word_29E25, word_29E4D, // 26
	word_29E75, word_29E9D, word_29EC5, word_29EED, // 30
};

// TODO name
static void op_sub_147E7(VMState& vm)
{
	uint16_t* si = word_31826[vm.readImm8() / 2];
	si[word_28522] |= vm_regA;
}

// addr seg00:5485
static uint16_t sub_15485(VMState& vm)
{
	uint16_t* si = word_31826[vm.readImm8() / 2];
	return si[word_28522];
}

// TODO name
static void op_sub_14A6B(VMState& vm)
{
	if (sub_15485(vm) != vm_regA)
		op_jumpToImmediate(vm);
	else
		vm.ip += 2;
}

// TODO name
static void op_loc_14686(VMState& vm)
{
	uint16_t* si = word_31826[vm.readImm8() / 2];
	si[word_28522] = vm_regA;
}

// TODO name
static void op_sub_14A1B(VMState& vm)
{
	if (sub_15485(vm) == vm_regA)
		op_jumpToImmediate(vm);
	else
		vm.ip += 2;
}

static const int NUM_TRANSLATED_ADDRESSES = 9;

namespace
{
struct AddressTranslation
{
	uint16_t address;
	uint16_t* variable;
};
}

static const std::array<AddressTranslation, NUM_TRANSLATED_ADDRESSES> ram_address_translations =
{{
	{0x0200, &word_286E0},
	{0x0202, &word_286E2},
	{0x0204, &word_286E4},
	{0x0236, &word_28716},
	{0x028C, &last_typed_key},
	{0x0348, &word_28828},
	{0x03B8, &buttons_pressed},
	{0x03C2, &word_288A2},
	{0x03CC, &word_288AC},
}};

static uint16_t* translateRamAddress(uint16_t addr)
{
	for (int i = 0; i < NUM_TRANSLATED_ADDRESSES; ++i)
	{
		auto& entry = ram_address_translations[i];
		if (entry.address == addr)
			return entry.variable;
	}

	errorQuit("Untranslated VM RAM address.", addr);
	return nullptr;
}

static uint16_t vm_checkBitRam(VMState& vm)
{
	uint16_t a = vm.readImm8() / 2;
	uint16_t* b = translateRamAddress(vm.readImm16());

	return vm_checkBit(a, *b);
}

// TODO name
static void op_sub_14DB1(VMState& vm)
{
	if (vm_checkBitRam(vm) == vm_regA)
		op_jumpToImmediate(vm);
	else
		vm.ip += 2;
}

// TODO name
static void op_sub_14B59(VMState& vm)
{
	vm_regA = vm_checkBitRam(vm);
}

// TODO name
static void op_sub_1431C(VMState& vm)
{
	word_28814 |= W28814_GOTO_NEXT_LEVEL;

	vm.return_si = word_28522;
	vm.run = false;
}

// addr seg00:3C93
static void sub_13C93(int di)
{
	if (word_29FB5[di] != 0)
	{
		assert(false); // TODO
	}

	uint16_t si = word_29CE5[di];
	if (si != 0xFFFF)
	{
		word_29D0D[si] = 0xFFFF;
	}

	si = word_29D0D[di];
	if (si != 0xFFFF)
	{
		word_29CE5[si] = 0xFFFF;
	}

	word_29835[di] = 0;
	word_29EED[di] = 0xFFFF;

	int di2 = di + 1;
	if (word_28852 == di2)
	{
		do
		{
			di2 -= 1;
		}
		while (di2 >= 0 && word_29835[di2] == 0);

		di2 += 1;
		word_28852 = di2;
	}

	if (word_29BA5[di] != 0xFFFF && (word_29A65[di] & BIT(8)))
	{
		sub_139EF();
		si = word_29BA5[di];
		// TODO
	}
}

// TODO name
static void op_sub_14327(VMState& vm)
{
	sub_13C93(word_28522);

	vm.return_si = word_28522;
	vm.run = false;
}

static void op_loc_146A3(VMState& vm)
{
	uint16_t si = vm.readImm16();
	*translateRamAddress(si) = vm_regA;
}

static void op_sub_14A2B(VMState& vm)
{
	uint16_t ax = *translateRamAddress(vm.readImm16());

	if (ax == vm_regA)
		op_jumpToImmediate(vm);
	else
		vm.ip += 2;
}

static void op_sub_14BA7(VMState& vm)
{
	uint16_t si = vm.readImm8() / 2;

	if (vm_regA != 0)
	{
		vm_regA = BIT(si);
	}

	uint16_t dx = ~BIT(si);

	uint16_t* var = translateRamAddress(vm.readImm16());
	*var = (*var & dx) | vm_regA;
}

static void op_sub_142C1(VMState& vm)
{
	word_2985D[word_28522] = vm.ip + 2;
	op_jumpToImmediate(vm);
}

static void op_sub_14532(VMState& vm)
{
	color1.rgb[0] = vm.readImm8() << 1;
	color1.rgb[1] = vm.readImm8() << 1;
	color1.rgb[2] = vm.readImm8() << 1;

	byte_303DD |= BIT(0);
	palette_action = PALACT_COPY;
	pal_pointer = palette2;

	fadePalKeepFirst();
}

static void op_sub_142D3(VMState& vm)
{
	vm.ip = word_2985D[word_28522];
}

static void op_sub_1434C(VMState& vm)
{
	uint8_t ax = vm.readImm8();

	if (ax == 0xD9)
	{
		assert(false); // TODO
	}
	else
	{
		vm.ip += 2;
	}

	if (ax == 0x11)
	{
		vga_vramCopy(0, 0x2ADC, 0x1600);
		vga_vramCopy(0, 0x70BC, 0x1600);

		vga_fillVRAM(0xF, 0, 0x0000, 0x2ADC);
		vga_fillVRAM(0xF, 0, 0x40DC, 0x2FE0);
		vga_fillVRAM(0xF, 0, 0x86BC, 0x7000);
	}
	else if (ax == 1)
	{
		freeMemAndQuit();
	}
}

static void op_sub_14590(VMState& vm)
{
	color1.rgb[0] = 0;
	color1.rgb[1] = 0;
	color1.rgb[2] = 0;

	byte_303DD &= ~BIT(0);
	if (byte_303DD == 0)
	{
		pal_pointer = palette1;
	}

	palette_action = PALACT_COPY;
	fadePalKeepFirst();
}

static void op_sub_145B5(VMState& vm)
{
	color2.rgb[0] = 0;
	color2.rgb[1] = 0;
	color2.rgb[2] = 0;

	byte_303DD &= ~BIT(1);
	if (byte_303DD == 0)
	{
		pal_pointer = palette1;
	}

	palette_action = PALACT_COPY;
	fadePalKeepFirst();
}

static void op_sub_1268D(VMState& vm)
{
	word_2A287[word_2A66F] = 6;
	word_2A287[word_2A66F + 1] = vm.readImm16();
	word_2A66F += 2;
}

// addr seg00:547E
static uint16_t sub_1547E(VMState& vm)
{
	return vm.readImm16();
}

// addr seg00:549A
static uint16_t sub_1549A(VMState& vm)
{
	return *translateRamAddress(vm.readImm16());
}

// addr seg00:54A3
static uint16_t sub_154A3(VMState& vm)
{
	uint16_t si = vm.readImm8() / 2;
	uint16_t* di = word_31826[si];
	si = word_28522;

}

uint16_t sub_15473(VMState& vm, uint16_t ax)
{
	typedef uint16_t (*FuncPtr)(VMState& vm);
	static const std::array<FuncPtr, 5> off_30C98 =
	{{
		sub_1547E,
		sub_15485,
		sub_1549A,
		nullptr, // TODO sub_154A3
		nullptr, // TODO sub_12312
	}};

	return off_30C98[ax & 0x7](vm);
}

void calcStringAddress(VMState& vm)
{
	uint16_t ax = vm.readImm8();
	word_28512 = ax;
	uint16_t si = sub_15473(vm, ax);

	word_2850A = load16LE(&script_data[si * 2]);
}

void getTextPosition(VMState& vm, uint16_t *out_x, uint16_t *out_y)
{
	uint16_t ax = vm.readImm8();

	word_2854C = sub_15473(vm, ax);
	*out_x = word_2854C;
	*out_y = sub_15473(vm, ax >> 3);
}

static void op_sub_12634(VMState& vm)
{
	calcStringAddress(vm);
	uint16_t pos_x, pos_y;
	getTextPosition(vm, &pos_x, &pos_y);

	word_2A287[word_2A66F] = 5; // Command (draw text?)
	word_2A287[word_2A66F + 1] = pos_x; // X Position
	word_2A287[word_2A66F + 2] = pos_y; // Y Position
	word_2A287[word_2A66F + 3] = word_2850A; // String address
	word_2A66F += 4;
}

static void op_sub_14604(VMState& vm)
{
	for (int i = word_29F65[word_28522]; i < word_29F8D[word_28522]; ++i)
	{
		word_2892D[i] |= ~(BIT(14) | BIT(13));
		word_2962D[i] = 2;
	}
}

static const int MAX_OPCODES = 216;
static const std::array<OpcodeHandlerPtr, MAX_OPCODES> opcode_table =
{{
	op_stopScript, // 0x00
	op_nop, // 0x01
	op_unsupported, // sub_177B2,      // 0x02
	op_jumpToImmediate, // 0x03
	op_unsupported, // sub_1782A,      // 0x04
	op_sub_142C1, // 0x05
	op_sub_142D3, // 0x06
	op_unsupported, // sub_1368C,      // 0x07
	op_unsupported, // sub_1367C,      // 0x08
	op_unsupported, // sub_13743,      // 0x09
	op_unsupported, // sub_13733,      // 0x0A
	op_unsupported, // sub_1369C,      // 0x0B
	op_unsupported, // sub_13753,      // 0x0C
	op_unsupported, // sub_142DC,      // 0x0D
	op_unsupported, // sub_142FC,      // 0x0E
	op_sub_1431C, // 0x0F
	op_sub_14327, // 0x10
	op_unsupported, // sub_14334,      // 0x11
	op_unsupported, // sub_14340,      // 0x12
	op_sub_1434C, // 0x13
	op_unsupported, // sub_14F59,      // 0x14
	op_unsupported, // sub_150FC,      // 0x15
	op_unsupported, // sub_15106,      // 0x16
	op_unsupported, // sub_143F2,      // 0x17
	op_unsupported, // sub_14409,      // 0x18
	op_obj_unknown1, // 0x19
	op_unsupported, // sub_1559C,      // 0x1A
	op_unsupported, // sub_143FE,      // 0x1B
	op_unsupported, // sub_1443D,      // 0x1C
	op_unsupported, // sub_15686,      // 0x1D
	op_unsupported, // sub_1444F,      // 0x1E
	op_unsupported, // sub_14469,      // 0x1F
	op_unsupported, // sub_144A9,      // 0x20
	op_unsupported, // sub_14483,      // 0x21
	op_unsupported, // sub_14453,      // 0x22
	op_unsupported, // sub_1446D,      // 0x23
	op_unsupported, // sub_144AD,      // 0x24
	op_unsupported, // sub_14487,      // 0x25
	op_unsupported, // sub_14EDD,      // 0x26
	op_unsupported, // sub_14F09,      // 0x27
	op_unsupported, // sub_14F27,      // 0x28
	op_unsupported, // sub_15017,      // 0x29
	op_unsupported, // sub_15078,      // 0x2A
	op_unsupported, // sub_15039,      // 0x2B
	op_unsupported, // sub_15F2C,      // 0x2C
	op_unsupported, // sub_15F25,      // 0x2D
	op_unsupported, // sub_1522C,      // 0x2E
	op_sub_13031, // 0x2F
	op_unsupported, // sub_144CF,      // 0x30
	op_unsupported, // sub_144D3,      // 0x31
	op_unsupported, // sub_15772,      // 0x32
	op_unsupported, // sub_157D5,      // 0x33
	op_unsupported, // sub_150B5,      // 0x34
	op_unsupported, // sub_15E91,      // 0x35
	op_unsupported, // sub_15E8A,      // 0x36
	op_unsupported, // sub_155C0,      // 0x37
	op_unsupported, // sub_156AA,      // 0x38
	op_unsupported, // sub_145DA,      // 0x39
	op_unsupported, // sub_14340,      // 0x3A
	op_unsupported, // sub_1524A,      // 0x3B
	op_unsupported, // sub_15838,      // 0x3C
	op_sub_14532, // 0x3D
	op_sub_14590, // 0x3E
	op_unsupported, // sub_145E5,      // 0x3F
	op_sub_14604, // 0x40
	op_unsupported, // sub_1242E,      // 0x41
	op_unsupported, // sub_12669,      // 0x42
	op_unsupported, // sub_1267B,      // 0x43
	op_unsupported, // sub_1246D,      // 0x44
	op_sub_12634, // 0x45
	op_sub_1268D, // 0x46
	op_unsupported, // locret_141F6,   // 0x47
	op_unsupported, // loc_14FEC,      // 0x48
	op_unsupported, // loc_14FC4,      // 0x49
	op_unsupported, // loc_14FC8,      // 0x4A
	op_unsupported, // loc_16252,      // 0x4B
	op_unsupported, // loc_14561,      // 0x4C
	op_sub_145B5, // 0x4D
	op_unsupported, // loc_144FD,      // 0x4E
	op_unsupported, // loc_14501,      // 0x4F
	op_unsupported, // loc_126A9,      // 0x50
	op_loadImmediateWord, // 0x51
	op_unsupported, // sub_1462E,      // 0x52
	op_unsupported, // sub_14646,      // 0x53
	op_unsupported, // sub_14652,      // 0x54
	op_unsupported, // sub_1466E,      // 0x55
	op_loc_14686, // 0x56
	op_loc_146A3, // 0x57
	op_unsupported, // loc_146B4,      // 0x58
	op_unsupported, // loc_146DE,      // 0x59
	op_unsupported, // loc_14704,      // 0x5A
	op_unsupported, // loc_14721,      // 0x5B
	op_unsupported, // loc_1474B,      // 0x5C
	op_unsupported, // loc_14771,      // 0x5D
	op_unsupported, // loc_1478B,      // 0x5E
	op_unsupported, // loc_147A7,      // 0x5F
	op_unsupported, // loc_147BF,      // 0x60
	op_unsupported, // loc_147CB,      // 0x61
	op_sub_147E7, // 0x62
	op_unsupported, // loc_147FF,      // 0x63
	op_unsupported, // loc_1480B,      // 0x64
	op_unsupported, // loc_14827,      // 0x65
	op_unsupported, // loc_1483F,      // 0x66
	op_unsupported, // loc_1484B,      // 0x67
	op_unsupported, // loc_14867,      // 0x68
	op_unsupported, // loc_14879,      // 0x69
	op_unsupported, // loc_1488B,      // 0x6A
	op_unsupported, // loc_1489D,      // 0x6B
	op_unsupported, // loc_148AF,      // 0x6C
	op_unsupported, // loc_148C1,      // 0x6D
	op_unsupported, // loc_148D3,      // 0x6E
	op_unsupported, // loc_148E5,      // 0x6F
	op_unsupported, // loc_148F7,      // 0x70
	op_unsupported, // loc_14909,      // 0x71
	op_unsupported, // loc_14A0B,      // 0x72
	op_sub_14A1B, // 0x73
	op_sub_14A2B, // 0x74
	op_unsupported, // loc_14A3B,      // 0x75
	op_unsupported, // loc_14A4B,      // 0x76
	op_unsupported, // loc_14A5B,      // 0x77
	op_sub_14A6B, // 0x78
	op_unsupported, // loc_14A7B,      // 0x79
	op_unsupported, // loc_14A8B,      // 0x7A
	op_unsupported, // loc_14A9B,      // 0x7B
	op_unsupported, // loc_1491B,      // 0x7C
	op_unsupported, // loc_14933,      // 0x7D
	op_unsupported, // loc_1494B,      // 0x7E
	op_unsupported, // loc_14963,      // 0x7F
	op_unsupported, // loc_1497B,      // 0x80
	op_unsupported, // loc_14993,      // 0x81
	op_unsupported, // loc_149AB,      // 0x82
	op_unsupported, // loc_149C3,      // 0x83
	op_unsupported, // loc_149DB,      // 0x84
	op_unsupported, // loc_149F3,      // 0x85
	op_unsupported, // loc_14AAB,      // 0x86
	op_unsupported, // loc_14ABB,      // 0x87
	op_unsupported, // loc_14ACB,      // 0x88
	op_unsupported, // loc_14ADB,      // 0x89
	op_unsupported, // loc_14AEB,      // 0x8A
	op_unsupported, // loc_14AFB,      // 0x8B
	op_unsupported, // loc_14B0B,      // 0x8C
	op_unsupported, // loc_14B1B,      // 0x8D
	op_unsupported, // loc_14B2B,      // 0x8E
	op_unsupported, // loc_14B3B,      // 0x8F
	op_unsupported, // loc_146D0,      // 0x90
	op_unsupported, // loc_146F6,      // 0x91
	op_unsupported, // loc_14713,      // 0x92
	op_unsupported, // loc_1473D,      // 0x93
	op_unsupported, // loc_14763,      // 0x94
	op_unsupported, // loc_1477D,      // 0x95
	op_unsupported, // loc_14675,      // 0x96
	op_testBitAndSet, // 0x97
	op_unsupported, // loc_14B52,      // 0x98
	op_sub_14B59, // 0x99
	op_unsupported, // loc_14B60,      // 0x9A
	op_unsupported, // loc_14B67,      // 0x9B
	op_unsupported, // loc_14B71,      // 0x9C
	op_sub_14BA7, // 0x9D
	op_unsupported, // loc_14BCF,      // 0x9E
	op_unsupported, // loc_14C09,      // 0x9F
	op_unsupported, // loc_14C37,      // 0xA0
	op_unsupported, // loc_14C59,      // 0xA1
	op_unsupported, // loc_14D0F,      // 0xA2
	op_unsupported, // loc_14D3D,      // 0xA3
	op_unsupported, // loc_14D5F,      // 0xA4
	op_unsupported, // loc_14C8D,      // 0xA5
	op_unsupported, // loc_14CBB,      // 0xA6
	op_unsupported, // loc_14CDD,      // 0xA7
	op_testBitJumpEq, // 0xA8
	op_unsupported, // loc_14DA1,      // 0xA9
	op_sub_14DB1, // 0xAA
	op_unsupported, // loc_14DC1,      // 0xAB
	op_unsupported, // loc_14DD1,      // 0xAC
	op_unsupported, // loc_14DE4,      // 0xAD
	op_unsupported, // loc_14DF4,      // 0xAE
	op_unsupported, // loc_14E04,      // 0xAF
	op_unsupported, // loc_14E14,      // 0xB0
	op_unsupported, // loc_14E24,      // 0xB1
	op_unsupported, // loc_14E37,      // 0xB2
	op_unsupported, // loc_14E47,      // 0xB3
	op_unsupported, // loc_14E57,      // 0xB4
	op_unsupported, // loc_14E67,      // 0xB5
	op_unsupported, // loc_14E77,      // 0xB6
	op_unsupported, // loc_14E8A,      // 0xB7
	op_unsupported, // loc_14E9A,      // 0xB8
	op_unsupported, // loc_14EAA,      // 0xB9
	op_unsupported, // loc_14EBA,      // 0xBA
	op_unsupported, // loc_14ECA,      // 0xBB
	op_unsupported, // loc_14681,      // 0xBC
	op_unsupported, // loc_1469E,      // 0xBD
	op_unsupported, // loc_146AF,      // 0xBE
	op_unsupported, // loc_1515C,      // 0xBF
	op_unsupported, // loc_1518A,      // 0xC0
	op_unsupported, // loc_151F2,      // 0xC1
	op_unsupported, // loc_151B8,      // 0xC2
	op_unsupported, // loc_15160,      // 0xC3
	op_unsupported, // loc_1518E,      // 0xC4
	op_unsupported, // loc_151F6,      // 0xC5
	op_unsupported, // loc_151BC,      // 0xC6
	op_unsupported, // loc_15268,      // 0xC7
	op_unsupported, // loc_1527B,      // 0xC8
	op_unsupported, // loc_1529A,      // 0xC9
	op_unsupported, // loc_152B3,      // 0xCA
	op_unsupported, // sub_1267B,      // 0xCB
	op_unsupported, // loc_152DE,      // 0xCC
	op_unsupported, // loc_152CA,      // 0xCD
	op_unsupported, // loc_152D6,      // 0xCE
	op_unsupported, // loc_152C6,      // 0xCF
	op_unsupported, // loc_15E7C,      // 0xD0
	op_unsupported, // loc_15F17,      // 0xD1
	op_unsupported, // loc_1287A,      // 0xD2
	op_unsupported, // loc_12829,      // 0xD3
	op_unsupported, // loc_1531C,      // 0xD4
	op_unsupported, // loc_178D6,      // 0xD5
	op_unsupported, // loc_178F1,      // 0xD6
	op_unsupported, // loc_1787F,      // 0xD7
}};

std::FILE* trace_file = nullptr;

// addr seg00:424C
uint16_t runObjectScript(int object_slot)
{
	if (word_29835[object_slot] == nullptr)
		return object_slot;

	if ((word_29A65[object_slot] & BIT(12)) && word_29BF5[object_slot] != 0)
	{
		word_29BF5[object_slot] -= 1;
	}

	VMState vm;
	vm.run = true;
	vm.rom = world_data;

	word_28522 = object_slot;
	word_2886E = 0;
	vm.rom = word_29835[object_slot];

	if ((word_29A65[object_slot] & BIT(9)) || word_2880F != 0)
	{
		if (word_29BCD[object_slot] & BIT(15))
			return object_slot;

		int bx = word_29BCD[object_slot];
		vm.rom = world_data;
		WorldData* world_data_entry = reinterpret_cast<WorldData*>(world_data + bx*21);

		obj_script_resume[object_slot] = world_data_entry->field_3;
	}

	vm.ip = obj_script_resume[object_slot];

	if (trace_file == nullptr)
	{
		trace_file = std::fopen("tracefile.txt", "w");
	}

	do
	{
		uint8_t next_instruction = vm.readImm8();
		std::fprintf(trace_file, "%04x: %02x\n", vm.ip - 1, next_instruction);
		std::fflush(trace_file);
		opcode_table[next_instruction](vm);
	}
	while(vm.run);

	std::fprintf(trace_file, "----: returned\n");

	return vm.return_si;
}


static void op2_unsupported(VMState& vm)
{
	errorQuit("Unsupported opcode in sub-VM 2.", vm.rom[vm.ip - 1]);
}

static void op2_stop(VMState& vm)
{
	vm.run = false;
}

static void op2_sub_132DF(VMState& vm)
{
	static const uint16_t word_132D7[] = { 8, 32, 16, 176 };
	static uint8_t byte_1339E = 0;

	uint16_t di = word_2855C;
	uint16_t si = vm.readImm8();

	assert(si < 4);
	uint16_t ax = word_132D7[si];

	byte_1339E = 0;

	if (si <= 2)
	{
		si = BIT(si);
	}
	else
	{
		si = BIT(1);
		byte_1339E = 0xFF;
	}

	if (word_2886C == 0)
	{
		for (; di < word_28560; ++di)
		{
			if (byte_1339E != 0)
			{
				assert(false); // TODO
			}

			word_2912D[di] = ax;
			word_2892D[di] = word_2892D[di] & ~BIT_RANGE(0, 2) | si;
			word_2962D[di] = 0x202;
		}
	}
	else
	{
		assert(false); // TODO
	}
}

static void op2_sub_130EF(VMState& vm)
{
	uint16_t si = word_2855C;
	uint16_t di = word_28522;
	uint16_t cx = word_29D35[di];

	if (word_2886C == 0)
	{
		for (; si < word_28560; ++si)
		{
			word_28D2D[si] = 72 * vm.readImm8() + cx;
			word_2962D[si] = 0x202;
		}
	}
	else
	{
		assert(false); // TODO
	}
}

static void op2_sub_130A2(VMState& vm)
{
	uint16_t si = word_2855C;
	uint16_t cx = 72 * vm.readImm8();

	if (word_2886C == 0)
	{
		assert(false); // TODO
	}
	else
	{
		assert(false); // TODO
	}
}

static void op2_sub_1339F(VMState& vm)
{
	uint16_t si = word_2855C;

	if (word_2886C == 0)
	{
		for (; si < word_28560; ++si)
		{
			word_2892D[si] |= BIT(14);
			reinterpret_cast<uint8_t*>(word_2962D)[si * 2 + 1] = 2; // TODO check this
		}
	}
	else
	{
		for (; si < word_28560; ++si)
		{
			if (word_28A2D[si] & word_2886C)
			{
				word_2892D[si] |= BIT(14);
				reinterpret_cast<uint8_t*>(word_2962D)[si * 2 + 1] = 2; // TODO check this
			}
		}
	}
}

static void sub_136FC(uint16_t di)
{
	if (word_29FB5[di] != 0)
	{
		uint16_t dx = word_29C1D[di] * 2;
		uint16_t cx = word_29F8D[di];
		for (uint16_t i = word_29F65[di]; i < cx; ++i)
		{
			word_28B2D[i] = dx - word_28B2D[i] - word_2912D[i];
			word_2892D[i] |= BIT(9);
			word_2962D[i] = 0x202;
		}
	}
}

static void op2_sub_131A4(VMState& vm)
{
	uint16_t dx = word_29C1D[word_28522];

	for (uint16_t si = word_2855C; si < word_28560; ++si)
	{
		uint16_t ax = vm.readImm16() + dx;

		word_28B2D[si] = ax;

		if (!(word_2892D[si] & BIT(12)))
		{
			word_2922D[si] = ax;
			word_2942D[si] = ax;
			word_2892D[si] |= BIT(12);
		}

		word_2962D[si] = 0x202;
	}

	if (word_29A65[word_28522] & BIT(6))
	{
		sub_136FC(word_28522);
	}
}

static void sub_137B8(uint16_t si)
{
	if (word_29FB5[si] != 0)
	{
		uint16_t dx = word_29C45[si] * 2;
		uint16_t cx = word_29F8D[si];

		for (uint16_t di = word_29F65[si]; di < cx; ++di)
		{
			word_28C2D[di] = dx - word_28C2D[di] - word_2912D[di];
			word_2892D[di] |= BIT(10);
			word_2962D[di] = 0x202;
		}
	}
}

static void op2_sub_1323D(VMState& vm)
{
	uint16_t dx = word_29C45[word_28522];

	uint16_t si;
	for (si = word_2855C; si < word_28560; ++si)
	{
		uint16_t ax = vm.readImm16() + dx;

		word_28C2D[si] = ax;

		if (!(word_2892D[si] & BIT(11)))
		{
			word_2932D[si] = ax;
			word_2952D[si] = ax;
			word_2892D[si] |= BIT(11);
		}

		word_2962D[si] = 0x202;
	}

	if (word_29A65[si] & BIT(7))
	{
		sub_137B8(si);
	}
}

static void op2_jumpToImmediate(VMState& vm)
{
	vm.ip = load16LE(&vm.rom[vm.ip]);
}

static void op2_sub_1341F(VMState& vm)
{
	uint16_t si = word_2855C;

	if (word_2886C == 0)
	{
		for (; si < word_28560; ++si)
		{
			word_28A2D[si] = vm.readImm8();
		}
	}
	else
	{
		assert(false); // TODO
	}
}

static void op2_sub_1345E(VMState& vm)
{
	vm.return_si = word_2855C;
	word_2886C = vm.readImm8();
}

static void op2_sub_133DE(VMState& vm)
{
	uint16_t si = word_2855C;

	if (word_2886C == 0)
	{
		for (; si < word_28560; ++si)
		{
			word_2892D[si] |= ~(BIT(14) | BIT(13));
			word_2962D[si] = 2;
		}
	}
	else
	{
		for (; si < word_28560; ++si)
		{
			if (word_28A2D[si] & word_2886C)
			{
				word_2892D[si] |= ~(BIT(14) | BIT(13));
				word_2962D[si] = 2;
			}
		}
	}
}

static const int MAX_OPCODES2 = 27;
static const std::array<OpcodeHandlerPtr, MAX_OPCODES2> opcode2_table =
{{
	op2_unsupported, // 0x00
	op2_sub_130EF, // 0x01
	op2_unsupported, // 0x02
	op2_jumpToImmediate, // 0x03
	op2_unsupported, // 0x04
	op2_unsupported, // 0x05
	op2_unsupported, // 0x06
	op2_unsupported, // 0x07
	op2_sub_131A4, // 0x08
	op2_unsupported, // 0x09
	op2_sub_1323D, // 0x0A
	op2_unsupported, // 0x0B
	op2_unsupported, // 0x0C
	op2_sub_1345E, // 0x0D
	op2_stop, // 0x0E
	op2_unsupported, // 0x0F
	op2_unsupported, // 0x10
	op2_unsupported, // 0x11
	op2_unsupported, // 0x12
	op2_sub_1341F, // 0x13
	op2_unsupported, // 0x14
	op2_sub_132DF, // 0x15
	op2_unsupported, // 0x16
	op2_unsupported, // 0x17
	op2_sub_1339F, // 0x18
	op2_sub_133DE, // 0x19
	op2_unsupported, // 0x1A
}};

// addr seg00:3084
static void sub_13084(VMState& vm)
{
	vm.run = true;

	if (word_28558 == 0 || --word_28558 == 0)
	{
		do
		{
			uint8_t next_instruction = vm.readImm8();
			std::fprintf(trace_file, "2| %04x: %02x\n", vm.ip - 1, next_instruction);
			std::fflush(trace_file);
			opcode2_table[next_instruction](vm);
		}
		while (vm.run);
	}
}

// addr seg00:303A
static void sub_1303A(VMState& vm)
{
	vm.ip = word_29EED[word_28522];
	if (vm.ip != 0xFFFF)
	{
		word_28558 = word_29F15[word_28522];
		word_2855A = word_29F3D[word_28522];
		word_2855C = word_29F65[word_28522];
		word_28560 = word_29F8D[word_28522];
		word_2886C = 0;

		sub_13084(vm);

		word_29EED[word_28522] = vm.ip;
		word_29F15[word_28522] = word_28558;
		word_29F3D[word_28522] = word_2855A;
	}
}
