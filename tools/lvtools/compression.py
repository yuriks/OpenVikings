from array import array
import itertools

from util import read_word

def decompress_data(data, decoded_data_len=None):
    if decoded_data_len is None:
        decoded_data_len = read_word(data, 0) + 1
        data = data[2:]

    scratch = array('B', itertools.repeat(0, 0x1000))
    dst = array('B')

    scratch_i = 0
    src_i = 0

    while True:
        al = data[src_i]
        src_i += 1

        for i in xrange(8):
            if al & 1:
                # Verbatim byte
                scratch[scratch_i] = data[src_i]
                scratch_i = (scratch_i + 1) % len(scratch)
                dst.append(data[src_i])
                if len(dst) == decoded_data_len:
                    return dst
                src_i += 1
            else:
                # Copy previous bytes from scratch area
                copy_src = read_word(data, src_i)
                src_i += 2

                copy_len = (copy_src >> 12) + 3
                copy_src = copy_src & 0xFFF

                for j in xrange(copy_len):
                    scratch[scratch_i] = scratch[copy_src]
                    scratch_i = (scratch_i + 1) % len(scratch)
                    dst.append(scratch[copy_src])
                    if len(dst) == decoded_data_len:
                        return dst
                    copy_src = (copy_src + 1) % len(scratch)
            al = al >> 1
