import sys
from array import array

from lvtools import chunk, util, graphics, level, palette, level_info
from lvtools.compression import decompress_data
import png

def extract_tiles(level_header):
    tileset_data = decompress_data(chunk.read_chunk(level_header.tileset_chunk))
    return graphics.interleave_tileset(tileset_data)

def rows_of_tiles(nt, n):
    divisible_len = nt // n * n
    for i in xrange(0, divisible_len, n):
        yield xrange(i, i+n)
    if divisible_len != nt:
        yield range(divisible_len, nt) + [0]*(divisible_len + n - nt)

def main(header_chunk_id, output_file, tiles_per_row):
    level_header = level_info.load_level_header(chunk_id=header_chunk_id)
    output_pal = palette.assemble_palette(level_header.load_list.palettes)
    tiles = extract_tiles(level_header)

    output_data, output_w, output_h = graphics.layout_tiles(tiles, 8, 8, rows_of_tiles(len(tiles), tiles_per_row))

    with open(output_file, 'wb') as f:
        w = png.Writer(output_w, output_h, palette=output_pal)
        w.write_array(f, output_data)

if __name__ == '__main__':
    main(int(sys.argv[1], 0), sys.argv[2], int(sys.argv[3]))
