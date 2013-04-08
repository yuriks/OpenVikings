import sys
from array import array

from lvtools import chunk, util, graphics, level, palette
from lvtools.compression import decompress_data
import png

level_chunk_id = int(sys.argv[1], 0)
output_file = sys.argv[2]
tiles_per_row = int(sys.argv[3])

level_header = level.LevelHeader()
level_header.load(decompress_data(chunk.read_chunk(level_chunk_id)))
output_pal = palette.assemble_palette(level_header.load_list.palettes)

tileset_data = decompress_data(chunk.read_chunk(level_header.tileset_chunk))
tiles = graphics.interleave_tileset(tileset_data)

def rows_of_tiles(nt, n):
    divisible_len = nt // n * n
    for i in xrange(0, divisible_len, n):
        yield xrange(i, i+n)
    if divisible_len != nt:
        yield range(divisible_len, nt) + [0]*(divisible_len + n - nt)

output_data, output_w, output_h = graphics.layout_tiles(tiles, 8, 8, rows_of_tiles(len(tiles), tiles_per_row))

with open(output_file, 'wb') as f:
    w = png.Writer(output_w, output_h, palette=output_pal)
    w.write_array(f, output_data)
