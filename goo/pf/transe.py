#!/opt/local/bin/python2.6
from world1 import *
import config
cfg = config.openconfig()

init('PC')
make_avail()

zero = ptrI(0)
funcall(cfg['#cache']['sysctlbyname'], ptr('security.mac.proc_enforce', True), 0, 0, zero, 4)
funcall(cfg['#cache']['sysctlbyname'], ptr('security.mac.vnode_enforce', True), 0, 0, zero, 4)
pf2 = ptr('/usr/lib/pf2', True)
funcall(cfg['#cache']['execve'], pf2, ptrI(pf2, 0), zero)

final = finalize(0x11000000+8)
#(for debug)
#heapdump(cache)

open('transeboot.txt', 'w').write(final)
