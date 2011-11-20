import sys
import os.path
import struct

entry_struct = struct.Struct('=3B')

def convert_pal(inf, outf, file_name):
    data = inf.read()
    n = len(data) / entry_struct.size
    entries = []
    for i in xrange(n):
        entries.append(entry_struct.unpack_from(data, i*entry_struct.size))

    outf.write("GIMP Palette\nName: %s\nColumns: 16\n#\n" % (file_name,))
    for e in entries:
        outf.write("%3d %3d %3d\tUntitled\n" % (e[0] << 2, e[1] << 2, e[2] << 2))

for raw_infn in sys.argv[1:]:
    basepath, infn = os.path.split(raw_infn)
    infn = os.path.splitext(infn)[0]
    outf = open(os.path.join(basepath, "pals", infn + ".gpl"), "w")
    inf = open(raw_infn, "rb")
    convert_pal(inf, outf, infn)
    outf.close()
    inf.close()
