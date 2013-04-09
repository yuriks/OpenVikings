import sys
import dump_tilemap
from lvtools import level_info

if __name__ == '__main__':
    filename = "tilemaps/%02d_%s.txt"
    for i, (chunk_id, name) in enumerate(level_info.level_info):
        if name in ('MainMenu', 'MainMenu2', 'Ending1'):
            # Don't have tilesets
            continue
        print "Dumping %s..." % (name,)

        with open(filename % (i, name), 'w') as f:
            dump_tilemap.main(level_info.load_level_header(chunk_id=chunk_id), f)
