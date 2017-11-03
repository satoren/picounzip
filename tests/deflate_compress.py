import zlib
import sys

def compress(filename):
    compressor = zlib.compressobj(9,zlib.DEFLATED,-zlib.MAX_WBITS)
    with open(filename,"rb") as rf, open(filename + ".z","wb") as wf:
        while True:
            data = rf.read(512)
            if not data:
                break
            wf.write(compressor.compress(data))
        wf.write(compressor.flush())
            
for file in sys.argv[1:]:
    compress(file);
