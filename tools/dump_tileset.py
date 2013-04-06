import sys
from array import array

from lvtools import chunk, util, graphics
from lvtools.compression import decompress_data

tileset_chunk = int(sys.argv[1], 0)
output_file = sys.argv[2]

tileset_data = decompress_data(chunk.read_chunk(tileset_chunk))
tiles = graphics.interleave_tileset(tileset_data)

outdata = array('B')
for tile in tiles:
    outdata += tile

util.dump_file(output_file, outdata)
