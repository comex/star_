import struct, zlib
a = zlib.compress(open('install.dylib').read())
b = open('freeze.tar.xz').read()
stuff = struct.pack('III', 0x42424242, len(a) + len(b) + 12, len(a)) + a + b
fp = open('wad.bin', 'w')
fp.write(stuff)
