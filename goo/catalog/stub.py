#!/opt/local/bin/python2.6
import sys, base64, shlex, zlib
import dmini
from world1 import *
import goo

if len(sys.argv) != 3:
    print >> sys.stderr, "usage: python stub.py -c cache"
    sys.exit(1)

dmini.init(sys.argv[1:])

# add sp, #136; pop {pc}
parse_callback = dmini.find_basic('+ 22 b0 00 bd')

init('PC')
make_avail()
