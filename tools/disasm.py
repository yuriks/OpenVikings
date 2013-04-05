import sys
from array import array
from struct import Struct
import itertools
import os

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

# From http://stackoverflow.com/a/312464/42029
def chunk_seq(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

word = Struct('<H')

opr_lens = {
    'b': 1,
    'w': 2
}

def calc_operands_len(operands):
    opr_len = 0

    for opr in chunk_seq(operands, 2):
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
        self.is_jmp = '$' in operands

instruction_table = {
    'suffix': '',
    0x00: Op('YIELD', '', desc="save IP and yield"),
    0x01: Op('NOP', ''),
    0x03: Op('JMP', '$w', flow=Op.FLOW_NORETURN, desc="unconditional JuMP { goto op0; }"),
    0x0F: Op('FINISH.LEVEL', '', flow=Op.FLOW_NORETURN, desc="yield & finish level"),
    0x19: Op('OBJ.UNK19', '#w'),
    0x2F: Op('SUBVM2', '', desc="Invokes VM2"),
    0x51: Op('LDA', '#w', desc="LoaD A { A = op0; }"),
    0x73: Op('OBJPROP.JE', '#b$w', desc="jump if obj. property op0/2 for current object == A"),
    0x97: Op('TBS', '#b#w', desc="Test Bit & Set { A = bool((1 << op0/2) & op1) }"),
    0x99: Op('TBS', '#b*w', desc="Test Bit & Set { A = bool((1 << op0/2) & op1) }"),
    0xA8: Op('TBJE', '#b#w$w', desc="Test Bit & Jump if Equal { if (bool((1 << op0/2) & op1) == A) goto op3; }"),
    0xAA: Op('TBJ', '#b*w$w', desc="Test Bit & Jump { if ((1 << op0/2) & op1) goto op3; }"),
}

def decode(table, rom, ip, ram_symbols):
    op = table.get(rom[ip])
    if op is None:
        return None, '%02x' % (rom[ip],), []

    original_ip = ip
    ip += 1

    next_ips = []
    opr_text = []
    for opr in chunk_seq(op.operands, 2):
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
        elif optype == '*': # Indirect
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
    next_ip = disasm_list[0][0]

    for ip, op, opr_text, vm_type in disasm_list:
        if ip != next_ip:
            yield ';---------'
            yield ''
        if op:
            text = '%04X%s: %s %s' % (ip, vm_type, op.mnemonic, ', '.join(opr_text))
            if op.desc:
                text = text.ljust(39) + ' ; ' + op.desc
            yield text
            if op.is_jmp:
                yield ''
            next_ip = ip + op.instr_len
        else:
            yield '%04X%s: !!! UNSUPPORTED OPCODE: %s' % (ip, vm_type, opr_text)

def main():
    filename = sys.argv[1]
    entry_point = int(sys.argv[2], 0)
    if len(sys.argv) > 3:
        map_filename = sys.argv[3]
    else:
        map_filename = None

    rom = array('B')
    ram_symbols = {}
    with open(filename, 'rb') as f:
        rom.fromfile(f, os.path.getsize(filename))
    if map_filename:
        with open(map_filename, 'rU') as f:
            ram_symbols = parse_map(f, 0x184E)

    for line in format_disasm(sorted(disasm(instruction_table, rom, entry_point, ram_symbols))):
        print line

if __name__ == '__main__':
    main()
