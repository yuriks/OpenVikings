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
    def __init__(self):
        self.palettes = None
        self.unknown = None
        self.objects = None
        self.animations = None

    def load(self, load_list):
        current_pos = seek_list(load_list)
        current_pos, self.palettes = load_palette_list(load_list, current_pos)
        current_pos, self.unknown = process_list(load_list, current_pos)
        current_pos, self.objects = load_chunk_list(load_list, current_pos)
        current_pos, self.animations = load_chunk_list(load_list, current_pos)

    def __str__(self):
        s = ''

        for chunk_id, load_position in self.palettes:
            s += "Load palette chunk 0x%03X into position %d (offs: 0x%03X)\n" % (
                    chunk_id, load_position, load_position*3)
        s += "Unknown bitset: %02x\n" % (self.unknown[0],)
        for vals in self.unknown[1]:
            s += "Unknown data: %02x %02x %02x\n" % vals
        for chunk_id in self.objects:
            s += "Load object graphics from chunk 0x%03X\n" % (chunk_id,)
        for chunk_id in self.animations:
            s += "Load animation graphics from chunk 0x%03X\n" % (chunk_id,)

        return s

class LevelHeader(object):
    def __init__(self):
        pass

    def load(self, data):
        self.next_level = read_word(data, 0x16)
        self.flags = data[0x1C]

        self.width = read_word(data, 0x29)
        self.height = read_word(data, 0x29)

        self.tilemap_chunk = read_word(data, 0x2E)
        self.tileset_chunk = read_word(data, 0x30)
        self.metatile_chunk = read_word(data, 0x32)

        self.load_list = LoadList()
        self.load_list.load(data[0x43:])

    format_string = \
"""Next level: 0x{0.next_level:02X}
Flags: 0x{0.flags:02X}
Width (metatiles): {0.width}
Height (metatiles): {0.height}
Tilemap chunk: 0x{0.tilemap_chunk:03X}
Tileset chunk: 0x{0.tileset_chunk:03X}
Metatile chunk: 0x{0.metatile_chunk:03X}

============ Load list ============
{0.load_list}"""

    def __str__(self):
        return self.format_string.format(self)
