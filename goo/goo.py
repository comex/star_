#!/opt/local/bin/python2.6
import struct, sys, os, re, anydbm, traceback
import warnings
warnings.simplefilter('error')

#os.chdir(os.path.dirname(os.path.abspath(sys.argv[0])))
dontcare = 0

heapdbg = False

def init(*regs):
    global fwds, heapstuff
    fwds = {}
    heapstuff = map(fwd, regs)

def heapadd(*stuff):
    global heapstuff
    if heapdbg:
        heapstuff.append(''.join(traceback.format_stack()))
    heapstuff += list(stuff)

def finalize(heapaddr_=None):
    global heapstuff, hidx, sheap, sheapaddr, heapaddr
    clear_fwd()
    heapaddr = heapaddr_
    if heapaddr is not None:
        sheapaddr = heapaddr + 4*len(heapstuff)
        print 'sheapaddr = %x'% sheapaddr
    sheap = ''
    for pass_num in xrange(2):
        for hidx in xrange(len(heapstuff)):
            thing = heapstuff[hidx]
            if heapdbg and isinstance(thing, basestring): continue
            if not isinstance(thing, int):
                try:
                    thing = int(thing)
                except NotYetError:
                    pass
                else:
                    heapstuff[hidx] = thing
    if heapdbg:
        for i in heapstuff:
            if isinstance(i, basestring):
                print i
            else:
                print hex(i)
        sys.exit(1)

    return struct.pack('<'+'I'*len(heapstuff), *heapstuff) + sheap

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
        return self._val
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
class sp_off(car):
    def val(self):
        return 4*hidx

class stackunkwrapper(car):
    def __init__(self, wrapped):
        self.wrapped = wrapped
    def val(self):
        global heapaddr
        self.addr = heapaddr + 4*hidx
        return int(self.wrapped)
class stackunk(car):
    def val(self):
        global heapaddr
        self.addr = heapaddr + 4*hidx
        return 0
class stackunkptr(car):
    def __init__(self, unk):
        self.unk = unk
    def val(self):
        if not hasattr(self.unk, 'addr'): raise NotYetError
        return self.unk.addr
# [0] evaluates to 0, [1] evaluates to the address of [0]
def stackunkpair():
    unk = stackunk()
    unkptr = stackunkptr(unk)
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
        while len(sheap) % 4 != 0: sheap += '\0'
        return ret

class later(car):
    def __init__(self, func):
        self.func = func
    def val(self):
        return self.func()



