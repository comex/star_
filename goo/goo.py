import struct, sys, os, re, anydbm, traceback
import warnings
import operator
warnings.simplefilter('error')
from goop import *

def gadget(PC, a, b=[], **kwargs):
    import dmini
    #print PC, hex(dmini.cur.find(PC))
    set_fwd('PC', dmini.cur.find(PC))
    for k, v in kwargs.iteritems():
        set_fwd(k, v)
    heapadd(*b)
    heapadd(*[fwd(i, True) for i in a.split(', ')])

#os.chdir(os.path.dirname(os.path.abspath(sys.argv[0])))

keep_debugs = False

class debug_marker(statue):
    __slots__ = ['name']
    def __init__(self, name):
        self.name = name
    def __len__(self):
        return 0
    def simplify(self, addr):
        return self if keep_debugs else ''
    def __repr__(self):
        return '<dbg:%s>' % self.name

def heapadd(*stuff):
    global heap
    #heap.dbginfo.append((len(heap), getdebugname())) # so we know where we came from
    heap.bits.append(debug_marker(getdebugname()))
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
    reverse = dict((v, k) for (k, v) in names.iteritems())
    pos = 0
    last_name = None
    for bit in heap.bits:
        if isinstance(bit, debug_marker) and bit.name != last_name:
            last_name = bit.name
            sys.stdout.write('\n%08x %s: ' % (pos, bit.name.ljust(15)))
        for entry in troll_string(bit).unpack():
            if isinstance(entry, fwd):
                if entry.debugname is not None:
                    sys.stdout.write('\x1b[31m{%s}\x1b[0m ' % entry.debugname)
                sys.stdout.write('%s=' % entry.name)
                entry = entry.val

            if isinstance(entry, reloc):
                sys.stdout.write('\'%x:' % entry.key)
                entry = entry.value
            if reverse.has_key(entry):
                sys.stdout.write('\x1b[31m%s\x1b[0m ' % reverse[entry])
            else:
                sys.stdout.write('\x1b[34m%s\x1b[0m ' % xrepr(entry))
            pos += 4
    sys.stdout.write('\n')
    sys.stdout.write('%08x end\n' % pos)

def ptr(str, null_terminate=False, heap=None):
    if heap is None: heap = sheap
    if null_terminate: str += '\0'
    str = pad(str, 4)
    result = pointed(str)
    heap.append(result)
    return pointer(result)

def ptrI(*xs, **kwargs):
    return ptr(reduce(operator.add, map(I, xs)), **kwargs)

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
    fwds[name].debugname = getdebugname()
    del fwds[name]

class fwd(statue):
    __slots__ = ['name', 'val', 'debugname']
    def __init__(self, name, force=False):
        if force:
            if fwds.has_key(name): exhaust_fwd(name)
        else:
            assert not fwds.has_key(name)
        fwds[name] = self
        self.name = name
        self.debugname = None
        self.val = None
    def simplify(self, addr):
        if keep_debugs:
            self.val = simplify(self.val, addr)
            return self
        assert self.val is not None
        return simplify(self.val, addr)
    def __repr__(self):
        return '<fwd: %s=%s>' % (self.name, self.val)

def init(*regs, **kwargs):
    global fwds, heap, sheap, heapaddr, dbginfo, pic, keep_debugs
    keep_debugs = True
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


def finalize(heapaddr=None, must_be_simple=True, should_heapdump=False):
    global heap, sheap, keep_debugs
    clear_fwd()
    nheap = heap + sheap 
    if should_heapdump:
        nheap = simplify_times(nheap, heapaddr, 4, False)
        heapdump(nheap)
    if must_be_simple:
        keep_debugs = False
        nheap = simplify_times(nheap, heapaddr, 4, True)
    return nheap

