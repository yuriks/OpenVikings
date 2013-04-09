from array import array
from util import read_word, take_n

def interleave_planar(data):
    plane_size = len(data) / 4

    pixels = zip(
        data[0*plane_size : 1*plane_size],
        data[1*plane_size : 2*plane_size],
        data[2*plane_size : 3*plane_size],
        data[3*plane_size : 4*plane_size]
    )

    out_data = array('B')
    for t in pixels:
        for p in t:
            out_data.append(p)

    return out_data

def read_image(data):
    """Deinterleaves a length-prefixed image"""
    data_len = read_word(data, 0) * 4
    return interleave_planar(data[2:2+data_len])

def interleave_tileset(data):
    tile_size = 8*8

    tiles = []
    for planar_tile in take_n(data, tile_size):
        tiles.append(interleave_planar(planar_tile))
    return tiles

def layout_tiles(tiles, tile_w, tile_h, tilemap):
    output = array('B')
    out_w = None
    out_h = 0

    for row in tilemap:
        out_h += tile_h
        for y in xrange(tile_h):
            for tile_id in row:
                data_begin = y * tile_w
                output += tiles[tile_id][data_begin : data_begin + tile_w]
            out_w = tile_w * len(row)

    return output, out_w, out_h

def flip_image(image, image_w, hflip=False, vflip=False):
    if not hflip and not vflip:
        return image

    rows = list(take_n(image, image_w))

    if vflip:
        rows.reverse()

    if hflip:
        for row in rows:
            row.reverse()

    # flatten list of arrays back
    return array('B', (pixel for row in rows for pixel in row))
