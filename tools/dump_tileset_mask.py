import sys
from array import array

from lvtools import chunk, graphics, level_info
from lvtools.compression import decompress_data
from dump_tileset import rows_of_tiles
import png

def extract_tiles(level_header):
    tileset_data = decompress_data(chunk.read_chunk(level_header.tileset_chunk + 1))

    expanded = array('B')
    for byte in tileset_data:
        for bit in xrange(8):
            expanded.append((byte & 0x80) and 1 or 0)
            byte <<= 1

    return graphics.interleave_tileset(expanded)

def main(level_header, output_file, tiles_per_row):
    tiles = extract_tiles(level_header)

    output_data, output_w, output_h = graphics.layout_tiles(tiles, 8, 8, rows_of_tiles(len(tiles), tiles_per_row))

    with open(output_file, 'wb') as f:
        w = png.Writer(output_w, output_h, greyscale=True, bitdepth=1)
        w.write_array(f, output_data)

if __name__ == '__main__':
    level_id = int(sys.argv[1], 0)
    level_header = level_info.load_level_header(level_id=level_id)
    main(level_header, sys.argv[2], int(sys.argv[3]))
