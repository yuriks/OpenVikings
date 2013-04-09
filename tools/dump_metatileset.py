import sys
from array import array

from lvtools import chunk, util, graphics, palette, level_info
from lvtools.compression import decompress_data
import png
import dump_tileset as tileset

def parse_metatile_info(data):
    metatiles = []
    for i in xrange(0, len(data), 2 * 4):
        # Read 4 values from data
        tile = (util.read_word(data, i+offs) for offs in xrange(0, 8, 2))
        # Extract attributes
        tile = [(x >> 6, x & 0x10, x & 0x20) for x in tile]
        metatiles.append(tile)
    return metatiles

def main(level_header, output_file, tiles_per_row):
    output_pal = palette.assemble_palette(level_header.load_list.palettes)
    tiles = tileset.extract_tiles(level_header)

    metatile_data = decompress_data(chunk.read_chunk(level_header.metatile_chunk))
    meta_info = parse_metatile_info(metatile_data)
    del metatile_data

    def assemble_metatile(info):
        flipped_tiles = [graphics.flip_image(tiles[i], 8, hf, vf) for i, hf, vf in info]
        return graphics.layout_tiles(flipped_tiles, 8, 8, [[0, 1], [2, 3]])[0]

    meta_imgs = [assemble_metatile(info) for info in meta_info]

    output_data, output_w, output_h = graphics.layout_tiles(meta_imgs, 16, 16, tileset.rows_of_tiles(len(meta_imgs), tiles_per_row))

    with open(output_file, 'wb') as f:
        w = png.Writer(output_w, output_h, palette=output_pal)
        w.write_array(f, output_data)

if __name__ == '__main__':
    level_id = int(sys.argv[1], 0)
    level_header = level_info.load_level_header(level_id=level_id)
    main(level_header, sys.argv[2], int(sys.argv[3]))
