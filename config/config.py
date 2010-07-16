#!/opt/local/bin/python2.6
import struct, re, subprocess, time, shelve, hashlib, cPickle, os, sys, plistlib, optparse, mmap, anydbm
try:
    import json
except:
    import simplejson as json

# todo: may as well merge it with kv :/
try:
    import pyximport; pyximport.install()
    import confighelper
except:
    print >> sys.stderr, '(Cython couldn\'t be imported; using slow confighelper)'
    import iconfighelper as confighelper

if __name__ == '__main__':
    basepath = os.path.realpath(os.path.dirname(sys.argv[0]))
    os.chdir(basepath)
    data = eval('{%s}' % open('configdata.py').read())
else:
    basepath = os.path.realpath(os.path.dirname(__file__))
cache = shelve.open(basepath + '/config.cache')

## syms
# why do I have so many ways of getting syms?

class gdb_symmer:
    def __init__(self, fn):
        self.symdict = None
        import popen2
        self.i, self.o = popen2.popen2(['/usr/bin/gdb', fn])
        self.o.write('start\n')
        for framework in ['/System/Library/Frameworks/IOKit.framework/IOKit', '/System/Library/PrivateFrameworks/IOSurface.framework/IOSurface']:
            self.o.write('call (int) dlopen("%s", 0)\r\n' % framework)
        self.o.flush()
        
    def __getitem__(self, thing):
        print 'getitem:', thing
        self.o.write('print/x &%s\n' % thing[1:])
        self.o.flush()
        while True:
            line = self.i.readline()
            print line
            m = re.search('= 0x(.{8})', line)
            if m:
                ret = int(m.group(1), 16)
                break
        self.o.write('disas %s %s+4\n' % (thing[1:], thing[1:]))
        self.o.write('print 57005\n')
        self.o.flush()
        while True:
            line = self.i.readline()
            thumb = False
            if '57005' in line:
                print 'done.'
                if thumb: ret |= 1
                return ret
            elif '+2>' in line:
                thumb = True
class anydbm_symmer:
    def __init__(self, fn):
        self.db = anydbm.open(fn)
    def __getitem__(self, val):
        addr = self.db[val]
        if isinstance(addr, basestring): addr = struct.unpack('I', addr)[0]
        return addr

        
def get_syms(d):
    if not d.has_key('@syms'): return None
    fn = d['@syms']
    if fn is None:
        return macho_load(d['@binary']).get_syms()
    elif fn.startswith('gdb:'):
        return gdb_symmer(fn[4:])
    else:
        return anydbm_symmer(fn)

## Mach-O

def do_symstring(syms, v):
    name = v[1:]
    offs = 0
    z = name.find('+')
    if z != -1:
        offs = eval(name[z+1:])
        name = name[:z]
    addr = syms[name]
    # Data, so even a thumb symbol shouldn't be &1
    if v[0] == '-': addr &= ~1
    addr += offs
    return addr

def do_binary_kv(syms, macho, k, v):
    if v[0] in ('-', '+') and v[1] != ' ':
        return do_symstring(syms, v)
    elif v[0] == '*' and v[1] != ' ':
        off = macho.lookup_off(syms[v[1:]] & ~1)
        return struct.unpack('I', macho.stuff[off:off+4])[0]
    elif v[0] == '$' and v[1] != ' ':
        off = macho.stuff.find(struct.pack('I', macho.lookup_addr(macho.stuff.find(v[1:] + '\0')))) - 8
        return struct.unpack('I', macho.stuff[off:off+4])[0]
    elif v == '!':
        off = re.search('\x14[\x14\x00]{256}', macho.stuff).start()
        val = macho.lookup_addr(off)
        val = (val + 4) & ~3
        return val

    bits = v.split(' ')
    sstr = ''
    soff = None
    loose = False
    aligned = False
    startoff = None
    n = 0
    for bit in bits:
        if bit.startswith('='):
            startoff = macho.lookup_off(syms[bit[1:]] & ~1)
        elif bit == '@':
            loose = True
        elif bit == '+':
            soff = n + 1
        elif bit == '-': # ARM or data
            soff = n
        elif bit == '%':
            soff = n
            aligned = True
        elif bit == '..':
            sstr += '.'
            n += 1
        else:
            sstr += re.escape(chr(int(bit, 16)))
            n += 1
    if soff is None:
        raise ValueError('No offset in %s' % (v,))
    if startoff is not None:
        print macho.stuff[startoff:startoff+64].encode('hex')
        offs = list(re.compile(sstr).finditer(macho.stuff, startoff, startoff+64))
    else:
        offs = list(re.finditer(sstr, macho.stuff))
    if len(offs) == 0:
        print repr(sstr)
        raise ValueError('I couldn\'t find %s' % v)
    elif not loose and len(offs) >= 2:
        raise ValueError('I found multiple (%d) %s' % (len(offs), v))
    off = offs[0].start()
    val = macho.lookup_addr(off + soff)
    if aligned and (val & 3):
        raise ValueError('%s is not aligned: %x' % (v, val))
    return val

class macho:
    def __init__(self, binary):
        fp = open(binary, 'rb')
        self.stuff = mmap.mmap(fp.fileno(), os.path.getsize(binary), prot=mmap.PROT_READ)
        magic, cputype, cpusubtype, \
        filetype, filetype, ncmds, sizeofcmds, \
        flags = struct.unpack('IHHIIIII', fp.read(0x1c))
        self.sects = sects = []
        while True:
            xoff = fp.tell()
            if xoff >= sizeofcmds: break
            cmd, clen = struct.unpack('II', fp.read(8))
            if cmd == 1: # LC_SEGMENT
                name = fp.read(16).rstrip('\0')
                #print name
                vmaddr, vmsize, foff, fsiz = struct.unpack('IIII', fp.read(16))
                sects.append((vmaddr, foff, fsiz))
            elif cmd == 2: # LC_SYMTAB
                self.symoff, self.nsyms, self.stroff, self.strsize = struct.unpack('IIII', fp.read(16))
            fp.seek(xoff + clen)
        self.fp = fp

    def get_syms(self):
        # This could be a lot more efficient.
        # and don't get me started about lc_dyld_info
        ret = {}
        self.fp.seek(self.symoff)
        for i in xrange(self.nsyms):
            n_strx, n_type, n_sect, n_desc, n_value = struct.unpack('IBBhI', self.fp.read(12))
            n_strx += self.stroff
            if n_desc & 8:
                # thumb
                n_value |= 1
            ret[self.stuff[n_strx:self.stuff.find('\0', n_strx)]] = n_value
        return ret

    def lookup_addr(self, off):
        for startaddr, startoff, size in self.sects:
            if off >= startoff and off < (startoff + size):
                val = startaddr + (off - startoff)
                break
        return val

    def lookup_off(self, addr):
        for startaddr, startoff, size in self.sects:
            if addr >= startaddr and addr < (startaddr + size):
                val = startoff + (addr - startaddr)
                break
        return val

machos = {}
def macho_load(binary):
    if not machos.has_key(binary): 
        machos[binary] = macho(binary)
    return machos[binary]

def do_binary_uncached_macho(d, binary, syms):
    macho = macho_load(binary)

    tocache = {}
    for k, v in d.iteritems():
        if k == '@binary'  or k == '@syms' or not isinstance(v, (basestring, unicode)): continue
        tocache[k] = do_binary_kv(syms, macho, k, v)
    return tocache

## dyld shared cache stupid object format
# more efficient.

def do_binary_uncached_dyldcache(d, binary, syms):
    mydict = {}
    soffs = {}
    result = {}
    for k, v in d.iteritems():
        if not isinstance(k, basestring) or k.startswith('@'): continue
        if v[0] in ('-', '+') and v[1] != ' ':
            result[k] = do_symstring(syms, v)
            continue
        soff = None
        sstr = ''
        bits = v.split(' ')
        assert bits[0] == '@'
        bits.pop(0)
        n = 0
        for bit in bits:
            if bit == '+':
                soff = n + 1
            elif bit == '-':
                soff = n
            else:
                sstr += chr(int(bit, 16))
            n += 1
        mydict[k] = sstr
        if soff is None:
            raise ValueError('No offset in %s' % (v,))
        soffs[k] = soff

    searched = confighelper.search_for_things(binary, mydict)
    print searched

    mappings = []
    f = open(binary, 'rb')
    f.read(16) # magic
    mappingOffset, mappingCount = struct.unpack('II', f.read(8))
    f.seek(mappingOffset)
    for i in xrange(mappingCount):
        sfm_address, sfm_size, sfm_file_offset, sfm_max_prot, sfm_init_prot = struct.unpack('QQQII', f.read(32))
        mappings.append((sfm_address, sfm_size, sfm_file_offset))

    warned = False

    for k, offset in searched.iteritems():
        if offset is None:
            print >> sys.stderr, 'ERROR: Could not find %s' % (d[k],)
            warned = True
            continue
        file_offset = offset + soffs[k]
        for sfm_address, sfm_size, sfm_file_offset in mappings:
            if file_offset >= sfm_file_offset and file_offset < (sfm_file_offset + sfm_size):
                result[str(k)] = sfm_address + file_offset - sfm_file_offset
                break
        else:
            raise ValueError('Could not turn offset %x into an address' % (file_offset,))
    if warned: raise ValueError
    
    return result

###

def do_binary(d):
    #for i in ['@syms', '@binary']:
    #    if d.has_key(i):
    #        d[i] = os.path.realpath(d[i])
    binary = d['@binary']
    cachekey = hashlib.sha1(cPickle.dumps((d, os.path.getmtime(binary)), cPickle.HIGHEST_PROTOCOL)).digest()
    if cache.has_key(cachekey):
        d.update(cache[cachekey])
        return
  
    syms = get_syms(d)
    if binary.endswith('.cache') or 'dyld_shared_cache' in binary:
        tocache = do_binary_uncached_dyldcache(d, binary, syms)
    else:
        tocache = do_binary_uncached_macho(d, binary, syms)
    d.update(tocache)
    cache[cachekey] = tocache

def dict_to_cflags(d):
    cflags = ''
    for k, v in d.iteritems():
        if not isinstance(k, basestring) or '-' in k or k.startswith('@'): continue
        if isinstance(v, dict):
            cflags += dict_to_cflags(v)
            continue
        elif isinstance(v, (int, long)):
            v = hex(v)
        elif not isinstance(v, basestring):
            continue
        cflags += ' -DCONFIG_%s=%s' % (k.upper(), v)
    return cflags

def merge(a, b):
    if isinstance(a, dict):
        new = a.copy()
        new.update(b)
        for k in new:
            if a.has_key(k) and b.has_key(k):
                new[k] = merge(a[k], b[k])
        return new
    else:
        return b

def get_data(platform):
    d = data[platform]
    if d.has_key('<'):
        parent = d['<']
        del d['<']
        d = merge(get_data(parent), d)
    return d

def pretty_print(d):
    for (name, d2) in d.items():
        if isinstance(d2, dict):
            for (k, v) in d2.items():
                if isinstance(v, (long, int)):
                    print '%s -> %s: 0x%x' % (name, k, v)

def make_config(platform):
    d = get_data(platform)
    for k, v in d.iteritems():
        if k.startswith('#'):
            print >> sys.stderr, 'doing', k
            do_binary(v)
    if verbose:
        pretty_print(d)
    open('config.json', 'w').write(json.dumps(d)+'\n')
    cflags = dict_to_cflags(d) + '\n'
    open('config.cflags', 'w').write(cflags)

if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-v', '--verbose', action='store_true', dest='verbose', default=False)
    (options, args) = parser.parse_args()
    verbose = options.verbose
    make_config(args[0])

class config_data(dict):
    def __init__(self, fn): 
        import __builtin__
        dict.__init__(self, json.loads(__builtin__.open(fn).read()))
    def get_syms(self, sub):
        return get_syms(self[sub])       
        
def open():
    return config_data(basepath + '/config.json')

