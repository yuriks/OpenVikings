import sys
import dumpplanarimg
from array import array

def main():
    infilename = sys.argv[1]
    offset = int(sys.argv[2], 0)
    plane_size = int(sys.argv[3], 0)
    image_count = int(sys.argv[4], 0)
    outfilename = sys.argv[5]

    with open(infilename, 'rb') as f:
        f.seek(offset)
        indata = array('B')
        indata.fromfile(f, plane_size*4*image_count)
    data_offset = 0
    outdata = array('B')
    for i in xrange(image_count):
        print "Dump at %x" % (data_offset,)
        dumpplanarimg.deplane_image(indata, data_offset, plane_size, outdata)
        data_offset += plane_size*4
        print "data size %d" % (len(outdata),)
    with open(outfilename, 'wb') as f:
        outdata.tofile(f)

if __name__ == "__main__":
    main()
