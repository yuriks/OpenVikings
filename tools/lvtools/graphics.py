from array import array
from util import read_word

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
