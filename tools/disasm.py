from lvtools.util import word, take_n

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

class Op(object):
    FLOW_NEXT = 0
    FLOW_NORETURN = 1

    def __init__(self, mnemonic, operands, flow=FLOW_NEXT, desc=None):
        self.mnemonic = mnemonic
        self.operands = operands
        self.flow = flow
        self.desc = desc
        self.instr_len = 1 + calc_operands_len(operands)
        self.is_jmp = '$' in operands or flow == self.FLOW_NORETURN

instruction_table = {
    'suffix': '',
    0x00: Op('YIELD', '', desc="save IP and yield"),
    0x01: Op('NOP', ''),
    0x02: Op('AUDIO.SFX', '#w', desc="Play sound effect"),
    0x03: Op('JMP', '$w', flow=Op.FLOW_NORETURN, desc="unconditional JuMP { goto op0; }"),
    0x05: Op('CALL', '$w', desc="Save next IP to link reg and jump"),
    0x06: Op('RET', '', flow=Op.FLOW_NORETURN, desc="Return to IP saved on link register"),
    0x0F: Op('FINISH.LEVEL', '', flow=Op.FLOW_NORETURN, desc="yield & finish level"),
    0x19: Op('SUBVM.IP', '#w', desc="Sets sub-vm instruction pointer"),
    0x1A: Op('J?.OBJ.UNK1A', '#b$w'),
    0x2F: Op('SUBVM.RUN', '', desc="Invoke VM2"),
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

def decode(table, rom, ip, ram_symbols):
    op = table.get(rom[ip])
    if op is None:
        return None, '%02x' % (rom[ip],), []

    original_ip = ip
    ip += 1

    next_ips = []
    opr_text = []
    for opr in take_n(op.operands, 2):
        optype, opsize = opr
        if opsize == 'b': # Byte
            value = rom[ip]
            value_w = 1
        elif opsize == 'w': # Word
            value = word.unpack_from(rom, ip)[0]
            value_w = 2
        else:
            raise ValueError("Invalid opsize %s" % (opsize,))
        ip += value_w

        if optype == '#': # Immediate
            opr_text.append('#%X' % (value,))
        elif optype == '*': # Memory pointer
            opr_text.append('($%s)' % (try_get_symbol(ram_symbols, value),))
        elif optype == '$': # Absolute label
            opr_text.append('$%X' % (value,))
            next_ips.append(value)
        else:
            raise ValueError("Invalid optype %s" % (optype,))

    assert (ip - original_ip) == op.instr_len

    if op.flow != Op.FLOW_NORETURN:
        next_ips.append(ip)

    return op, opr_text, next_ips

def disasm(table, rom, entry_point, ram_symbols):
    branch_list = [entry_point]
    program_lines = {}

    while branch_list:
        ip = branch_list.pop()
        if ip in program_lines:
            continue

        op_info, opr_text, next_ips = decode(table, rom, ip, ram_symbols)
        branch_list += next_ips
        program_lines[ip] = (ip, op_info, opr_text, table['suffix'])

    return program_lines.values()

def format_disasm(disasm_list):
    for ip, op, opr_text, vm_type in disasm_list:
        if op:
            text = '%04X%s: %s %s' % (ip, vm_type, op.mnemonic, ', '.join(opr_text))
            if op.desc:
                text = text.ljust(39) + ' ; ' + op.desc
            yield text
            if op.flow == Op.FLOW_NORETURN:
                yield ';---------'
                yield ''
            elif op.is_jmp:
                yield ''
        else:
            yield '%04X%s: !!! UNSUPPORTED OPCODE: %s' % (ip, vm_type, opr_text)
            yield ';---------'
            yield ''

argparser = argparse.ArgumentParser(description="diassemble The Lost Vikings PC script binaries")
argparser.add_argument('binary', help="path to binary to diassemble")
argparser.add_argument('entrypoint', help="starting address for the disassembly")
argparser.add_argument('-m', '--map', help="symbol map of vikings.exe. Used to show symbols instead of memory addresses")

def main():
    args = argparser.parse_args()
    entry_point = int(args.entrypoint, 0)

    rom = array('B')
    ram_symbols = {}
    with open(args.binary, 'rb') as f:
        rom.fromfile(f, os.path.getsize(args.binary))
    if args.map:
        with open(args.map, 'rU') as f:
            ram_symbols = parse_map(f, 0x184E)

    print '; %s - %04X' % (args.binary, entry_point)
    for line in format_disasm(sorted(disasm(instruction_table, rom, entry_point, ram_symbols))):
        print line

if __name__ == '__main__':
    main()
