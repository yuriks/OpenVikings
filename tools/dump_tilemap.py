import sys

from lvtools import chunk, level_info
from lvtools.compression import decompress_data
from lvtools.util import take_n, read_word

def main(level_header, out):
    out.write("%d %d\n" % (level_header.width, level_header.height))

    tilemap_data = decompress_data(chunk.read_chunk(level_header.tilemap_chunk))

    for row in take_n(tilemap_data, level_header.width * 2):
        out.write(' '.join(str(read_word(row, i)) for i in xrange(0, len(row), 2)))
        out.write('\n')

if __name__ == '__main__':
    level_id = int(sys.argv[1], 0)
    main(level_info.load_level_header(level_id=level_id), sys.stdout)
