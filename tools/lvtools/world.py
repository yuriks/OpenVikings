from util import read_word, take_n

class ObjectType(object):
    ENTRY_SIZE = 0x15

    def __init__(self, data):
        self.gfx_chunk_id = read_word(data, 0x0)
        self.num_sprites = data[0x2]
        self.script_entry_point = read_word(data, 0x3)
        self.unk_5 = read_word(data, 0x5)
        self.unk_7 = read_word(data, 0x7)
        self.width = data[0x9]
        self.height = data[0xA]
        self.unk_B = read_word(data, 0xB)
        self.unk_D = read_word(data, 0xD)
        self.unk_F = read_word(data, 0xF)
        self.max_x_velocity = read_word(data, 0x11)
        self.max_y_velocity = read_word(data, 0x13)

    format_str = (
"""Graphics Chunk: {0.gfx_chunk_id:04X}h
Number of sprites: {0.num_sprites}
Script entry point: {0.script_entry_point:04X}h + 3
unk_5: {0.unk_5:X}h
unk_7: {0.unk_7:X}h
Width: {0.width}
Height: {0.height}
unk_B: {0.unk_B:X}h
unk_D: {0.unk_D:X}h
unk_F: {0.unk_F:X}h
Max X velocity: {0.max_x_velocity}
Max Y velocity: {0.max_y_velocity}""")

    def __str__(self):
        return self.format_str.format(self)

def print_object(world_data, obj_idx):
    offset = obj_idx * ObjectType.ENTRY_SIZE
    print ObjectType(world_data[offset:offset + ObjectType.ENTRY_SIZE])

def print_objects(world_data, num_objects):
    objects_data = world_data[:num_objects * ObjectType.ENTRY_SIZE]

    for i, obj_data in enumerate(take_n(objects_data, ObjectType.ENTRY_SIZE)):
        obj = ObjectType(obj_data)
        print "=== Object type %Xh\n%s\n" % (i, obj)
