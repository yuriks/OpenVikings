import sys
import dumpchunk
from array import array
from struct import Struct
import itertools

def deplane_image(data):
    plane_size = len(data) / 4

    pixels = zip(
        data[0*plane_size:1*plane_size],
        data[1*plane_size:2*plane_size],
        data[2*plane_size:3*plane_size],
        data[3*plane_size:4*plane_size]
    )

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
