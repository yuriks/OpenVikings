import sys

import lvtools.util as util
from lvtools.loadlist import LoadList

def main():
    header_data = util.load_file(sys.argv[1])

    load_list = LoadList()
    load_list.load(header_data)
    load_list.print_list()

if __name__ == "__main__":
    main()
