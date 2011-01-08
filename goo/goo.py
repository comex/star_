import struct, sys, os, re, anydbm, traceback
import warnings
warnings.simplefilter('error')

#os.chdir(os.path.dirname(os.path.abspath(sys.argv[0])))
dontcare = 0

def init(*regs):
    global fwds, heapstuff
    fwds = {}
    heapstuff = map(fwd, regs)

dbginfo = []

def heapadd(*stuff):
    global heapstuff
    # so we know where we came from
    dbginfo.append((len(heapstuff), sys._getframe().f_back.f_code.co_name))
    for a in stuff:
        if hasattr(a, 'added'): a.added()
        heapstuff.append(a)

def finalize(heapaddr_=None):
    global heapstuff, sheap, sheapaddr, heapaddr, heapdbgnames
    clear_fwd()
    heapaddr._val = heapaddr_
    if heapaddr_ is not None:
        sheapaddr = heapaddr_ + 4*len(heapstuff)
        print hex(sheapaddr)
    sheap = ''
    heapdbgnames = [(obj.name if hasattr(obj, 'name') else None) for obj in heapstuff]
    heapstuff = map(int, heapstuff)
#    for pass_num in xrange(2):
#        for hidx in xrange(len(heapstuff)):
#            thing = heapstuff[hidx]
#            if not isinstance(thing, int):
#                try:
#                    thing = int(thing)
#                except NotYetError:
#                    pass
#                else:
#                    heapstuff[hidx] = thing

    return struct.pack('<'+'I'*len(heapstuff), *heapstuff) + sheap

def heapdump(names=None):
    if names is None: names = {}
    dbginfo.append((10000, '?'))
    reverse = dict((v, k) for (k, v) in names.iteritems())
    stackunktargets = set()
    for i, entry in enumerate(heapstuff):
        if i >= dbginfo[0][0]:
            print
            print '%08x %s:' % (heapaddr._val + 4*i, dbginfo.pop(0)[1].ljust(15)),
        if heapdbgnames[i] is not None:
            sys.stdout.write(' %s=' % heapdbgnames[i])
        if heapaddr._val <= entry <= heapaddr._val + 4*len(heapstuff):
            stackunktargets.add((entry - heapaddr._val)/4)
            print hex(entry),
        elif i in stackunktargets:
            print '\x1b[34m\x1b[1m%s\x1b[0m' % hex(entry),
        elif reverse.has_key(entry):
            print '\x1b[31m%s\x1b[0m' % reverse[entry],
        else:
            print '\x1b[34m%s\x1b[0m' % hex(entry),
    print
    

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
    def __init__(self, name, force=False):
        if force:
            if fwds.has_key(name): exhaust_fwd(name)
        else:
            assert not fwds.has_key(name)
        fwds[name] = self
        self.name = name
        self.val = None
    def __int__(self):
        assert self.val is not None
        return int(self.val)
    def __repr__(self):
        return '<fwd: %s>' % self.val
class NotYetError(Exception): pass
class car:
    def __int__(self):
        if not hasattr(self, '_val'):
            self._val = int(self.val())
        return int(self._val)
    def __add__(self, other):
        return later(lambda: (int(self) + int(other)) % (2**32))
    def __sub__(self, other):
        return later(lambda: (int(self) - int(other)) % (2**32))
    def __radd__(self, other):
        return later(lambda: (int(other) - int(self)) % (2**32))
    def __rsub__(self, other):
        return later(lambda: (int(other) - int(self)) % (2**32))
    def __mul__(self, other):
        return later(lambda: (int(self) * int(other)) % (2**32))
    def __rmul__(self, other):
        return later(lambda: (int(other) * int(self)) % (2**32))
    def __and__(self, other):
        return later(lambda: (int(self) & int(other)) % (2**32))
    def __len__(self):
        return int(self)

class later(car):
    def __init__(self, func):
        self.func = func
    def val(self):
        return self.func()

class stackunk(car):
    def added(self):
        self.addr = heapaddr + 4 * len(heapstuff)
    def val(self):
        return 0
class stackunkptr(car):
    def __init__(self, unk, name=None):
        self.unk = unk
        self.name = name
    def val(self):
        if not hasattr(self.unk, 'addr'):
            raise NotYetError(self.name)
        return self.unk.addr
# [0] evaluates to 0 (initially), [1] evaluates to the address of [0]
def stackunkpair():
    unk = stackunk()
    name = 'line %d' % sys._getframe().f_back.f_lineno
    unkptr = stackunkptr(unk, name)
    return unk, unkptr

class ptrI(car):
    def __init__(self, *args):
        self.args = args
    def val(self):
        global sheapaddr, sheap
        self.args = map(int, self.args)
        q = struct.pack('I'*len(self.args), *self.args)
        ret = sheapaddr + len(sheap)
        sheap += q
        while len(sheap) % 4 != 0: sheap += '\0'
        return ret
    
class ptr(car):
    def __init__(self, str, null_terminate=False):
        self.str = str
        self.null_terminate = null_terminate
    def __len__(self):
        return len(self.str)
    def val(self):
        global sheapaddr, sheap
        ret = sheapaddr + len(sheap)
        sheap += self.str
        if self.null_terminate: sheap += '\0'
        sheap += '\0' * (-len(sheap) & 3)
        return ret

class marker(car):
    def __init__(self):
        assert heapaddr is not None
    def mark(self):
        self._val = heapaddr + 4 * len(heapstuff)
        return self
    def val(self):
        raise ValueError("marker didn't mark")

heapaddr = car() # finalize fills this in
