from lvtools.util import word, take_n
import lvtools.world

import sys
from array import array
import os
import argparse
from copy import copy

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

def hex_leading_zero(val):
    s = '{:X}'.format(val)
    if not s[0].isdigit():
        s = '0' + s
    return s

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
    def __init__(self, value, const_map=None):
        self.value = value
        self.symbol = const_map and const_map.get(self.value)

    def __str__(self):
        if self.symbol:
            return '#{}'.format(self.symbol)
        else:
            return '#{}'.format(hex_leading_zero(self.value))

class OperandDataPointer(Operand):
    def __init__(self, address, ram_symbols=None):
        self.address = address
        self.symbol = ram_symbols and ram_symbols.get(self.address)

    def __str__(self):
        if self.symbol:
            return '(${})'.format(self.symbol)
        else:
            return '(${})'.format(hex_leading_zero(self.address))

class OperandCodePointer(Operand):
    def __init__(self, address):
        self.address = address

    def __str__(self):
        return '${}'.format(hex_leading_zero(self.address))

def unsupported_mode(x):
    def f(rom, ip, ram_symbols):
        raise ValueError("Unsupported addressing mode %s" % (x,))
    return f

class Op(object):
    FLOW_NEXT = 0
    FLOW_NORETURN = 1
    FLOW_CALL = 2

    addressing_modes = {
        0: lambda rom, ip, ram_symbols: (OperandImmediate(word.unpack_from(rom, ip)[0]), 2),
        1: unsupported_mode(1),
        2: lambda rom, ip, ram_symbols: (OperandDataPointer(word.unpack_from(rom, ip)[0], ram_symbols), 2),
        3: unsupported_mode(3),
        4: unsupported_mode(4),
        5: unsupported_mode(5),
        6: unsupported_mode(6),
        7: unsupported_mode(7),
    }

    def __init__(self, mnemonic, operands, flow=FLOW_NEXT, desc=None, jump_target_vm=None, operand_tags={}):
        self.mnemonic = mnemonic
        self.operands = operands
        self.flow = flow
        self.desc = desc
        self.is_jmp = ('$' in operands and jump_target_vm is None and flow != self.FLOW_CALL) or flow == self.FLOW_NORETURN
        self.jump_target_vm = jump_target_vm
        self.operand_tags = operand_tags

    def decode_operand(self, opr, operand_tag, state, rom, ip, ram_symbols):
        optype, opsize = opr
        operands = []
        oper_width = 0
        next_states = []

        if optype in '#*$':
            if opsize == 'b': # Byte
                value = rom[ip]
                oper_width += 1
            elif opsize == 'w': # Word
                value = word.unpack_from(rom, ip)[0]
                oper_width += 2
            else:
                raise ValueError("Invalid opsize %s" % (opsize,))

            if optype == '#': # Immediate
                operands.append(OperandImmediate(value, operand_tag))
            elif optype == '*': # Memory pointer
                operands.append(OperandDataPointer(value, ram_symbols))
            elif optype == '$': # Absolute label
                operands.append(OperandCodePointer(value))
                next_states.append(state.jumped(value, self.jump_target_vm))
        elif optype == 'A':
            addressing_mode = rom[ip]
            oper_width = 1
            if opsize in '12':
                new_operand, w = self.addressing_modes[addressing_mode % 8](rom, ip+oper_width, ram_symbols)
                operands.append(new_operand)
                oper_width += w
                if opsize == '2':
                    new_operand, w = self.addressing_modes[(addressing_mode >> 3) % 8](rom, ip+oper_width, ram_symbols)
                    operands.append(new_operand)
                    oper_width += w
            else:
                raise ValueError("Invalid opsize %s" % (opsize,))
        else:
            raise ValueError("Invalid optype %s" % (optype,))

        return operands, oper_width, next_states

    def decode(self, state, rom, obj, ram_symbols):
        ip = state.ip + 1

        next_states = []
        operands = []
        for i, opr in enumerate(take_n(self.operands, 2)):
            tag = self.operand_tags and self.operand_tags[i]
            new_operands, operand_size, added_next_states = self.decode_operand(opr, tag, state, rom, ip, ram_symbols)
            ip += operand_size
            operands += new_operands
            next_states += added_next_states

        if self.flow != self.FLOW_NORETURN:
            next_states.append(state.jumped(ip))

        return self, operands, next_states


class OpSprites(Op):
    def __init__(self, mnemonic, operands, flow=Op.FLOW_NEXT, desc=None, jump_target_vm=None, filtered=False):
        # super hard TODO: handle filtered
        super(OpSprites, self).__init__(mnemonic, operands, flow, desc, jump_target_vm)

    def decode(self, state, rom, obj, ram_symbols):
        ip = state.ip + 1

        next_states = []
        operands = []
        for sprite_i in xrange(obj.num_sprites):
            for opr in take_n(self.operands, 2):
                tag = self.operand_tags and self.operand_tags[i]
                new_operands, operand_size, added_next_states = self.decode_operand(opr, tag, state, rom, ip, ram_symbols)
                ip += operand_size
                operands += new_operands
                next_states += added_next_states

        assert (ip - state.ip) == 1 + obj.num_sprites * calc_operands_len(self.operands)

        if self.flow != self.FLOW_NORETURN:
            next_states.append(state.jumped(ip))

        return self, operands, next_states


class OpExtended():
    def __init__(self, extended_codes):
        self.extended_codes = extended_codes

    def decode(self, state, rom, obj, ram_symbols):
        state = state.jumped(state.ip + 1)
        extended_byte = rom[state.ip]

        if extended_byte in self.extended_codes:
            op = self.extended_codes[extended_byte]
        else:
            op = self.extended_codes[None]

        return op.decode(state, rom, obj, ram_symbols)

property_symbolic_constants = {
        0x08: 'PROP_FLAGS',
        0x16: 'PROP_USERDATA',
        0x1C: 'PROP_TIMER',
        0x1E: 'PROP_XPOS',
        0x20: 'PROP_YPOS',
        0x2C: 'PROP_GFX_PTR',
        0x2E: 'PROP_USER1',
        0x30: 'PROP_USER2',
        0x32: 'PROP_USER3',
        0x34: 'PROP_USER4',
        0x38: 'PROP_XSPEED',
        0x3A: 'PROP_YSPEED',
        }

instruction_table = {
    'suffix': '',
    0x00: Op('YIELD', '', desc="save IP and yield"),
    0x01: Op('NOP.01', ''), # TODO
    0x02: Op('AUDIO.SFX', '#w', desc="Play sound effect"),
    0x03: Op('JMP', '$w', flow=Op.FLOW_NORETURN, desc="unconditional JuMP { goto op0; }"),

    0x05: Op('CALL', '$w', flow=Op.FLOW_CALL, desc="Save next IP to link reg and jump"),
    0x06: Op('RET', '', flow=Op.FLOW_NORETURN, desc="Return to IP saved on link register"),

    0x0F: Op('FINISH.LEVEL', '', flow=Op.FLOW_NORETURN, desc="yield & finish level"),

    0x13: OpExtended({
            0x01: Op('SPECIAL.QUIT', '#w', flow=Op.FLOW_NORETURN, desc="Quit game"),
            0x11: Op('SPECIAL.COPYSTATUS', '#w', desc="Copy statusbar area to middle of screen"),
            0xD9: Op('SPECIAL.LOADPAL', '#w', desc="Load palette from ROM address op1"),
            None: Op('SPECIAL.NOP', '#w')
        }),

    0x19: Op('SPRVM.IP', '$w', desc="Sets sub-vm instruction pointer", jump_target_vm='sprvm'),

    0x1A: Op('J?.OBJ.UNK1A', '#b$w'),

    0x1D: Op('UNK.1D', '#w$w', flow=Op.FLOW_CALL, desc="Test ??? and call (probably related to collisions)"),

    0x2F: Op('SPRVM.RUN', '', desc="Invoke animation VM"),

    0x3D: Op('PAL1.FADE', '#b#b#b', desc="Set palette fade amount. (color1)"),
    0x3E: Op('PAL1.RESTORE', '', desc="Remove palette fade. (color1)"),

    0x3F: Op('SPR.HIDE2', '', desc="Hides all the object's sprites using the HIDDEN2 flag."),
    0x40: Op('SPR.SHOW', '', desc="Unhide all the object's sprites."),
    0x41: Op('DIALOG.DIALOG', 'A2A2', desc="Appends a dialog display command"),
    0x42: Op('DIALOG.CLEAR', '', desc="Appends a clear command"),

    0x45: Op('DIALOG.STRING', 'A1A2', desc="Appends a putString command"),
    0x46: Op('DIALOG.COLOR', '#w', desc="Appends a dialog color change command"),

    0x4C: Op('PAL2.FADE', '#b#b#b', desc="Set palette fade amount. (color2)"),

    0x50: Op('DIALOG.CHAR', 'A2A1', desc="Appends a putChar command"),
    0x51: Op('LDA', '#w', desc="LoaD A { A = op0; }"),
    0x52: Op('OBJPROP.LDA', '#b', operand_tags=(property_symbolic_constants,), desc="LoaD A from current object op0/2"),
    0x53: Op('LDA', '*w', desc="LoaD A from memory"),

    0x56: Op('OBJPROP.STA', '#b', operand_tags=(property_symbolic_constants,), desc="STore A to current object op0/2"),
    0x57: Op('STA', '*w', desc="STore A to memory"),
    0x58: Op('OTHERPROP.STA', '#b', operand_tags=(property_symbolic_constants,), desc="Stores A to access target prop op0/2"),
    0x59: Op('OBJPROP.ADD', '#b', operand_tags=(property_symbolic_constants,), desc="ADD A from current object property op0/2"),
    0x5A: Op('ADD', '*w', desc="ADD A to memory { *op0 += A; }"),

    0x5C: Op('OBJPROP.SUB', '#b', operand_tags=(property_symbolic_constants,), desc="SUBtract A from current object property op0/2"),

    0x5F: Op('OBJPROP.ANDA', '#b', operand_tags=(property_symbolic_constants,), desc="AND current object op0/2 with A"),
    0x60: Op('ANDA', '*w', desc="AND memory with A { op0 &= A; }"),

    0x62: Op('OBJPROP.ORA', '#b', operand_tags=(property_symbolic_constants,), desc="OR current object property op0/2 with A"),

    0x68: Op('JB', '#w$w', desc="Jump if Below { if (op0 < A) goto op1; }"),

    0x72: Op('JE', '#w$w', desc="Jump if Equal { if (op0 == A) goto op1; }"),
    0x73: Op('OBJPROP.JE', '#b$w', operand_tags=(property_symbolic_constants, None), desc="jump if current object property op0/2 == A"),
    0x74: Op('JE', '*w$w', desc="Jump if Equal { if (*op0 == A) goto op1; }"),

    0x77: Op('JNE', '#w$w', desc="Jump if Not Equal { if (op0 != A) goto op1; }"),
    0x78: Op('OBJPROP.JNE', '#b$w', operand_tags=(property_symbolic_constants, None), desc="jump if current object property op0/2 != A"),
    0x79: Op('JNE', '*w$w', desc="Jump if Not Equal { if (*op0 != A) goto op1; }"),

    0x7C: Op('JL', '#w$w', desc="Jump if Less (signed) { if (op0 < A) goto op1; }"),

    0x81: Op('JGE', '#w$w', desc="Jump if Greater or Equal (signed) { if (op0 >= A) goto op1; }"),

    0x96: Op('OBJ.OTHER.STA', '', desc="Store A to current object property access target"),
    0x97: Op('TBS', '#b#w', desc="Test Bit & Set { A = bool((1 << op0/2) & op1) }"),

    0x99: Op('TBS', '#b*w', desc="Test Bit & Set { A = bool((1 << op0/2) & op1) }"),

    0x9D: Op('AB', '#b*w', desc="Assign Bit { if (A != 0) { A = (1 << op0/2); } *op1 = *op1 & ~(1 << op0/2) | A; }"),

    0xA8: Op('TBJE', '#b#w$w', desc="Test Bit & Jump if Equal { if (bool((1 << op0/2) & op1) == A) goto op3; }"),

    0xAA: Op('TBJ', '#b*w$w', desc="Test Bit & Jump { if ((1 << op0/2) & op1) goto op3; }"),

    0xCF: Op('UNKCF', '$w'),

    0xD3: Op('CHECK.PASSWORD', '', desc="Executes a password check and sets global variables acoordingly."),
}

sprvm_instruction_table = {
    'suffix': '-sprvm',
    0x00: Op('SPR.ADVANCE', '#b', desc="Advance animation for sprites by op0 tiles."),
    0x01: OpSprites('SPR.TILES', '#b', filtered=True, desc="Set list of tile indices for sprites."),
    0x02: instruction_table[0x02],
    0x03: Op('JMP', '$w', flow=Op.FLOW_NORETURN, desc="Unconditional JuMP { goto op0; }"),
    0x04: Op('NOP.04', '#b'),
    0x05: Op('CALL', '$w', flow=Op.FLOW_CALL, desc="Save next IP to link reg and jump"),
    0x06: Op('RET', '', flow=Op.FLOW_NORETURN, desc="Return to IP saved on link register"),
    0x07: Op('UNK07', '#b'), # TODO
    0x08: OpSprites('SPR.XPOS', '#w', desc="Set X sprite positions relative to object."),
    0x09: Op('UNK09', '#b'), # TODO
    0x0A: OpSprites('SPR.YPOS', '#w', desc="Set Y sprite positions relative to object."),
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
    0x16: Op('NOP.16', '#b'),
    0x17: Op('UNK17', '#w'), # TODO
    0x18: Op('SPR.SET.HIDDEN2', '', desc="Set hidden2 flag on sprites."),
    0x19: Op('SPR.CLR.HIDDEN12', '', desc="Clear hidden and hidden2 flags on sprites."),
    0x1A: Op('STOP', '', flow=Op.FLOW_NORETURN, desc="Clear IP and yield")
}

vm_types = {
    'vm': instruction_table,
    'sprvm': sprvm_instruction_table,
}

def decode(state, rom, obj, ram_symbols):
    """Returns: (op_info, operands, next_states)"""

    table = vm_types[state.vm_type]
    op = table.get(rom[state.ip])
    if op is None:
        return None, '%02x' % (rom[state.ip],), []

    return op.decode(state, rom, obj, ram_symbols)

def disasm(rom, entrypoints, obj, ram_symbols):
    branch_list = entrypoints[:]
    program_lines = {}

    while branch_list:
        state = branch_list.pop()
        if state in program_lines:
            continue

        op_info, operands, next_states = decode(state, rom, obj, ram_symbols)
        branch_list += next_states

        program_lines[state] = DisassembledInstruction(state, op_info, operands)

    return program_lines.values()

class VMState(object):
    def __init__(self, vm_type, ip):
        self.vm_type = vm_type
        self.ip = ip

    def key(self):
        return (self.ip, self.vm_type)

    def __hash__(self):
        return hash(self.key())

    def __eq__(self, other):
        return self.key() == other.key()

    def __str__(self):
        vm_type_suffix = vm_types[self.vm_type]['suffix']
        return '{0:04X}{1}'.format(self.ip, vm_type_suffix)

    def __repr__(self):
        return 'VMState(vm_type={0}, ip={1})'.format(self.vm_type, self.ip)

    def jumped(self, ip, vm=None):
        n = copy(self)
        n.ip = ip
        n.vm_type = vm or self.vm_type
        return n

class DisassembledInstruction(object):
    def __init__(self, state, op, operands):
        self.state = state
        self.op = op
        self.operands = operands

    def __repr__(self):
        return 'DisassembledInstruction(state={0}, op={1}, operands={2})'.format(self.state, self.op, self.operands)

def format_disasm(disasm_list):
    for instruction in disasm_list:
        address = str(instruction.state)
        op = instruction.op
        operands = instruction.operands

        if op:
            text = '%s: %s %s' % (address, op.mnemonic, ', '.join(map(str, operands)))
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
            yield '%s: !!! UNSUPPORTED OPCODE: %s' % (address, operands)
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

    entry_point = VMState(args.type, args.entrypoint)
    disassembled_instructions = disasm(rom, [entry_point], obj, ram_symbols)
    for line in format_disasm(sorted(disassembled_instructions, key=lambda x: x.state.key())):
        print line

if __name__ == '__main__':
    main()
