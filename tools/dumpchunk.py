import sys

import lvtools.chunk as chunk
import lvtools.util as util

def write_chunk(func, file_prefix='chunk'):
    chunk_id = int(sys.argv[1], 16)
    chunk_data = func(chunk_id)
    util.dump_file('chunks/%s%03x.bin' % (file_prefix, chunk_id), chunk_data)

def main():
    write_chunk(chunk.read_chunk)

if __name__ == '__main__':
    main()
