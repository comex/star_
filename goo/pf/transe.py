#!/opt/local/bin/python2.6
from world1 import *
import config
cfg = config.openconfig()

init('PC')
make_avail()

funcall('_sysctlbyname', ptr('security.mac.proc_enforce', True), 0, 0, ptrI(0), 4)
funcall('_dlopen', ptr('/var/root/libpf2.dylib', True))

final = finalize(0x11000000+8)
#(for debug)
#heapdump(cache)

open('transeboot.txt', 'w').write(final)
