from util import read_word

def seek_list(data):
    pos = 0
    while True:
        val = read_word(data, pos)
        if val == 0xFFFF:
            return pos + 2
        else:
            pos += 0xE;

def load_palette_list(data, pos):
    pals = []

    while True:
        chunk_id = read_word(data, pos)
        pos += 2
        if chunk_id == 0xFFFF:
            break

        load_position = data[pos]
        pos += 1

        pals.append((chunk_id, load_position))

    return pos, pals

def process_list(data, pos):
    unk_bitset = read_word(data, pos)
    pos += 2

    unk_list = []

    while True:
        val_a = data[pos]
        if val_a == 0:
            pos += 1
            break
        val_b = data[pos+1]
        val_c = data[pos+2]
        pos += 3

        unk_list.append((val_a, val_b, val_c))

        while True:
            val_d = read_word(data, pos)
            pos += 2
            if val_d == 0xFFFF:
                break

    return pos, (unk_bitset, unk_list)

def load_chunk_list(data, pos):
    chunks = []
    while True:
        chunk_id = read_word(data, pos)
        pos += 2
        if chunk_id == 0xFFFF:
            break

        chunks.append(chunk_id)

    return pos, chunks

class LoadList(object):
    # Offset of the load list into the level header
    LOAD_LIST_OFFSET = 0x43

    def __init__(self):
        self.palettes = None
        self.unknown = None
        self.objects = None
        self.animations = None

    def load(self, header_data):
        load_list = header_data[self.LOAD_LIST_OFFSET:]

        current_pos = seek_list(load_list)
        current_pos, self.palettes = load_palette_list(load_list, current_pos)
        current_pos, self.unknown = process_list(load_list, current_pos)
        current_pos, self.objects = load_chunk_list(load_list, current_pos)
        current_pos, self.animations = load_chunk_list(load_list, current_pos)

    def print_list(self):
        for chunk_id, load_position in self.palettes:
            print "Load palette chunk 0x%03X into position %d (offs: 0x%03X)" % (
                    chunk_id, load_position, load_position*3)
        print "Unknown bitset: %02x" % (self.unknown[0],)
        for vals in self.unknown[1]:
            print "Unknown data: %02x %02x %02x" % vals
        for chunk_id in self.objects:
            print "Load object graphics from chunk 0x%03X" % (chunk_id,)
        for chunk_id in self.animations:
            print "Load animation graphics from chunk 0x%03X" % (chunk_id,)
