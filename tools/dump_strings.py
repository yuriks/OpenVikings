import sys
import string

from lvtools import config
from lvtools.util import fread_word

if len(sys.argv) >= 2:
    table_start = int(sys.argv[1], 0)
else:
    table_start = 0x7C60
vikings = open(config.vikings_exe_path, 'rb')
vikings.seek(table_start)

current_position = 0
lowest_string = 0x10000
addresses = []
while current_position < lowest_string:
    address = fread_word(vikings)
    current_position += 2
    lowest_string = min(lowest_string, address)
    addresses.append(address)

for i, addr in enumerate(addresses):
    vikings.seek(table_start + addr)
    b = vikings.read(1)
    if ord(b) < 0x20:
        xpos = ord(b)
        ypos = ord(vikings.read(1))
        b = vikings.read(1)
        print '; %d %d %d' % (i, xpos, ypos)
    else:
        print '; %d' % (i,)

    s = ''
    while b != '\0':
        if b == '\r':
            s += '\n'
        elif b not in string.printable:
            s += '\\x%02x' % (ord(b),)
        else:
            s += b
        b = vikings.read(1)

    print s
    print
