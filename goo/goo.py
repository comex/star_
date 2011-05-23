import struct, sys, os, re, anydbm, traceback
import warnings
import operator
warnings.simplefilter('error')
from goop import *

#os.chdir(os.path.dirname(os.path.abspath(sys.argv[0])))

def heapadd(*stuff):
    global heap, dbginfo
    dbginfo.append((len(heap), getdebugname())) # so we know where we came from
    for a in stuff:
        assert not isinstance(a, (troll_string, str)) or len(a) % 4 == 0
        heap.append(I(a))

def xrepr(a):
    if isinstance(a, (int, long)):
        return hex(a)
    else:
        return repr(a)

def heapdump(heap, names=None):
    if names is None: names = {}
    dbginfo.append((10000, '?'))
    reverse = dict((v, k) for (k, v) in names.iteritems())
    stackunktargets = set()
    for i, entry in enumerate(troll_string(heap).unpack()):
        if 4*i >= dbginfo[0][0]:
            sys.stdout.write('\n%08x %s: ' % (4*i, dbginfo.pop(0)[1].ljust(15)))
        if isinstance(entry, reloc):
            sys.stdout.write('\'%x:' % entry.key)
            entry = entry.value
        if reverse.has_key(entry):
            sys.stdout.write('\x1b[31m%s\x1b[0m ' % reverse[entry])
        else:
            sys.stdout.write('\x1b[34m%s\x1b[0m ' % xrepr(entry))
    sys.stdout.write('\n')
    sys.stdout.write('%08x end\n' % (4*i + 4))

def ptr(str, null_terminate=False):
    global sheap
    if null_terminate: str += '\0'
    str = pad(str, 4)
    result = pointed(str)
    sheap.append(result)
    return pointer(result)

def ptrI(*xs):
    return ptr(reduce(operator.add, map(I, xs)))


def clear_fwd():
    global fwds
    for a in fwds.values():
        a.val = 0
    fwds = {}


# [0] evaluates to 0 (initially), [1] evaluates to the address of [0]
def stackunkpair():
    unk = pointed('\0\0\0\0')
    unkptr = pointer(unk)
    return unk, unkptr

def exhaust_fwd(*names):
    for name in names:
        if fwds.has_key(name):
            fwds[name].val = 0
            del fwds[name]
def set_fwd(name, val):
    assert fwds.has_key(name) and val is not None
    fwds[name].val = val
    del fwds[name]

class fwd(statue):
    __slots__ = ['name', 'val']
    def __init__(self, name, force=False):
        if force:
            if fwds.has_key(name): exhaust_fwd(name)
        else:
            assert not fwds.has_key(name)
        fwds[name] = self
        self.name = name
        self.val = None
    def simplify(self, addr):
        assert self.val is not None
        return simplify(self.val, addr)
    def __repr__(self):
        return '<fwd: %s>' % self.val

def init(*regs, **kwargs):
    global fwds, heap, sheap, heapaddr, dbginfo, pic
    pic = False
    if kwargs.has_key('pic'):
        pic = True
        del kwargs['pic']
    if kwargs: raise ValueError(kwargs)
    fwds = {}
    dbginfo = []
    heap = troll_string('')
    sheap = troll_string('')
    heapadd(*map(fwd, regs))


def finalize(heapaddr=None, must_be_simple=True):
    global heap, sheap
    clear_fwd()
    nheap = heap + sheap 
    result = simplify_times(nheap, heapaddr, 4, must_be_simple)
    
    return result

