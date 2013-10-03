#!/usr/bin/env python
# encoding: utf-8
import sys
import re
import bisect

code_address_re = re.compile(r'(?<!\()\$([0-9][0-9a-fA-F]*)(?!\))')
line_address_re = re.compile(r'^([0-9a-fA-f]+)')

def strip_comment(line, comment_char=';'):
    return line.partition(comment_char)[0].strip()

def gather_jumps(lines):
    addr_to_line = {}
    jumps = []
    for i, line in enumerate(lines):
        l = strip_comment(line)
        addr, sep, instruction = l.partition(':')
        addr_match = line_address_re.match(addr)
        if addr_match:
            cur_addr = int(addr_match.group(1), 16)
            addr_to_line[cur_addr] = i
            if "SPRVM.IP" not in instruction:
                for jump_match in code_address_re.finditer(instruction):
                    target_addr = int(jump_match.group(1), 16)
                    jumps.append((cur_addr, target_addr))

    return addr_to_line, jumps

class Jumps(object):
    def __init__(self, destination, sources):
        self.destination = destination
        self.sources = sources
        self.lower_bound = min(destination, *sources)
        self.upper_bound = max(destination, *sources)

    def __repr__(self):
        return 'Jumps({} <- {})'.format(self.destination,
                ', '.join(str(x) for x in self.sources))

    def range(self):
        return (self.lower_bound, self.upper_bound)

    def length(self):
        return self.upper_bound - self.lower_bound + 1

def combine_jumps(jumps):
    """Receives an iterable of (source, destination) jumps and combines them by
    destination, returning a [Jumps]."""

    combined_jumps = {}
    for src, dst in jumps:
        if dst not in combined_jumps:
            combined_jumps[dst] = set()
        combined_jumps[dst].add(src)
    return [Jumps(dst, sources) for dst, sources in
            combined_jumps.iteritems()]

def range_intersects(a, b):
    return a[1] >= b[0] and a[0] <= b[1]

def range_normalized(a):
    return (a[0], a[1]) if a[0] <= a[1] else (a[1], a[0])

def try_to_insert(column, column_keys, jump):
    i = bisect.bisect_left(column_keys, jump.lower_bound)
    if i < len(column) and range_intersects(jump.range(), column[i].range()):
        return None
    if i > 0 and range_intersects(jump.range(), column[i-1].range()):
        return None

    column.insert(i, jump)
    column_keys.insert(i, jump.lower_bound)
    return i

def main(stdin, stdout):
    input_lines = stdin.readlines()
    addr_to_line, jump_pairs = gather_jumps(input_lines)
    jumps = sorted(combine_jumps((addr_to_line[a], addr_to_line[b]) for a, b
            in jump_pairs), key=lambda x: x.length())

    # Lay out lines
    columns = []
    columns_keys = []
    for jump in jumps:
        for i in xrange(len(columns)):
            if try_to_insert(columns[i], columns_keys[i], jump) is not None:
                break
        else:
            columns.append([jump])
            columns_keys.append([jump.lower_bound])

    # Output arrows with original text
    col_i = [0] * len(columns)
    for l in xrange(len(input_lines)):
        out_line = u''
        for c in reversed(xrange(len(columns))):
            i = col_i[c]
            if i == len(columns[c]):
                out_line += u'  '
                continue

            jump = columns[c][i]

            if l >= jump.upper_bound:
                col_i[c] += 1
            if l < jump.lower_bound or l > jump.upper_bound:
                out_line += u'  '
                continue

            if l == jump.destination:
                if jump.destination in jump.sources:
                    second_char = u'►'
                else:
                    second_char = u'→'
            elif l in jump.sources:
                second_char = u'─'
            else:
                second_char = u' '

            if jump.sources == [jump.destination]:
                first_char = u' '
            elif l == jump.lower_bound:
                first_char = u'┌'
            elif l == jump.upper_bound:
                first_char = u'└'
            elif l == jump.destination or l in jump.sources:
                first_char = u'├'
            else:
                first_char = u'│'

            out_line += first_char + second_char
        stdout.write(out_line + ' ' + input_lines[l])

if __name__ == '__main__':
    import codecs
    stdin = sys.stdin
    stdout = sys.stdout
    if len(sys.argv) == 2:
        stdout = codecs.open(sys.argv[1], 'w', 'utf-8')
    main(stdin, stdout)
