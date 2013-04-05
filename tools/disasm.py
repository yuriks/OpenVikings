import sys
from array import array
from struct import Struct
import itertools
import os

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
    def __init__(self, mnemonic, operands, halts=False, desc=None):
        self.mnemonic = mnemonic
        self.operands = operands
        self.halts = halts
        self.desc = desc
        self.instr_len = 1 + calc_operands_len(operands)
        self.is_jmp = '$' in operands

instruction_table = {
    0x00: Op('YIELD', '', halts=True, desc="Save IP and yield"),
    0x01: Op('NOP', '', desc="No-op"),
    0x03: Op('JMP', '$w', halts=True, desc="Unconditional jump"),
    0x0f: Op('FINISH.LEVEL', '', halts=True, desc="Yield & finish level"),
    0x51: Op('LDA', '#w', desc="Load A"),
    0x73: Op('JE.UNK73', '#b$w', desc="Jump if <unknown> == op0"),
    0x97: Op('TBS', '#b#w', desc="Test bit op0/2 in op1 and set A=1"),
    0x99: Op('TBS', '#b*w', desc="Test bit op0/2 in (op1) and set A=1"),
    0xA8: Op('TBJE', '#b#w$w', desc="Test bit op0/2 in op1 and jump if == A"),
    0xAA: Op('TBJ', '#b*w$w', desc="Test bit op0/2 in (op1) and jump"),
}

def decode(rom, ip):
    op = instruction_table[rom[ip]]

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
            opr_text.append('($%X)' % (value,))
        elif optype == '$': # Absolute label
            opr_text.append('$%X' % (value,))
            next_ips.append(value)
        else:
            raise ValueError("Invalid optype %s" % (optype,))

    if not op.halts:
        next_ips.append(ip)

    assert (ip - original_ip) == op.instr_len

    return op, opr_text, next_ips

def disasm(rom, entry_point):
    branch_list = [entry_point]
    program_lines = {}

    while branch_list:
        ip = branch_list.pop()
        if ip in program_lines:
            continue

        opcode = rom[ip]
        try:
            op_info, opr_text, next_ips = decode(rom, ip)
            branch_list += next_ips
        except KeyError:
            op_info = None
            opr_text = '%02x' % (opcode,)
        program_lines[ip] = (ip, op_info, opr_text)

    return sorted(program_lines.values())

def format_disasm(disasm_list):
    next_ip = disasm_list[0][0]

    for ip, op, opr_text in disasm_list:
        if ip != next_ip:
            yield ';---------'
            yield ''
        if op:
            text = '%04X: %s %s' % (ip, op.mnemonic, ', '.join(opr_text))
            if op.desc:
                text = text.ljust(31) + ' ; ' + op.desc
            yield text
            if op.is_jmp:
                yield ''
            next_ip = ip + op.instr_len
        else:
            yield '%04X: !!! UNSUPPORTED OPCODE: %s' % (ip, opr_text)

def main():
    filename = sys.argv[1]
    entry_point = int(sys.argv[2], 0)

    rom = array('B')
    with open(filename, 'rb') as f:
        rom.fromfile(f, os.path.getsize(filename))

    for line in format_disasm(disasm(rom, entry_point)):
        print line

if __name__ == '__main__':
    main()
