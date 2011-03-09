import struct, sys, os, re, anydbm, traceback
import warnings
import operator
warnings.simplefilter('error')

#os.chdir(os.path.dirname(os.path.abspath(sys.argv[0])))
dontcare = 0

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
    heapadd(*map(fwd, regs))
    sheap = troll_string('')

def getdebugname():
    f = sys._getframe().f_back
    while True:
        f = f.f_back
        fn = os.path.basename(f.f_code.co_filename)
        if False and ('world' in fn or 'goo' in fn): continue
        return '%s:%d' % (fn, f.f_lineno)

def heapadd(*stuff):
    global heap, dbginfo
    dbginfo.append((len(heap), getdebugname())) # so we know where we came from
    for a in stuff:
        heap.append(I(a))

def finalize(heapaddr=None):
    global heap, sheap
    clear_fwd()
    nheap = heap + sheap
    for i in xrange(4):
        try:
            result = nheap.simplify(heapaddr)
            break
        except NotYetError:
            if i == 3: raise
    
    return result

def heapdump(heap, names=None):
    if names is None: names = {}
    dbginfo.append((10000, '?'))
    reverse = dict((v, k) for (k, v) in names.iteritems())
    stackunktargets = set()
    for i, entry in enumerate(heap.unpack()):
        if 4*i >= dbginfo[0][0]:
            sys.stdout.write('\n%08x %s: ' % (4*i, dbginfo.pop(0)[1].ljust(15)))
        if isinstance(entry, reloc):
            sys.stdout.write('\'%x:' % entry.key)
            entry = entry.value
        if reverse.has_key(entry):
            sys.stdout.write('\x1b[31m%s\x1b[0m ' % reverse[entry])
        else:
            sys.stdout.write('\x1b[34m0x%x\x1b[0m ' % entry)
    sys.stdout.write('\n')

class statue(object):
    def __getstate__(self):
        d = {}
        for s in self.__slots__:
            d[s] = getattr(self, s)
        return d
    def __setstate__(self, state):
        for s in self.__slots__:
            setattr(self, s, state[s])
        

class troll_string(statue):
    __slots__ = ['bits', 'length']
    def __init__(self, s):
        if isinstance(s, troll_string):
            self.bits = s.bits
            self.length = s.length
        elif s == '':
            self.bits = []
            self.length = 0
        else:
            self.bits = [s]
            self.length = len(s)

    def __len__(self):
        return self.length
        
    def __add__(self, other):
        if len(self.bits) == 0: return other
        result = troll_string(self)
        result.append(other)
        return result

    def __radd__(self, other):
        if len(self.bits) == 0: return other
        return troll_string(other) + self

    def append(self, other):
        other = troll_string(other)
        if len(self.bits) > 0 and len(other.bits) > 0 and \
           isinstance(self.bits[-1], str) and \
           isinstance(other.bits[0], str):
            self.bits = self.bits[:-1] + [self.bits[-1] + other.bits[0]] + other.bits[1:]
        else:
            self.bits += other.bits
        self.length += other.length

    def simplify(self, addr):
        r = troll_string('')
        nye = False
        for bit in self.bits:
            try: 
                bit = simplify(bit, addr)
            except NotYetError:
                nye = True
            else:
                r.append(bit)
            addr += len(bit)
        if nye: raise NotYetError
        return r

    def unpack(self):
        bits = []
        for bit in self.bits:
            if isinstance(bit, I_):
                bits.append(bit.sub)
            elif isinstance(bit, str):
                assert len(bit) % 4 == 0
                bits += struct.unpack('I'*(len(bit)/4), bit)
            else:
                raise ValueError('unpack: %r' % bit)
        return bits

class I_(statue):
    __slots__ = ['sub']
    def __init__(self, sub):
        self.sub = sub
    def __len__(self):
        return 4
    def simplify(self, addr):
        return I(simplify(self.sub, addr))
    
def I(x):
    if hasattr(x, '__len__'):
        return x
    elif isinstance(x, (int, long)):
        return struct.pack('I', x)
    else:
        return troll_string(I_(x))
    
def simplify(x, addr):
    if isinstance(x, (int, long, basestring)):
        return x
    else:
        return x.simplify(addr)

def clear_fwd():
    global fwds
    for a in fwds.values():
        a.val = dontcare
    fwds = {}

def exhaust_fwd(*names):
    for name in names:
        if fwds.has_key(name):
            fwds[name].val = dontcare
            del fwds[name]
def set_fwd(name, val):
    assert fwds.has_key(name) and val is not None
    fwds[name].val = val
    del fwds[name]
class fwd:
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
class NotYetError(Exception): pass
class car:
    __slots__ = ['_val', 'addr']
    def simplify(self, addr):
        if not hasattr(self, '_val'):
            self.addr = addr
            self._val = self.val()
        return simplify(self._val, addr)
    def later(self, func):
        return later(lambda addr: func(self.simplify(addr)))
    def __add__(self, other):
        return self.later(lambda s: s + other)
    def __sub__(self, other):
        return self.later(lambda s: s - other)
    def __radd__(self, other):
        return self.later(lambda s: other + s)
    def __rsub__(self, other):
        return self.later(lambda s: other - s)
    def __mul__(self, other):
        return self.later(lambda s: s * other)
    def __rmul__(self, other):
        return self.later(lambda s: other * s)
    def __and__(self, other):
        return self.later(lambda s: s & other)

class later(car):
    __slots__ = ['func']
    def __init__(self, func):
        self.func = func
    def val(self):
        return self.func(self.addr)

# [0] evaluates to 0 (initially), [1] evaluates to the address of [0]
def stackunkpair():
    unk = pointed('\0\0\0\0')
    unkptr = pointer(unk)
    return unk, unkptr


class pointed(object): # string-like
    __slots__ = ['sub', 'addr']
    def __init__(self, sub):
        self.sub = sub
        self.addr = None
    def simplify(self, addr):
        #print 'pointed.simplify self=%r addr=%r' % (self, addr)
        self.addr = addr
        return simplify(self.sub, addr)
    def __len__(self):
        return len(self.sub)

class pointer(car): # int-like
    __slots__ = ['sub']
    def __init__(self, sub):
        self.sub = sub
    def simplify(self, addr):
        #print 'pointer.simplify sub=%r addr=%r' % (self.sub, addr)
        return car.simplify(self, addr)
    def val(self):
        addr = self.sub.addr
        if addr is None:
            raise NotYetError
        #print 'pointer.val sub=%r addr=%r' % (self.sub, addr)
        return addr

def ptr(str, null_terminate=False):
    global sheap
    sheap.append('\0' * (-len(sheap) & 3))
    result = pointed(str)
    sheap.append(result)
    if null_terminate: sheap.append('\0')
    return pointer(result)

def ptrI(*xs):
    return ptr(reduce(operator.add, map(I, xs)))

class reloc:
    def __init__(self, key, value, alignment=1):
        self.key = key
        self.value = value
        self.alignment = alignment

    # limited set of operations - for others, the reloc wouldn't make sense
    def __add__(self, other):
        return reloc(self.key, self.value + other)
    def __sub__(self, other):
        return reloc(self.key, self.value - other)
    def __radd__(self, other):
        return reloc(self.key, other + self.value)
    def __mod__(self, other):
        return reloc(self.key, self.value % other)
    def __and__(self, other):
        mask = self.alignment - 1
        if (other & ~mask) == 0:
            return self.value & other
        elif (other & mask) == 0:
            return reloc(self.key, self.value & other)
        else:
            raise Exception('reloc & %r, but not page aligned' % other)

    def simplify(self, addr):
        if isinstance(self.value, (int, long)):
            return self
        else:
            return reloc(self.key, simplify(self.value, addr), self.alignment)

    def __repr__(self):
        return 'reloc(0x%x, 0x%x, 0x%x)' % (self.key, self.value, self.alignment)
