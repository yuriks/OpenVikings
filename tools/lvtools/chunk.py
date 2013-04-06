import os
from struct import Struct
from array import array

import config

offset_pair_struct = Struct('<II')

data_file = None

def get_data_file():
    global data_file

    if data_file is None:
        data_file = open(config.data_dat_path, 'rb')
    return data_file

def seek_to_chunk(f, chunk_id):
    f.seek(chunk_id * 4)
    chunk_begin, chunk_end = offset_pair_struct.unpack(f.read(offset_pair_struct.size))
    f.seek(chunk_begin)
    return chunk_end - chunk_begin

def read_chunk(chunk_id):
    f = get_data_file()
    chunk_size = seek_to_chunk(f, chunk_id)
    data = array('B')
    data.fromfile(f, chunk_size)
    return data
