import sys

from lvtools import util

def strip_newline(s):
    if s and s[-1] == '\n':
        return s[:-1]
    return s

def process_escapes(s):
    outs = ''
    i = 0
    while i < len(s):
        if s[i:i+2] == '\\\\':
            outs += '\\'
            i += 2
        elif s[i:i+2] == '\\x':
            outs += chr(int(s[i+2:i+4], 16))
            i += 4
        else:
            outs += s[i]
            i += 1
    return outs

def find_duplicate(s, strings):
    for k in strings:
        if s == strings[k]:
            return k
    return None

inf = open(sys.argv[1], 'rU')
max_string = 0
strings = {}

current_id = None
for line in inf:
    line = strip_newline(line)
    if line and line[0] == ';':
        if current_id is not None:
            max_string = max(max_string, current_id)
            s = process_escapes(s[:-1]) + '\0' # Strip trailing newline
            dup = find_duplicate(s, strings)
            if dup is not None:
                strings[current_id] = dup
            else:
                strings[current_id] = s

        line = line[1:].split()
        current_id = int(line[0])
        if len(line) > 1:
            s = chr(int(line[1])) + chr(int(line[2]))
        else:
            s = ''
    else:
        s += line + '\r'

max_string = max(max_string, current_id)
s = process_escapes(s[:-1]) + '\0' # Strip trailing newline
dup = find_duplicate(s, strings)
if dup is not None:
    strings[current_id] = dup
else:
    strings[current_id] = s

inf.close()

outf = open(sys.argv[2], 'wb')
cur_pos = (max_string+1)*2
positions = []
for i in xrange(max_string+1):
    if i in strings:
        if isinstance(strings[i], int):
            val = positions[strings[i]]
        else:
            val = cur_pos
            cur_pos += len(strings[i])
    else:
        val = 0
        print 'WARNING: Missing string %d' % (i,)
    positions.append(val)
    outf.write(util.word.pack(val))
for i in xrange(max_string+1):
    if i in strings and not isinstance(strings[i], int):
        outf.write(strings[i])

if cur_pos % 16 != 0:
    outf.write('\xff' * (16 - cur_pos % 16))

print 'Wrote %d string ids.' % (max_string+1)
print 'Result size: %04Xh (%04Xh padded)' % (cur_pos, outf.tell())

outf.close()
