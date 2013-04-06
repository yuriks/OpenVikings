import sys

import lvtools.chunk as chunk
from lvtools.compression import decompress_data
from lvtools.loadlist import LoadList
import lvtools.palette as palette
import lvtools.util as util

level_header_id = int(sys.argv[1], 0)
output_file = sys.argv[2]

level_header = decompress_data(chunk.read_chunk(level_header_id))
load_list = LoadList()
load_list.load(level_header)
pal = palette.assemble_palette(load_list.palettes)
util.dump_file(output_file, palette.pack_palette(pal))
