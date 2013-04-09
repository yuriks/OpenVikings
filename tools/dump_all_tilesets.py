import dump_tileset
from lvtools.level_info import level_info


if __name__ == '__main__':
    filename = "tilesets/%02d_%s.png"
    for i, (chunk_id, name) in enumerate(level_info):
        if name in ('MainMenu', 'MainMenu2', 'Ending1'):
            # Don't have tilesets
            continue
        print "Dumping %s..." % (name,)
        dump_tileset.main(chunk_id, filename % (i, name), 16)
