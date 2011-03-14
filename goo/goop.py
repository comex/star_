import struct, sys, os

def getdebugname():
    f = sys._getframe().f_back
    while True:
        f = f.f_back
        fn = os.path.basename(f.f_code.co_filename)
        if True and ('world' in fn or 'goo' in fn): continue
        return '%s:%d' % (fn, f.f_lineno)


def simplify_times(heap, addr, times):
    for i in xrange(times):
        try:
            return simplify(heap, addr)
        except NotYetError:
            if i == times - 1: raise

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
            try:
                self.length = len(s)
            except NotYetError:
                self.length = None

    def len(self):
        if self.length is None:
            total = 0
            for bit in self.bits:
                try:
                    l = len(bit)
                except NotYetError:
                    total = None
                else:
                    if total is not None: total += l
            if total is None: raise NotYetError
            self.length = total
            return total
        return self.length

    def __len__(self):
        try:
            return self.len()
        except NotYetError:
            return later(lambda addr: self.len())
        
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
        nye = False
        addr1 = addr
        for bit in self.bits:
            try: 
                bit = simplify(bit, addr1)
            except NotYetError:
                nye = True
            else:
                r.append(bit)
                addr += len(bit)
            if addr1 is not None:
                try:
                    addr1 += len(bit)
                except NotYetError:
                    nye = True
                    addr1 = None
        if addr1 is not None: self.length = addr1 - addr
        if nye: raise NotYetError
        if len(r.bits) == 1:
            return r.bits[0]
        else:
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

    def __repr__(self):
        return '<troll_string (%r): %s>' % (self.length, self.bits)

def len(x):
    return x.__len__()

def simplify(x, addr):
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
        if self.length is None:
            raise NotYetError
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
        if hasattr(arg, '__len__'):
            result += arg
        elif isinstance(arg, (int, long)):
            result += struct.pack('I', arg)
        else:
            result += troll_string(I_(arg))
    return result

class NotYetError(Exception): pass
class car(statue):
    __slots__ = ['_val', 'addr']
    def simplify(self, addr):
        print 'self=%r' % self
        if addr is not None and not hasattr(self, '_val'):
            self.addr = addr
            self._val = self.val()
            assert self._val is not None
        return simplify(self._val, addr)
    def later(self, other, func):
        return later(lambda addr: func(self.simplify(addr), simplify(other, addr)))
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
    def __len__(self):
        return len(self.sub)

def pointed(*args, **kwargs):
    return troll_string(pointed_(*args, **kwargs))

class pointer(car): # int-like
    __slots__ = ['sub']
    def __init__(self, sub):
        self.sub = sub
    def val(self):
        addr = self.sub.bits[0].addr
        if addr is None:
            raise NotYetError
        return addr

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

failures = set()
def simplify(x, addr):
    if isinstance(x, (int, long, basestring)):
        return x
    else:
        try:
            result = x.simplify(addr)
        except NotYetError:
            if not isinstance(x, troll_string): failures.add(x)
            raise
        else:
            if x in failures: failures.remove(x)
            return result

def excepthook(typ, value, traceback):
    sys.__excepthook__(typ, value, traceback)
    print 'The failures were:'
    fail = list(failures)
    for f in fail:
        print f
        print '--'
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
