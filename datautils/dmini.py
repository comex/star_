import os, re, sys, atexit
import cPickle, shelve # for cache
from ctypes import *

MUST_FIND, TO_EXECUTE, PRIVATE_SYM, IMPORTED_SYM, EXTEND_RANGE = 1, 2, 4, 8, 16

data_dir = os.path.join(os.path.dirname(__file__), '../data')
if sys.platform == 'darwin':
    datar = CDLL(data_dir + '/universal/libdata.dylib')
else:
    datar = CDLL(data_dir + '/native/libdata.so')

class DataSegment(Structure):
    pass

class DataSym(Structure):
    _fields_ = [('name', c_char_p),
                ('address', c_uint32)]

class Binary(Structure):
    _fields_ = [('valid', c_bool),
                ('segments', POINTER(DataSegment)),
                ('nsegments', c_uint32),
                ('actual_cpusubtype', c_int),
                ('the_rest', c_char * 200)]

class Range(Structure):
    _fields_ = [('binary', POINTER(Binary)),
                ('start', c_uint32),
                ('size', c_size_t)]
    def data(self):
        return data.rangeconv(self).data()
    def data_as(self, typ):
        return cast(data.rangeconv(self).start, POINTER(typ))
    def __repr__(self):
        return 'Range(%r, %x, %x)' % (self.binary, self.start, self.size)

class Prange(Structure):
    _fields_ = [('start', c_void_p),
                ('size', c_size_t)]

    def data(self):
        return string_at(self.start, self.size)

DataSegment._fields_ = [('file_range', Range),
                        ('vm_range', Range),
                        ('native_segment', c_void_p)]

datar.b_init.argtypes = [POINTER(Binary)]
datar.b_load_dyldcache.argtypes = [POINTER(Binary), c_char_p]
datar.b_load_macho.argtypes = [POINTER(Binary), c_char_p]
datar.b_dyldcache_load_macho.argtypes = [POINTER(Binary), c_char_p, POINTER(Binary)]
datar.b_find_data_anywhere.argtypes = [POINTER(Binary), c_char_p, c_int, c_int]
datar.b_find_data_anywhere.restype = c_uint32
datar.find_data.argtypes = [Range, c_char_p, c_int, c_int]
datar.find_data.restype = c_uint32
datar.b_sym.argtypes = [POINTER(Binary), c_char_p, c_int]
datar.b_sym.restype = c_uint32
datar.b_macho_segrange.argtypes = [POINTER(Binary), c_char_p]
datar.b_macho_segrange.restype = Range
datar.b_macho_sectrange.argtypes = [POINTER(Binary), c_char_p, c_char_p]
datar.b_macho_sectrange.restype = Range
datar.rangeconv.argtypes = [Range]
datar.rangeconv.restype = Prange
datar.b_relocate.argtypes = [POINTER(Binary), POINTER(Binary), c_int, c_void_p, c_uint32]
datar.b_copy_syms.argtypes = [POINTER(Binary), POINTER(POINTER(DataSym)), POINTER(c_uint32), c_int]
datar.data_call_init.argtypes = [c_void_p]
datar.data_call_fini.restype = c_char_p
datar.find_bof.argtypes = [Range, c_uint32, c_int]
datar.find_bof.restype = c_uint32
        
class DataError(Exception):
    pass

class DataWrapper:
    def __getattr__(self, attr):
        old = getattr(datar, attr)
        def func(*args):
            datar.data_call_init(old)
            datar.data_call.argtypes = old.argtypes
            datar.data_call.restype = old.restype
            result = datar.data_call(*args)
            err = datar.data_call_fini()
            if err:
                raise DataError(err.rstrip())
            else:
                return result
        return func
#data = DataWrapper() 
data = datar

class DminiError(Exception):
    pass

def cached(func):
    def f2(self, *args):
        cache_key = cPickle.dumps((self.path, self.mtime, func.__name__,) + args)
        g = self.cache.get(cache_key, None)
        if g is None:
            g = self.cache[cache_key] = func(self, *args)
        return self.wrap(g)
    return f2

class Connection:
    def __init__(self, path, is_cache=False, use_private_sym=False, rw=False):
        self.path = path
        self.mtime = os.path.getmtime(path)
        self.stack = []
        self.cache = {}#shelve.open('/tmp/dmini-cache')
        self.binaries = {}
        self.use_private_sym = use_private_sym

        self.binary = Binary()
        self.binary.actual_cpusubtype = 42
        data.b_init(byref(self.binary))
        self.is_cache = is_cache
        if is_cache:
            data.b_load_dyldcache(byref(self.binary), path, rw)
            self.add_lib('libsystem', '/usr/lib/libSystem.B.dylib')
        else:
            data.b_load_macho(byref(self.binary), path, rw)

    def close(self):
        pass

    def add_lib(self, short, lib):
        assert not self.binaries.has_key(short)
        self.binaries[short] = Binary()
        data.b_dyldcache_load_macho(byref(self.binary), lib, byref(self.binaries[short]))
    
    def add_43_lib(self, short, lib):
        self.add_lib(short, lib)

    def explode_name(self, name):
        if self.is_cache:
            short = 'libsystem'
            if '.' in name:
                short, name = name.split('.')
            return self.binaries[short], name
        return self.binary, name
    
    @cached
    def find(self, thing):
        if isinstance(thing, tuple):
            thing = thing[self.binary.actual_cpusubtype != 9]
        result = data.b_find_data_anywhere(byref(self.binary), thing, 2 if thing.find('+') != -1 else 4, 0)
        if result == 0:
            raise DataError("didn't find [%s]" % thing)
        return result

    @cached
    def sym(self, name, type='normal'):
        if type == 'normal' and self.use_private_sym: type = 'private'
        binary, name = self.explode_name(name)
        flags = {'normal': 0, 'private': PRIVATE_SYM, 'imported': IMPORTED_SYM}
        result = data.b_sym(byref(binary), name, TO_EXECUTE | flags[type])
        if result == 0:
            print 'For reference', self._syms(binary, type)
            raise DataError('no such sym %s' % name)
        return result

    def private_sym(self, name):
        changeme

    def syms(self, b='libsystem'):
        return self._syms(self.binaries[b] if self.is_cache else self.binary)

    def _syms(self, binary, type='normal'):
        syms = POINTER(DataSym)()
        nsyms = c_uint32()
        data.b_copy_syms(binary, byref(syms), byref(nsyms), TO_EXECUTE | {'normal': 0, 'private': PRIVATE_SYM}[type])
        result = {}
        for i in xrange(nsyms.value):
            result[syms[i].name] = syms[i].address
        return result

    def wrap(self, num):
        return num

    def relocate(self, target, slide, my_ls=None):
        @CFUNCTYPE(c_uint32, POINTER(Binary), c_char_p)
        def lookup_sym(binary, sym):
            if my_ls is not None:
                result = my_ls(binary, sym)
                if result is not None:
                    return result
            return data.b_sym(binary, sym, TO_EXECUTE | MUST_FIND)

        data.b_relocate(self.binary, target.binary, 0, lookup_sym, slide)
        return self
        
    def nth_segment(self, n):
        if n >= self.binary.nsegments: raise DataError
        return self.binary.segments[n].vm_range

    def segrange(self, segname):
        return b_macho_segrange(self.binary, segname)
    
    def sectrange(self, segname, sectname):
        return b_macho_sectrange(self.binary, segname, sectname)


cur = None

def init(*args):
    global cur
    if cur is not None: cur.close()
    cur = Connection(*args)
