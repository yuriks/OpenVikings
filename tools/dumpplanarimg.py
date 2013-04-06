import sys
import dumpchunk
from array import array
from struct import Struct
import itertools

def deplane_image(data, offset=0, plane_size=None, out_data=None):
    if plane_size is None:
        plane_size = len(data) / 4

    pixels = zip(
        data[offset + 0*plane_size : offset + 1*plane_size],
        data[offset + 1*plane_size : offset + 2*plane_size],
        data[offset + 2*plane_size : offset + 3*plane_size],
        data[offset + 3*plane_size : offset + 4*plane_size]
    )

    if out_data is None:
        out_data = array('B')
    for t in pixels:
        for p in t:
            out_data.append(p)

    return out_data

def main():
    word = Struct('<H')

    def deplane(f, chunk_id):
        data = dumpchunk.read_chunk(f, chunk_id)
        plane_size = word.unpack_from(data, 0)[0]
        return deplane_image(data[2:2+plane_size*4])

    dumpchunk.write_chunk(deplane, 'image')

if __name__ == '__main__':
    main()
