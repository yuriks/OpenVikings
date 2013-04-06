from array import array

import chunk
from compression import decompress_data
import util

def unpack_palette(data):
    pal = []
    for r,g,b in util.take_n(data, 3):
        pal.append((r, g, b))
    return pal

def pack_palette(pal):
    data = array('B')
    for r,g,b in pal:
        data.append(r)
        data.append(g)
        data.append(b)
    return data

def assemble_palette(pal_list):
    # Initialize with neon pink
    pal = [(255, 0, 255)] * 256

    for chunk_id, position in pal_list:
        loaded_pal = unpack_palette(decompress_data(chunk.read_chunk(chunk_id)))

        # Convert from 6-bit to 8-bit
        pal[position:position + len(loaded_pal)] = ((r << 2, g << 2, b << 2) for r,g,b in loaded_pal)

    # Zero entries of each sub-pal like the game does
    for i in xrange(16, 256, 16):
        pal[i] = (0, 0, 0)

    return pal
