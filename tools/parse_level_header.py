import sys

import lvtools.chunk as chunk
from lvtools.compression import decompress_data
from lvtools.level import LevelHeader

level_header_id = int(sys.argv[1], 0)

header_data = decompress_data(chunk.read_chunk(level_header_id))
header = LevelHeader()
header.load(header_data)
print header
