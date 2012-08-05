#include "vikings.hpp"
#include "vm.hpp"

#include "vars.hpp"
#include "main.hpp"

// seg04:008A
static uint16_t vm_regA;

struct VMState
{
	bool run;
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
	obj_script_resume[word_28522] = vm.ip;
	vm.run = false;
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
	uint8_t a = vm.readImm8();
	uint16_t b = vm.readImm16();
	vm_regA = vm_checkBit(a, b);
}

static void op_testBitJumpEq(VMState& vm)
{
	uint8_t a = vm.readImm8();
	uint16_t b = vm.readImm16();
	if (vm_checkBit(a, b) == vm_regA)
		op_jumpToImmediate(vm);
}


static const int MAX_OPCODES = 216;
static const OpcodeHandlerPtr opcode_table[MAX_OPCODES] =
{
	op_stopScript, // 0x00
	op_nop, // 0x01
	op_unsupported, // sub_177B2,      // 0x02
	op_jumpToImmediate, // 0x03
	op_unsupported, // sub_1782A,      // 0x04
	op_unsupported, // sub_142C1,      // 0x05
	op_unsupported, // sub_142D3,      // 0x06
	op_unsupported, // sub_1368C,      // 0x07
	op_unsupported, // sub_1367C,      // 0x08
	op_unsupported, // sub_13743,      // 0x09
	op_unsupported, // sub_13733,      // 0x0A
	op_unsupported, // sub_1369C,      // 0x0B
	op_unsupported, // sub_13753,      // 0x0C
	op_unsupported, // sub_142DC,      // 0x0D
	op_unsupported, // sub_142FC,      // 0x0E
	op_unsupported, // sub_1431C,      // 0x0F
	op_unsupported, // sub_14327,      // 0x10
	op_unsupported, // sub_14334,      // 0x11
	op_unsupported, // sub_14340,      // 0x12
	op_unsupported, // sub_1434C,      // 0x13
	op_unsupported, // sub_14F59,      // 0x14
	op_unsupported, // sub_150FC,      // 0x15
	op_unsupported, // sub_15106,      // 0x16
	op_unsupported, // sub_143F2,      // 0x17
	op_unsupported, // sub_14409,      // 0x18
	op_unsupported, // sub_14428,      // 0x19
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
	op_unsupported, // sub_13031,      // 0x2F
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
	op_unsupported, // sub_14532,      // 0x3D
	op_unsupported, // sub_14590,      // 0x3E
	op_unsupported, // sub_145E5,      // 0x3F
	op_unsupported, // sub_14604,      // 0x40
	op_unsupported, // sub_1242E,      // 0x41
	op_unsupported, // sub_12669,      // 0x42
	op_unsupported, // sub_1267B,      // 0x43
	op_unsupported, // sub_1246D,      // 0x44
	op_unsupported, // sub_12634,      // 0x45
	op_unsupported, // loc_1268D,      // 0x46
	op_unsupported, // locret_141F6,   // 0x47
	op_unsupported, // loc_14FEC,      // 0x48
	op_unsupported, // loc_14FC4,      // 0x49
	op_unsupported, // loc_14FC8,      // 0x4A
	op_unsupported, // loc_16252,      // 0x4B
	op_unsupported, // loc_14561,      // 0x4C
	op_unsupported, // loc_145B5,      // 0x4D
	op_unsupported, // loc_144FD,      // 0x4E
	op_unsupported, // loc_14501,      // 0x4F
	op_unsupported, // loc_126A9,      // 0x50
	op_loadImmediateWord, // 0x51
	op_unsupported, // sub_1462E,      // 0x52
	op_unsupported, // sub_14646,      // 0x53
	op_unsupported, // sub_14652,      // 0x54
	op_unsupported, // sub_1466E,      // 0x55
	op_unsupported, // loc_14686,      // 0x56
	op_unsupported, // loc_146A3,      // 0x57
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
	op_unsupported, // loc_147E7,      // 0x62
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
	op_unsupported, // op_loc_14A1B,   // 0x73
	op_unsupported, // loc_14A2B,      // 0x74
	op_unsupported, // loc_14A3B,      // 0x75
	op_unsupported, // loc_14A4B,      // 0x76
	op_unsupported, // loc_14A5B,      // 0x77
	op_unsupported, // loc_14A6B,      // 0x78
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
	op_unsupported, // op_sub_14B59,   // 0x99
	op_unsupported, // loc_14B60,      // 0x9A
	op_unsupported, // loc_14B67,      // 0x9B
	op_unsupported, // loc_14B71,      // 0x9C
	op_unsupported, // loc_14BA7,      // 0x9D
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
	op_unsupported, // op_sub_14DB1,   // 0xAA
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
};

// addr seg00:424C
void runObjectScript(int object_slot)
{
	if (word_29835[object_slot] == nullptr)
		return;

	if ((word_29A65[object_slot] & BIT(12)) && word_29BF5[object_slot] != 0)
	{
		word_29BF5[object_slot] -= 1;
	}

	word_28522 = object_slot;
	word_2886E = 0;
	uint8_t* es = word_29835[object_slot];

	if ((word_29A65[object_slot] & BIT(9)) || word_2880F != 0)
	{
		if (word_29BCD[object_slot] & BIT(15))
			return;

		int bx = word_29BCD[object_slot];
		WorldData* world_data_entry = reinterpret_cast<WorldData*>(world_data + bx*21);

		obj_script_resume[object_slot] = world_data_entry->field_3;
	}

	VMState vm;
	vm.run = true;
	vm.rom = world_data;
	vm.ip = obj_script_resume[object_slot];

	do
	{
		uint8_t next_instruction = vm.readImm8();
		opcode_table[next_instruction](vm);
	}
	while(vm.run);
}
