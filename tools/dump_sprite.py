import sys

from lvtools import palette, level_info, compression, chunk, graphics
import png

def calc_frame_length(size):
    return size*size / 8 * 9

def extract_frame(data, size):
    masked = graphics.mask_sprite(data, size)
    return graphics.interleave_planar(masked)

def extract_frames(chunk_id, offset, count, size):
    frames = []
    frame_len = calc_frame_length(size)
    sprite_data = compression.decompress_data(chunk.read_chunk(chunk_id))
    if count == -1:
        count = len(sprite_data) / frame_len
    data_size = frame_len * count
    sprite_data = sprite_data[offset:offset+data_size]
    for offs in xrange(0, data_size, frame_len):
        frames.append(extract_frame(sprite_data[offs:offs+frame_len], size))
    return frames

def join_arrays(arraylist):
    from array import array
    out = array('B')
    for a in arraylist:
        out.extend(a)
    return out

def main(output_file, output_pal, chunk_id, offset, count, size):
    assert size in (8, 16, 32)

    frames = extract_frames(chunk_id, offset, count, size)

    output_data, output_w, output_h = graphics.layout_tiles(frames, size, size, ((i,) for i in xrange(len(frames))))

    with open(output_file, 'wb') as f:
        w = png.Writer(output_w, output_h, palette=output_pal)
        w.write_array(f, output_data)

def module_main(argv):
    if len(argv) == 7:
        level_id = int(argv[5], 0)
        level_header = level_info.load_level_header(level_id=level_id)
        output_pal = palette.assemble_palette(level_header.load_list.palettes)
        main(argv[6], output_pal, int(argv[1], 0), int(argv[2], 0), int(argv[3], 0), int(argv[4]))
    else:
        print 'Usage: {0} <chunk_id> <offset> <count> <size> <level_palette> <output.png>'.format(argv[0])

if __name__ == '__main__':
    module_main(sys.argv)
