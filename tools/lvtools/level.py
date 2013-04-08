from util import read_word

class ObjectInstance(object):
    def __init__(self, xpos, ypos, width, height, type_id, unk4, user_data):
        self.xpos = xpos
        self.ypos = ypos
        self.width = width
        self.height = height
        self.type_id = type_id
        self.unk4 = unk4
        self.user_data = user_data

    def __str__(self):
        return "ObjectInstance {{ x: {0.xpos:4}, y: {0.ypos:4}, w: {0.width:3}, h: {0.height:3}, type: {0.type_id:02X}h, {0.unk4:3X}h, userdata: {0.user_data:4X}h }}".format(self)

def load_object_list(data, pos):
    objects = []

    while True:
        xpos = read_word(data, pos)
        if xpos == 0xFFFF:
            pos += 2
            break
        ypos = read_word(data, pos+2)
        width = read_word(data, pos+4)
        height = read_word(data, pos+6)
        type_id = read_word(data, pos+8)
        unk4 = read_word(data, pos+10)
        user_data = read_word(data, pos+12)

        objects.append(ObjectInstance(xpos, ypos, width, height, type_id, unk4, user_data))

        pos += 14;

    return pos, objects

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

def load_palette_cycle_list(data, pos):
    active_cycles = read_word(data, pos)
    pos += 2

    cycles = []

    while True:
        speed = data[pos]
        if speed == 0:
            pos += 1
            break
        first = data[pos+1]
        last = data[pos+2]
        pos += 3

        cycles.append((speed, first, last))

        while True:
            val_d = read_word(data, pos)
            pos += 2
            if val_d == 0xFFFF:
                break

    return pos, active_cycles, cycles

def load_object_graphics_list(data, pos):
    chunks = []
    while True:
        chunk_id = read_word(data, pos)
        pos += 2
        if chunk_id == 0xFFFF:
            break

        chunks.append(chunk_id)

    return pos, chunks

def load_animation_list(data, pos):
    chunks = []
    while True:
        chunk_id = read_word(data, pos)
        if chunk_id == 0xFFFF:
            break
        pos += 5

        chunks.append(chunk_id)

    return chunks

class LoadList(object):
    def __init__(self):
        self.objects = None
        self.palettes = None
        self.active_pal_cycles = None
        self.pal_cycles = None
        self.objects = None
        self.animations = None

    def load(self, load_list):
        current_pos, self.objects = load_object_list(load_list, 0)
        current_pos, self.palettes = load_palette_list(load_list, current_pos)
        current_pos, self.active_pal_cycles, self.pal_cycles = load_palette_cycle_list(load_list, current_pos)
        current_pos, self.object_gfx = load_object_graphics_list(load_list, current_pos)
        self.animations = load_animation_list(load_list, current_pos)

    def __str__(self):
        s = ''

        for obj in self.objects:
            s += str(obj) + '\n'
        for chunk_id, load_position in self.palettes:
            s += "Load palette chunk 0x%03X into position %d (offs: 0x%03X)\n" % (
                    chunk_id, load_position, load_position*3)
        s += "Enabled cycles: {0:08b}\n".format(self.active_pal_cycles)
        for i, vals in enumerate(self.pal_cycles):
            if (1 << i) & self.active_pal_cycles:
                active_str = ''
            else:
                active_str = " (inactive)"
            s += "Palette cycle:%s speed: %d, colors: %d-%d\n" % ((active_str,) + vals)
        for chunk_id in self.object_gfx:
            s += "Load object graphics from chunk 0x%03X\n" % (chunk_id,)
        for chunk_id in self.animations:
            s += "Load animation graphics from chunk 0x%03X\n" % (chunk_id,)

        return s

class LevelHeader(object):
    def __init__(self):
        pass

    def load(self, data):
        self.special_objs_type = data[0x7]
        self.special_obj_x = read_word(data, 0x8)
        self.special_obj_y = read_word(data, 0xA)
        self.special_obj_type = read_word(data, 0xC)
        self.special_obj_unk1 = read_word(data, 0xE)
        self.special_obj_user_data = read_word(data, 0x10)

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
Special objects type: {0.special_objs_type}
Special object info: {{ x: {0.special_obj_x}, y: {0.special_obj_y}, type: {0.special_obj_type:02X}h, {0.special_obj_unk1:X}h, userdata: {0.special_obj_user_data:X}h }}

============ Load list ============
{0.load_list}"""

    def __str__(self):
        return self.format_string.format(self)
