import sys
from array import array
from struct import Struct

offset_pair_struct = Struct('<II')

def seek_to_chunk(f, chunk_id):
    f.seek(chunk_id * 4)
    chunk_begin, chunk_end = offset_pair_struct.unpack(f.read(offset_pair_struct.size))
    f.seek(chunk_begin)
    return chunk_end - chunk_begin

def read_chunk(f, chunk_id):
    chunk_size = seek_to_chunk(f, chunk_id)
    data = array('B')
    data.fromfile(f, chunk_size)
    return data

def write_chunk(func):
    chunk_id = int(sys.argv[1], 16)
    with open('data.dat', 'rb') as f:
        chunk_data = func(f, chunk_id)
    with open('chunks/chunk%03x.bin' % (chunk_id,), 'wb') as f:
        f.write(chunk_data)

def main():
    write_chunk(read_chunk)

if __name__ == '__main__':
    main()
