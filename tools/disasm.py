from lvtools.util import word, take_n
import lvtools.world

import sys
from array import array
import os
import argparse

def parse_map(f, seg):
    symbols = {}
    for l in f:
        l = l.strip()
        sl = l.split()
        if len(sl) != 2:
            continue
        lseg, loff = sl[0].split(':')
        if int(lseg, 16) == seg:
            symbols[int(loff, 16)] = sl[1]

    return symbols

def try_get_symbol(symbol_map, addr):
    if addr in symbol_map:
        return symbol_map[addr]
    else:
        return '%X' % (addr,)

opr_lens = {
    'b': 1,
    'w': 2
}

def calc_operands_len(operands):
    opr_len = 0

    for opr in take_n(operands, 2):
        optype, opsize = opr
        opr_len += opr_lens[opsize]

    return opr_len

class Operand(object):
    pass

class OperandImmediate(Operand):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return '#{:X}'.format(self.value)

class OperandDataPointer(Operand):
    def __init__(self, address, ram_symbols=None):
        self.address = address
        self.symbol = ram_symbols and ram_symbols.get(self.address)

    def __str__(self):
        if self.symbol:
            return '(${})'.format(self.symbol)
        else:
            return '(${:X})'.format(self.address)

class OperandCodePointer(Operand):
    def __init__(self, address):
        self.address = address

    def __str__(self):
        return '${:X}'.format(self.address)

class Op(object):
    FLOW_NEXT = 0
    FLOW_NORETURN = 1

    def __init__(self, mnemonic, operands, flow=FLOW_NEXT, desc=None, jump_target_table=None):
        self.mnemonic = mnemonic
        self.operands = operands
        self.flow = flow
        self.desc = desc
        self.instr_len = 1 + calc_operands_len(operands)
        self.is_jmp = ('$' in operands and jump_target_table is None) or flow == self.FLOW_NORETURN
        self.jump_target_table = jump_target_table

    def _decode_operand(self, opr, rom, ip, next_ips, ram_symbols):
        optype, opsize = opr
        if opsize == 'b': # Byte
            value = rom[ip]
            value_w = 1
        elif opsize == 'w': # Word
            value = word.unpack_from(rom, ip)[0]
            value_w = 2
        else:
            raise ValueError("Invalid opsize %s" % (opsize,))

        if optype == '#': # Immediate
            operand = OperandImmediate(value)
        elif optype == '*': # Memory pointer
            operand = OperandDataPointer(value, ram_symbols)
        elif optype == '$': # Absolute label
            operand = OperandCodePointer(value)
            next_ips.append((self.jump_target_table, value))
        else:
            raise ValueError("Invalid optype %s" % (optype,))

        return operand, value_w

    def decode(self, rom, ip, obj, ram_symbols):
        original_ip = ip
        ip += 1

        next_ips = []
        operands = []
        for opr in take_n(self.operands, 2):
            operand, operand_size = self._decode_operand(opr, rom, ip, next_ips, ram_symbols)
            ip += operand_size
            operands.append(operand)

        assert (ip - original_ip) == self.instr_len

        if self.flow != self.FLOW_NORETURN:
            next_ips.append((None, ip))

        return operands, next_ips


class OpSprites(Op):
    def __init__(self, mnemonic, operands, flow=Op.FLOW_NEXT, desc=None, jump_target_table=None, filtered=False):
        # super hard TODO: handle filtered
        super(OpSprites, self).__init__(mnemonic, operands, flow, desc, jump_target_table)
        self.instr_len = None

    def decode(self, rom, ip, obj, ram_symbols):
        original_ip = ip
        ip += 1

        next_ips = []
        operands = []
        for sprite_i in xrange(obj.num_sprites):
            for opr in take_n(self.operands, 2):
                operand, operand_size = self._decode_operand(opr, rom, ip, next_ips, ram_symbols)
                ip += operand_size
                operands.append(operand)

        assert (ip - original_ip) == 1 + obj.num_sprites * calc_operands_len(self.operands)

        if self.flow != self.FLOW_NORETURN:
            next_ips.append((None, ip))

        return operands, next_ips

# "Forward declare" dictionary
sprvm_instruction_table = {}

instruction_table = {
    'suffix': '',
    0x00: Op('YIELD', '', desc="save IP and yield"),
    0x01: Op('NOP', ''),
    0x02: Op('AUDIO.SFX', '#w', desc="Play sound effect"),
    0x03: Op('JMP', '$w', flow=Op.FLOW_NORETURN, desc="unconditional JuMP { goto op0; }"),
    0x05: Op('CALL', '$w', desc="Save next IP to link reg and jump"),
    0x06: Op('RET', '', flow=Op.FLOW_NORETURN, desc="Return to IP saved on link register"),
    0x0F: Op('FINISH.LEVEL', '', flow=Op.FLOW_NORETURN, desc="yield & finish level"),
    0x19: Op('SPRVM.IP', '$w', desc="Sets sub-vm instruction pointer", jump_target_table=sprvm_instruction_table),
    0x1A: Op('J?.OBJ.UNK1A', '#b$w'),
    0x2F: Op('SPRVM.RUN', '', desc="Invoke animation VM"),
    # Variable Length Instruction, needs more work to decode: 0x41: Op('DIALOG.TEXT', '#b__#b'
    0x46: Op('DIALOG.COLOR', '#w', desc="Appends a dialog color change command"),
    0x51: Op('LDA', '#w', desc="LoaD A { A = op0; }"),
    0x52: Op('OBJPROP.LDA', '#b', desc="LoaD A from current object op0/2"),
    0x53: Op('LDA', '*w', desc="LoaD A from memory"),
    0x56: Op('OBJPROP.STA', '#b', desc="STore A to current object op0/2"),
    0x57: Op('STA', '*w', desc="STore A to memory"),
    0x58: Op('OTHERPROP.STA', '#b', desc="Stores A to access target prop op0/2"),
    0x5A: Op('ADD', '*w', desc="ADD A to memory { *op0 += A; }"),
    0x5F: Op('OBJPROP.ANDA', '#b', desc="AND current object op0/2 with A"),
    0x60: Op('ANDA', '*w', desc="AND memory with A { op0 &= A; }"),
    0x62: Op('OBJPROP.ORA', '#b', desc="OR current object property op0/2 with A"),
    0x68: Op('JB', '#w$w', desc="Jump if Below { if (op0 < A) goto op1; }"),
    0x72: Op('JE', '#w$w', desc="Jump if Equal { if (op0 == A) goto op1; }"),
    0x73: Op('OBJPROP.JE', '#b$w', desc="jump if current object property op0/2 == A"),
    0x74: Op('JE', '*w$w', desc="Jump if Equal { if (*op0 == A) goto op1; }"),
    0x77: Op('JNE', '#w$w', desc="Jump if Not Equal { if (op0 != A) goto op1; }"),
    0x78: Op('OBJPROP.JNE', '#b$w', desc="jump if current object property op0/2 != A"),
    0x79: Op('JNE', '*w$w', desc="Jump if Not Equal { if (*op0 != A) goto op1; }"),
    0x96: Op('OBJ.OTHER.STA', '', desc="Store A to current object property access target"),
    0x97: Op('TBS', '#b#w', desc="Test Bit & Set { A = bool((1 << op0/2) & op1) }"),
    0x99: Op('TBS', '#b*w', desc="Test Bit & Set { A = bool((1 << op0/2) & op1) }"),
    0xA8: Op('TBJE', '#b#w$w', desc="Test Bit & Jump if Equal { if (bool((1 << op0/2) & op1) == A) goto op3; }"),
    0xAA: Op('TBJ', '#b*w$w', desc="Test Bit & Jump { if ((1 << op0/2) & op1) goto op3; }"),
    0xCF: Op('UNKCF', '$w'),
}

sprvm_instruction_table.update({
    'suffix': '-sprvm',
    0x00: Op('SPR.ADVANCE', '#b', desc="Advance animation for sprites by op0 tiles."),
    0x01: OpSprites('SPR.TILES', '#b', filtered=True, desc="Set list of tile indices for sprites."),
    0x02: instruction_table[0x02],
    0x03: Op('JMP', '$w', flow=Op.FLOW_NORETURN, desc="Unconditional JuMP { goto op0; }"),
    0x04: Op('NOP', '#b'),
    0x05: Op('CALL', '$w', desc="Save next IP to link reg and jump"),
    0x06: Op('RET', '', flow=Op.FLOW_NORETURN, desc="Return to IP saved on link register"),
    0x07: Op('UNK07', '#b'), # TODO
    0x08: OpSprites('UNK08', '#w'), # TODO
    0x09: Op('UNK09', '#b'), # TODO
    0x0A: OpSprites('UNK0A', '#w'), # TODO
    0x0B: Op('BRK', '', desc="Break into debugger."),
    0x0C: OpSprites('SPR.PALS', '#b', filtered=True, desc="Set list of palettes for sprites."),
    0x0D: Op('FILTER', '#b', desc="Set the value of the tag filter."),
    0x0E: Op('YIELD', '', desc="Save IP and yield"),
    0x0F: Op('DELAY', '#b', desc="Set counter, save IP and yield"),
    0x10: Op('SPR.XFLIP', '', desc="Toggle sprite X flip."),
    0x11: Op('SPR.YFLIP', '', desc="Toggle sprite Y flip."),
    0x12: Op('SPR.XYFLIP', '', desc="Toggle sprite X and Y flip."),
    0x13: OpSprites('SPR.TAGS', '#b', filtered=True, desc="Set list of sprite tags."),
    0x14: Op('UNK14', '#b'), # TODO
    0x15: Op('SPR.SIZE', '#b', desc="Set size of sprites. (0=8, 1=32, 2=16)"),
    0x16: Op('NOP', '#b'),
    0x17: Op('UNK17', '#w'), # TODO
    0x18: Op('SPR.SET.HIDDEN2', '', desc="Set hidden2 flag on sprites."),
    0x19: Op('SPR.CLR.HIDDEN12', '', desc="Clear hidden and hidden2 flags on sprites."),
    0x1A: Op('STOP', '', flow=Op.FLOW_NORETURN, desc="Clear IP and yield")
})

vm_types = {
    'vm': instruction_table,
    'sprvm': sprvm_instruction_table,
}

def decode(table, rom, ip, obj, ram_symbols):
    op = table.get(rom[ip])
    if op is None:
        return None, '%02x' % (rom[ip],), []

    operands, next_ips = op.decode(rom, ip, obj, ram_symbols)
    return op, operands, next_ips

def disasm(rom, entrypoints, obj, ram_symbols):
    branch_list = entrypoints[:]
    program_lines = {}

    while branch_list:
        table, ip = branch_list.pop()
        if ip in program_lines:
            continue

        op_info, operands, next_ips = decode(table, rom, ip, obj, ram_symbols)
        for t, a in next_ips:
            branch_list.append((t or table, a))

        program_lines[ip] = (ip, op_info, operands, table['suffix'])

    return program_lines.values()

def format_disasm(disasm_list):
    for ip, op, operands, vm_type in disasm_list:
        if op:
            text = '%04X%s: %s %s' % (ip, vm_type, op.mnemonic, ', '.join(map(str, operands)))
            if op.desc:
                text = text.ljust(39) + ' ; ' + op.desc
            yield text
            if op.flow == Op.FLOW_NORETURN:
                yield ';---------'
                yield ''
            elif op.is_jmp:
                yield ''
        else:
            # TODO clean up use of operands
            yield '%04X%s: !!! UNSUPPORTED OPCODE: %s' % (ip, vm_type, operands)
            yield ';---------'
            yield ''

def create_argument_parser():
    def hex_int(s):
        return int(s, 0)

    parser = argparse.ArgumentParser(description="diassemble The Lost Vikings PC script binaries")
    parser.add_argument('binary', help="path to binary to diassemble")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-e', '--entrypoint', type=hex_int, help="starting address for the disassembly")
    group.add_argument('-o', '--object', type=hex_int, help="object of which script to disassemble (recommended)")
    parser.add_argument('-m', '--map', help="symbol map of vikings.exe. Used to show symbols instead of memory addresses")
    parser.add_argument('-t', '--type', choices=('vm', 'sprvm'), default='vm', help="type of program to diassemble")
    return parser

def main():
    args = create_argument_parser().parse_args()

    rom = array('B')
    ram_symbols = {}
    with open(args.binary, 'rb') as f:
        rom.fromfile(f, os.path.getsize(args.binary))
    if args.map:
        with open(args.map, 'rU') as f:
            ram_symbols = parse_map(f, 0x184E)

    obj = None
    obj_string = ''
    if args.object:
        obj = lvtools.world.parse_object(rom, args.object)
    if obj:
        args.entrypoint = obj.script_entry_point + 3
        obj_string = 'object {:X}h - '.format(args.object)

    print('; {} - {}entrypoint {:04X}h {}'.format(args.binary, obj_string, args.entrypoint, args.type))
    for line in format_disasm(sorted(disasm(rom, [(vm_types[args.type], args.entrypoint)], obj, ram_symbols))):
        print line

if __name__ == '__main__':
    main()
