#!/opt/local/bin/python2.6
import struct, re, subprocess, time, shelve, hashlib, cPickle, os, sys, plistlib, optparse, mmap, anydbm, marshal
try:
    import json
except:
    import simplejson as json

if __name__ == '__main__':
    basepath = os.path.realpath(os.path.dirname(sys.argv[0]))
    os.chdir(basepath)
else:
    basepath = os.path.realpath(os.path.dirname(__file__))

VM_PROT_READ = 1
VM_PROT_WRITE = 2
VM_PROT_EXECUTE = 4

cache = None
def cache_has_key(cachekey):
    global cache
    if cache is None:
        cache = shelve.open(basepath + '/config.cache')
    if uncached: return False
    return cache.has_key(cachekey)

def sym_(binary, v):
    assert v[0] in '-+'
    name = v[1:]
    addr = binary.get_sym(name)
    # Data, so even a thumb symbol shouldn't be &1
    if v[0] == '-': addr &= ~1
    return addr

def exeggcute(binary, mtime, d, v):
    class a:
        def sysctl(name):
            off = binary.stuff.find(struct.pack('I', binary.lookup_addr(binary.stuff.find(name + '\0')))) - 8
            return struct.unpack('I', binary.stuff[off:off+4])[0]

        def stringref(string):
            stringpos = binary.stuff.find('\0' + string + '\0')
            assert stringpos != -1
            refpos = binary.stuff.find(struct.pack('I', binary.lookup_addr(stringpos) + 1))
            assert refpos != -1
            return binary.lookup_addr(refpos)

        def bof(eof, is_thumb=True):
            # push {..., lr}; add r7, sp, ...
            # thumb: xx b5 xx af
            # arm: xx xx 2d e9 xx xx 8d e2
            eoff = binary.lookup_off(eof)
            if is_thumb:
                b5 = binary.stuff.rfind('\xb5', 0, eoff)
                while 0 == (b5 & 1) or binary.stuff[b5+2] != '\xaf': b5 = binary.stuff.rfind('\xb5', 0, b5)
                r = b5 - 1
            else:
                _2de9 = binary.stuff.rfind('\x2d\xe9', 0, eoff)
                while 2 != (_2de9 & 3) or binary.stuff[_2de9+4:_2de9+6] != '\x8d\xe2': _2de9 = binary.stuff.rfind('\x2d\xe9', 0, _2de9)
                r = _2de9 - 2
            return binary.lookup_addr(r)
                

        def scratch():
            off = re.search('\x14[\x14\x00]{256}', binary.stuff).start()
            val = binary.lookup_addr(off)
            return (val + 4) & ~3

        def mpo():
            found = binary.stuff.find('Seatbelt sandbox policy\0')
            if found == -1: # 3.1.x
                found = binary.stuff.find('Seatbelt Policy\0')
            assert found != -1
            off = binary.stuff.find(struct.pack('I', binary.lookup_addr(found))) + 12
            return struct.unpack('I', binary.stuff[off:off+4])[0]

        def sym(v):
            return sym_(binary, v)

        def deref(addr):
            return binary.deref(addr)

        def k(the_k):
            return do_binary_k_cached(binary, mtime, d, the_k)

    return eval(v.func_code, a.__dict__)

def do_binary_kv(binary, mtime, d, k, v):
    if callable(v):
        return exeggcute(binary, mtime, d, v)
    elif isinstance(v, list):
        return v
    elif (v.startswith('-') or v.startswith('+')) and v[1] != ' ':
        return sym_(binary, v)

    bits = v.split(' ')
    xstr = ''
    sstr = ''
    soff = None
    loose = False
    align = 2
    startoff = 0
    nonexact = False
    thumb = False
    exec_required = True
    n = 0
    for bit in bits:
        if bit.startswith('='):
            startoff = binary.lookup_off(do_binary_k_cached(binary, mtime, d, bit[1:]))
        elif bit == '@':
            loose = True
        elif bit == '+': # Thumb
            soff = n
            thumb = True
        elif bit == '-': # ARM
            soff = n
        elif bit == '/': # data
            soff = n
            exec_required = False
        elif bit == '~':
            align = 1
        elif bit == '%':
            align = 4
        elif bit == '..':
            sstr += '.'
            nonexact = True
            n += 1
        elif '/' in bit:
            sstr += '[%s]' % ''.join(re.escape(chr(int(i, 16))) for i in bit.split('/')) 
            nonexact = True
            n += 1
        else:
            x = chr(int(bit, 16))
            xstr += x
            sstr += re.escape(x)
            n += 1
    if soff is None:
        raise ValueError('No offset in %s' % (v,))
    def addrok(off):
        # returns the address, or None if it's not suitable
        try:
            addr, prot = binary.lookup_addr_and_prot(startoff + off + soff)
        except ValueError:
            return None
        if addr & (align - 1):
            return None
        if exec_required and not (prot & VM_PROT_EXECUTE):
            return None
        if thumb: addr |= 1
        return addr
    if loose:
        assert startoff == 0
        if nonexact:
            m = re.search(sstr, binary.stuff)
            while m:
                val = addrok(m.start())
                if val is not None: break
                m = re.search(sstr, binary.stuff, m.start() + 1)
            else:
                raise ValueError('I couldn\'t find (loose) %s' % v)
            off = m.start()
        else:
            off = binary.stuff.find(xstr, 0)
            while off != -1:
                val = addrok(off)
                if val is not None: break
                off = binary.stuff.find(xstr, off + 1)
            else:
                raise ValueError('I couldn\'t find (loose, exact) %s' % v)
    else:
        if startoff:
            #print binary.stuff[startoff:startoff+256].encode('hex')
            offs = list(re.finditer(sstr, binary.stuff[startoff:startoff+256]))
        else:
            offs = list(re.finditer(sstr, binary.stuff))
        vals = filter(lambda a: a is not None, [addrok(m.start()) for m in offs])
        if len(vals) == 0:
            print repr(sstr)
            raise ValueError('I couldn\'t find %s' % v)
        elif len(vals) >= 2:
            raise ValueError('I found multiple (%d) %s' % (len(vals), v))
        val = vals[0]
    #print 'for', k, 'off=%x soff=%x addr=%x' % (off, soff, val)
    return val

def my_str(v):
    # like str, but in the case of a lambda, does something useful
    if hasattr(v, 'func_code'):
        return marshal.dumps(v.func_code)
    else:
        return str(v)

def do_binary_k_cached(binary, mtime, d, k):
    v = d[k]
    if k == '@binary' or isinstance(v, (int, long)): return v
    cachekey = mtime + str(k) + my_str(v)
    if not cache_has_key(cachekey):
        cache[cachekey] = do_binary_kv(binary, mtime, d, k, v)
    return cache[cachekey]

class basebin:
    def lookup_addr(self, off):
        for startaddr, startoff, size, prot in self.sects:
            if off >= startoff and off < (startoff + size):
                return startaddr + (off - startoff)
        raise ValueError('No address for offset %x' % off)
    
    def lookup_addr_and_prot(self, off):
        for startaddr, startoff, size, prot in self.sects:
            if off >= startoff and off < (startoff + size):
                return (startaddr + (off - startoff), prot)
        raise ValueError('No address for offset %x' % off)

    def lookup_off(self, addr):
        for startaddr, startoff, size, prot in self.sects:
            if addr >= startaddr and addr < (startaddr + size):
                return startoff + (addr - startaddr)
        raise ValueError('No offset for address %x' % addr)
    
    def deref(self, addr):
        off = self.lookup_off(addr)
        return struct.unpack('I', self.stuff[off:off+4])[0]

    def get_sym(self, sym):
        if self.name is None:
            return self.get_sym_uncached(sym)
        cachekey = hashlib.sha1(str(self.name) + struct.pack('f', os.path.getmtime(self.name)) + sym).digest()
        if cache_has_key(cachekey):
            return cache[cachekey]
        value = self.get_sym_uncached(sym)
        cache[cachekey] = value
        return value

    def __getitem__(self, sym):
        return self.get_sym(sym)

class macho(basebin):
    def __init__(self, name, stuff):
        self.name = name
        self.stuff = stuff
        xbase = stuff.tell()
        magic, cputype, cpusubtype, \
        filetype, filetype, ncmds, sizeofcmds, \
        flags = struct.unpack('IHHIIIII', stuff.read(0x1c))
        self.sects = sects = []
        self.nsyms = None
        self.syms = None
        while True:
            xoff = stuff.tell()
            if xoff >= xbase + sizeofcmds: break
            cmd, clen = struct.unpack('II', stuff.read(8))
            if cmd == 1: # LC_SEGMENT
                name = stuff.read(16).rstrip('\0')
                vmaddr, vmsize, foff, fsiz, maxprot, initprot = struct.unpack('IIIIii', stuff.read(24))
                # why is the prot wrong in the file?
                if name == '__PRELINK_TEXT': initprot = 5
                sects.append((vmaddr, foff, fsiz, initprot))
            elif cmd == 2: # LC_SYMTAB
                self.symoff, self.nsyms, self.stroff, self.strsize = struct.unpack('IIII', stuff.read(16))
            elif cmd == 11: # LC_DYSYMTAB
                self.ilocalsym, self.nlocalsym = struct.unpack('II', stuff.read(8))
                self.iextdefsym, self.nextdefsym = struct.unpack('II', stuff.read(8))
                self.iundefsym, self.nundefsym = struct.unpack('II', stuff.read(8))
            stuff.seek(xoff + clen)

    def print_all_syms(self):
        print 'local:', self.ilocalsym, self.nlocalsym
        print 'extdef:', self.iextdefsym, self.nextdefsym
        print 'undef:', self.iundefsym, self.nundefsym
        for i in xrange(self.nsyms):
            off = self.symoff + 12*i
            n_strx, n_type, n_sect, n_desc, n_value = struct.unpack('IBBhI', self.stuff[off:off+12])
            n_strx += self.stroff
            psym = self.stuff[n_strx:self.stuff.find('\0', n_strx)]
            print '%d: %s' % (i, psym)
    
    def get_syms(self):
        syms = {}
        for off in xrange(self.symoff, self.symoff + 12*self.nsyms, 12):
            n_strx, n_type, n_sect, n_desc, n_value = struct.unpack('IBBhI', self.stuff[off:off+12])
            if n_value == 0 or n_strx == 0: continue
            if n_strx > self.strsize: break # wtf
            n_strx += self.stroff
            psym = self.stuff[n_strx:self.stuff.find('\0', n_strx)]
            if n_desc & 8:
                # thumb
                n_value |= 1
            syms[psym] = n_value
        return syms
            
    def get_sym_uncached(self, sym):
        # Local syms aren't sorted.  So I can't use a binary search.
        if self.nsyms is None:
            raise KeyError, sym
        if self.syms is None:
            self.syms = self.get_syms()
        return self.syms[sym]


class dyldcache(basebin):
    def __init__(self, name, stuff):
        self.name = name
        self.stuff = stuff
        stuff.seek(0)
        magic = stuff.read(16)
        assert re.match('dyld_v1   armv.\0' , magic)
    
        mappingOffset, mappingCount = struct.unpack('II', stuff.read(8))
        imagesOffset, imagesCount = struct.unpack('II', stuff.read(8))

        stuff.seek(mappingOffset)
        self.sects = []
        for i in xrange(mappingCount):
            sfm_address, sfm_size, sfm_file_offset, sfm_max_prot, sfm_init_prot = struct.unpack('QQQII', stuff.read(32))
            self.sects.append((sfm_address, sfm_file_offset, sfm_size, sfm_init_prot))
        
        self.files = {}
        for i in xrange(imagesCount):
            stuff.seek(imagesOffset + 32*i)
            address, modTime, inode, pathFileOffset, pad = struct.unpack('QQQII', stuff.read(32))
            stuff.seek(self.lookup_off(address))
            self.files[stuff[pathFileOffset:stuff.find('\0', pathFileOffset)]] = macho(None, stuff)

    def get_sym_uncached(self, sym):
        for name, macho in self.files.iteritems():
            try:
                result = macho.get_sym(sym)
            except KeyError:
                continue
            else:
                return result
        raise KeyError, sym

class nullbin(basebin):
    name = None
    def get_sym_uncached(self, sym):
        raise KeyError, sym

###
def binary_open(filename):
    if filename is None:
        return nullbin()
    fp = open(filename, 'rb')
    magic = fp.read(4)
    stuff = mmap.mmap(fp.fileno(), os.path.getsize(filename), prot=mmap.PROT_READ)
    if magic == 'dyld':
        binary = dyldcache(filename, stuff)
    elif magic == struct.pack('I', 0xfeedface):
        binary = macho(filename, stuff)
    else:
        raise Exception('Unknown magic %r in %s' % (magic, filename))
    return binary
   
def do_adjusted_vram_baseaddr(d, k):
    if not d.has_key(k): return
    r7_key, pc_key = d[k]
    if not isinstance(r7_key, basestring): return
    r7s = d[r7_key]
    if not isinstance(r7s, list): r7s = d[r7_key] = [r7s]
    r7s_max = max(r7s)
    r7s_min = min(r7s)
    pc = d[pc_key]
    cachekey = 'vram' + struct.pack('II', r7s_max, pc)
    if cache_has_key(cachekey):
        d[k] = cache[cachekey]
    else:
        assert not any(r7 & 1 for r7 in r7s)
        size, dr7 = min((max((pc * (i + r7s_max) * 4) & 0xffffffff, r7s_max - r7s_min + i + 0x10000), i) for i in xrange(0, 1000000, 4))
        size = (size + 3) & ~3
        r7_ = r7s_max + dr7
        print 'well', k, size, hex(r7_), map(hex, r7s)
        cache[cachekey] = d[k] = (size, r7_)

def do_binary(name, d):
    if d.has_key('@binary'):
        filename = d['@binary']
    else:
        plat = '../bs/%s' % platform
        filename = plat + '/' + name[1:]
        if not os.path.exists(filename):
            if not os.path.exists(plat): os.mkdir(plat)
            assert 0 == os.system('curl "http://$BS_HOST/%s.lzma" | lzma -d > "%s"' % (filename[3:], filename))
            assert os.path.exists(filename)
    if filename is not None:
        d['@binary'] = os.path.realpath(filename)
        mtime = str(os.path.getmtime(filename))
    else:
        mtime = 0
    binary = binary_open(filename)

    for k in d.iterkeys():
        d[k] = do_binary_k_cached(binary, mtime, d, k)

    do_adjusted_vram_baseaddr(d, 'adjusted_vram_baseaddr')
    do_adjusted_vram_baseaddr(d, 'adjusted_vram_baseaddr_atboot')


def dict_to_header(d):
    header = ''
    for k, v in d.iteritems():
        if not isinstance(k, basestring) or '-' in k or k.startswith('@'): continue
        if isinstance(v, dict):
            header += dict_to_header(v)
            continue
        elif isinstance(v, (int, long)):
            v = hex(v)
        elif not isinstance(v, basestring):
            continue
        elif ',' in v:
            v = '"%s"' % v
        header += '#define CONFIG_%s %s\n' % (k.upper(), v)
    return header

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

def make_config(platform_):
    global platform
    platform = None
    for k in data.keys():
        if k == platform_ or k.startswith(platform_ + '_'):
            if platform:
                raise KeyError('Ambiguous platform %s' % platform_)
            platform = k
    if platform is None:
        raise KeyError(platform_)
    d = get_data(platform)
    d['platform'] = platform
    for k, v in d.iteritems():
        if k.startswith('#'):
            print >> sys.stderr, 'doing', k
            do_binary(k, v)
    if verbose:
        pretty_print(d)
    open('config.json', 'w').write(json.dumps(d)+'\n')
    h = dict_to_header(d) + '\n'
    open('config.h', 'w').write(h)

if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-v', '--verbose', action='store_true', dest='verbose', default=False)
    parser.add_option('-u', '--uncached', action='store_true', dest='uncached', default=False)
    parser.add_option('-w', '--world', action='store', dest='world', default='1')
    (options, args) = parser.parse_args()
    world = options.world
    verbose = options.verbose
    uncached = options.uncached
    stuff = eval('{%s}' % (open('configdata.py').read()))
    data = stuff['all'].copy()
    data.update(stuff['world%s' % world])
    make_config(args[0])
else:
    verbose = False
    uncached = False
        
def openconfig():
    return json.loads(open(basepath + '/config.json').read())

