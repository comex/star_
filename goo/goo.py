import struct, sys, os, re, anydbm, traceback
import warnings
import operator
warnings.simplefilter('error')

#os.chdir(os.path.dirname(os.path.abspath(sys.argv[0])))
dontcare = 0

def init(*regs):
    global fwds, heapstuff, heapaddr, dbginfo
    fwds = {}
    heapstuff = map(fwd, regs)
    heapaddr = car() # finalize fills this in
    dbginfo = []

def heapadd(*stuff):
    global heapstuff
    # so we know where we came from
    dbginfo.append((len(heapstuff), sys._getframe().f_back.f_code.co_name))
    for a in stuff:
        heapstuff.append(a)

def finalize(heapaddr_=None):
    global heapstuff, sheap, sheapaddr, heapaddr, heapdbgnames
    clear_fwd()
    heapaddr._val = heapaddr_
    if heapaddr_ is not None:
        sheapaddr = heapaddr_ + 4*len(heapstuff)
        #print hex(sheapaddr)
    sheap = ''
    heapdbgnames = [(obj.name if hasattr(obj, 'name') else None) for obj in heapstuff]
    for pass_num in xrange(2):
        for hidx in xrange(len(heapstuff)):
            thing = heapstuff[hidx]
            try:
                thing = simplify(thing)
            except NotYetError:
                if pass_num == 1:
                    raise Exception("%r doesn't want to be an int" % thing)
            else:
                heapstuff[hidx] = thing
    hidx = None

    text = struct.pack('<'+'I'*len(heapstuff), *heapstuff) + sheap
    return text

def heapdump(names=None):
    if names is None: names = {}
    dbginfo.append((10000, '?'))
    reverse = dict((v, k) for (k, v) in names.iteritems())
    stackunktargets = set()
    for i, entry in enumerate(heapstuff):
        if i >= dbginfo[0][0]:
            sys.stdout.write('\n%08x %s: ' % (int(heapaddr._val + 4*i), dbginfo.pop(0)[1].ljust(15)))
        if heapdbgnames[i] is not None:
            sys.stdout.write('%s=' % heapdbgnames[i])
        if all_relocs.has_key(i*4):
            sys.stdout.write('r%x:' % all_relocs[i*4])
        if heapaddr._val <= entry <= heapaddr._val + 4*len(heapstuff):
            stackunktargets.add((entry - heapaddr._val)/4)
            sys.stdout.write('0x%x ' % entry)
        elif i in stackunktargets:
            sys.stdout.write('\x1b[34m\x1b[1m0x%x\x1b[0m ' % entry)
        elif reverse.has_key(entry):
            sys.stdout.write('\x1b[31m%s\x1b[0m ' % reverse[entry])
        else:
            sys.stdout.write('\x1b[34m0x%x\x1b[0m ' % entry)
    sys.stdout.write('\n')

class troll_string(object):
    __slots__ = ['bits', 'length']
    def __init__(self, s):
        if isinstance(s, troll_string):
            self.bits = s.bits
            self.length = s.length
        else:
            self.bits = [s]
            self.length = len(S)
        
    def __add__(self, other):
        result = troll_string(other)
        if len(result.bits) > 0 and len(other.bits) > 0 and \
           isinstance(result.bits[-1], str) and \
           isinstance(other.bits[0], str):
           result.bits = self.bits[:-1] + [result.bits[-1] + other.bits[0]] + other.bits[1:]
        else:
            result.bits = self.bits + result.bits
        result.length = self.length + result.length
        return result

    def __radd__(self, other):
        return troll_string(other) + self

    def simplify(self):
        return troll_string(map(simplify, self.bits), self.length)

class I_(object):
    __slots__ = ['sub']
    def __init__(self, sub):
        self.sub = sub
    def __len__(self):
        return 4
    def simplify(self):
        return I(simplify(self.sub))
    
def I(x):
    x = simplify(x)
    if isinstance(x, (int, long)):
        return struct.pack('I', x)
    else:
        return I_(x)
    
def simplify(x):
    if isinstance(x, (int, long)):
        return x
    else:
        return x.simplify()

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
    def simplify(self):
        assert self.val is not None
        return simplify(self.val)
    def __repr__(self):
        return '<fwd: %s>' % self.val
class NotYetError(Exception): pass
class car:
    __slots__ = ['_val']
    def simplify(self):
        if not hasattr(self, '_val'):
            self._val = self.val()
        return simplify(self._val)
    def __add__(self, other):
        return later(lambda: simplify(self) + other)
    def __sub__(self, other):
        return later(lambda: simplify(self) - other)
    def __radd__(self, other):
        return later(lambda: other - simplify(self))
    def __rsub__(self, other):
        return later(lambda: other - simplify(self))
    def __mul__(self, other):
        return later(lambda: simplify(self) * other)
    def __rmul__(self, other):
        return later(lambda: other * simplify(self))
    def __and__(self, other):
        return later(lambda: simplify(self) & other)
    def __len__(self):
        return int(self)

class later(car):
    __slots__ = ['func']
    def __init__(self, func):
        self.func = func
    def val(self):
        return self.func()

class stackunk(car):
    __slots__ = ['addr']
    def val(self):
        self.addr = heapaddr + 4 * hidx
        return 0
class stackunkptr(car):
    __slots__ = ['unk', 'name']
    def __init__(self, unk, name=None):
        self.unk = unk
        self.name = name
    def val(self):
        if not hasattr(self.unk, 'addr'):
            raise NotYetError(self.name)
        return self.unk.addr
    def __repr__(self):
        addr = hex(self.unk.addr) if hasattr(self.unk, 'addr') else '(no address)'
        return '<stackunkptr "%s": %s>' % (self.name, addr)
# [0] evaluates to 0 (initially), [1] evaluates to the address of [0]
def stackunkpair():
    unk = stackunk()
    name = 'line %d' % sys._getframe().f_back.f_lineno
    unkptr = stackunkptr(unk, name)
    return unk, unkptr

class ptr(car):
    def __init__(self, str, null_terminate=False):
        self.str = str
        self.null_terminate = null_terminate
    def __len__(self):
        return len(self.str)
    def val(self):
        global sheapaddr, sheap
        sheap += '\0' * (-len(sheap) & 3)
        ret = sheapaddr + len(sheap)
        sheap += self.str
        if self.null_terminate: sheap += '\0'
        return ret

def ptrI(*xs):
    print map(I, xs)
    return ptr(reduce(operator.add, map(I, xs)))

class marker(car):
    def __init__(self):
        assert heapaddr is not None
    def mark(self):
        self._val = heapaddr + 4 * len(heapstuff)
        return self
    def val(self):
        raise ValueError("marker didn't mark")

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

    def simplify(self):
        if isinstance(self.value, (int, long)):
            return self
        else:
            return reloc(self.key, simplify(self.value), self.alignment)

    def __repr__(self):
        return 'reloc(0x%x, 0x%x, 0x%x)' % (self.key, self.value, self.alignment)
