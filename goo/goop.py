import struct, sys, os

def getdebugname():
    f = sys._getframe().f_back
    while True:
        f = f.f_back
        fn = os.path.basename(f.f_code.co_filename)
        if True and ('world' in fn or 'goo' in fn): continue
        return '%s:%d' % (fn, f.f_lineno)


def simplify_times(heap, addr, times, must_be_simple=True):
    for i in xrange(times):
        heap = simplify(heap, addr)
    if must_be_simple and not is_simple(heap):
        print heap
        raise Exception('simplify_times: not simple')
    return heap

def is_simple(heap):
    if isinstance(heap, (basestring, long, int, reloc)):
        return True
    elif isinstance(heap, I_):
        return is_simple(heap.sub)
    elif isinstance(heap, troll_string):
        return all(map(is_simple, heap.bits))
    else:
        return False

# __slots__ objects require __getstate__
# why does this silly fix work?
class statue(object):
    __slots__ = []
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
    def __init__(self, s=''):
        if isinstance(s, troll_string):
            self.bits = list(s.bits)
            self.length = s.length
        elif s == '':
            self.bits = []
            self.length = 0
        else:
            self.bits = [s]
            self.length = len(s)

    def len(self):
        if self.length is None:
            total = 0
            for bit in self.bits:
                l = len(bit)
                if l is None:
                    total = None
                elif total is not None:
                    total += l
            self.length = total
            return total
        return self.length

    def __len__(self):
        result = self.len()
        if result is not None:
            return result
        else:
            surrogate = troll_string(self)
            return later(lambda addr: surrogate.len())
        
    def __add__(self, other):
        if len(self.bits) == 0: return other
        result = troll_string(self)
        result.append(other)
        if len(result.bits) == 1: result = result.bits[0]
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
        if other.length is None:
            self.length = None
        elif self.length is not None:
            self.length += other.length

    def simplify(self, addr):
        r = troll_string('')
        addr1 = addr
        for bit in self.bits:
            new = simplify(bit, addr1)
            if addr1 is not None:
                length = len(new)
                if length is not None:
                    addr1 += length
            r.append(new)
        self.length = r.length
        self.bits = r.bits
        if len(self.bits) == 1:
            return self.bits[0]
        else:
            return self

    def unpack(self):
        bits = []
        for bit in self.bits:
            if isinstance(bit, I_):
                bits.append(bit.sub)
            elif isinstance(bit, str):
                assert len(bit) % 4 == 0
                bits += struct.unpack('I'*(len(bit)/4), bit)
            elif len(bit) == 0:
                pass
            else:
                raise ValueError('unpack: %r' % bit)
        return bits

    def __getslice__(self, i, j):
        if j < i: return ''
        addr = 0
        slic = troll_string('')
        for bit in self.bits:
            length = len(bit)
            assert length is not None
            if i < addr + length:
                if j < addr:
                    pass
                elif i <= addr and j >= addr + length:
                    slic.append(bit)
                else:
                    slic.append(bit[max(i-addr, 0):min(j-addr, length)]) 

            addr += length
            if addr >= j: break
        return slic

    def __setslice__(self, i, j, seq):
        new = troll_string(self[:i] + seq + self[j:])
        self.bits = new.bits
        self.length = new.length

    def __repr__(self):
        return '<troll_string (%r): %s>' % (self.length, self.bits)

def len(x):
    return x.__len__()

def simplify(x, addr=None):
    if isinstance(x, (int, long, basestring)):
        return x
    else:
        return x.simplify(addr)

class later_s_(statue):
    __slots__ = ['func', 'length', '_val']
    def __init__(self, func, length=None):
        self.func = func
        self.length = None
        self._val = None

    def __len__(self):
        return self.length

    def simplify(self, addr):
        if self._val is None:
            val = self.func(addr)
            self.length = len(val)
            self._val = simplify(val, addr)
        return self._val

def later_s(*args, **kwargs):
    return troll_string(later_s_(*args, **kwargs))

class I_(statue):
    __slots__ = ['sub']
    def __init__(self, sub):
        self.sub = sub
    def __len__(self):
        return 4
    def simplify(self, addr):
        return I(simplify(self.sub, addr))
    def __repr__(self):
        return '<I: %r>' % (self.sub,)
    
def I(*args):
    result = ''
    for arg in args:
        assert arg is not None
        if hasattr(arg, '__len__'):
            result += arg
        elif isinstance(arg, (int, long)):
            result += struct.pack('I', arg)
        else:
            result += troll_string(I_(arg))
    return result

class car(statue):
    __slots__ = ['_val', 'addr']
    def simplify(self, addr):
        if not hasattr(self, '_val'):
            self.addr = addr
            v = self.val()
            if v is None: return self
            self._val = v
        return simplify(self._val, addr)
    def later(self, other, func):
        def f(addr):
            s = self.simplify(addr)
            o = simplify(other, addr)
            if s is not self or o is not other:
                return func(s, o)
            else:
                return None
        return later(f)
    def __add__(self, other):
        return self.later(other, lambda s, o: s + o)
    def __sub__(self, other):
        return self.later(other, lambda s, o: s - o)
    def __radd__(self, other):
        return self.later(other, lambda s, o: o + s)
    def __rsub__(self, other):
        return self.later(other, lambda s, o: o - s)
    def __mul__(self, other):
        return self.later(other, lambda s, o: s * o)
    def __rmul__(self, other):
        if isinstance(other, basestring):
            return later_s(lambda addr: other * self.simplify(addr))
        return self.later(other, lambda s, o: o * s)
    def __and__(self, other):
        return self.later(other, lambda s, o: s & o)
    def __neg__(self):
        return later(lambda addr: -self.simplify(addr))

class later(car):
    __slots__ = ['func']
    def __init__(self, func):
        self.func = func
    def val(self):
        return self.func(self.addr)

class pointed_(statue): # string-like
    __slots__ = ['sub', 'addr']
    def __init__(self, sub):
        self.sub = sub
        self.addr = None
    def simplify(self, addr):
        #print 'pointed.simplify self=%r addr=%r' % (self, addr)
        if addr is not None:
            self.addr = addr
            return simplify(self.sub, addr)
        else:
            return self
    def __len__(self):
        return len(self.sub)
    def __repr__(self):
        return '<pointed: %r>' % self.sub

def pointed(*args, **kwargs):
    return troll_string(pointed_(*args, **kwargs))

class pointer(car): # int-like
    __slots__ = ['sub']
    def __init__(self, sub):
        assert isinstance(sub.bits[0], pointed_)
        self.sub = sub
    def val(self):
        addr = self.sub.bits[0].addr
        if addr is None: return None
        return addr
    def __repr__(self):
        return '<pointer: ...>'
        #return '<pointer: %r>' % self.sub

reloc_handlers = {}

class reloc:
    def __init__(self, key, value, alignment=1):
        self.key = key
        self.value = value
        self.alignment = alignment

    # limited set of operations - for others, the reloc wouldn't make sense
    def __add__(self, other):
        return reloc(self.key, self.value + other)
    def __sub__(self, other):
        if isinstance(other, reloc) and self.key == other.key:
            return reloc(self.key, self.value - other.value)
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
        if reloc_handlers.has_key(self.key):
            return reloc_handlers[self.key](self.value, addr)
        elif isinstance(self.value, (int, long)):
            return self
        else:
            return reloc(self.key, simplify(self.value, addr), self.alignment)

    def __repr__(self):
        return 'reloc(0x%x, 0x%x, 0x%x)' % (self.key, self.value, self.alignment)



# for debugging!
if False:
    failures = set()
    def simplify(x, addr):
        if isinstance(x, (int, long, basestring)):
            return x
        else:
            result = x.simplify(addr)
            if result is x and not isinstance(x, (troll_string, reloc)):
                failures.add(x)
            elif x in failures:
                failures.remove(x)
            return result

    def excepthook(typ, value, traceback):
        global failures
        sys.__excepthook__(typ, value, traceback)
        print 'The failures were:'
        failures = list(failures)
        for f in failures:
            print f
            print '--'
        import code
        code.interact(local=globals())
        
    sys.excepthook = excepthook

    def hook_init(cls):
        class newclass(cls):
            def __init__(self, *args, **kwargs):
                self.debugname = getdebugname()
                cls.__init__(self, *args, **kwargs)
            def __repr__(self):
                return '"%s"%s' % (self.debugname, cls.__repr__(self))
        newclass.__name__ = cls.__name__
        return newclass
    later_s_ = hook_init(later_s_)
    later = hook_init(later)
    pointed_ = hook_init(pointed_)
