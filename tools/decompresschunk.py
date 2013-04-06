import dumpchunk
import lvtools.chunk as chunk
from lvtools.compression import decompress_data

def main():
    def decomp(chunk_id):
        return decompress_data(chunk.read_chunk(chunk_id))

    dumpchunk.write_chunk(decomp)

if __name__ == '__main__':
    main()
