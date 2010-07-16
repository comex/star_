#!/opt/local/bin/python2.6
import struct, re, subprocess, time, shelve, hashlib, cPickle, os, sys, plistlib, optparse, mmap, anydbm
try:
    import json
except:
    import simplejson as json

if __name__ == '__main__':
    basepath = os.path.realpath(os.path.dirname(sys.argv[0]))
    os.chdir(basepath)
    data = eval('{%s}' % open('configdata.py').read())
else:
    basepath = os.path.realpath(os.path.dirname(__file__))
cache = shelve.open(basepath + '/config.cache')

def do_symstring(binary, v):
    name = v[1:]
    offs = 0
    z = name.find('+')
    if z != -1:
        offs = eval(name[z+1:])
        name = name[:z]
    addr = binary.get_sym(name)
    # Data, so even a thumb symbol shouldn't be &1
    if v[0] == '-': addr &= ~1
    addr += offs
    return addr

def do_binary_kv(binary, k, v):
    if v[0] in ('-', '+') and v[1] != ' ':
        return do_symstring(binary, v)
    elif v[0] == '*' and v[1] != ' ':
        off = binary.lookup_off(binary.get_sym(v[1:]) & ~1)
        return struct.unpack('I', binary.stuff[off:off+4])[0]
    elif v[0] == '$' and v[1] != ' ':
        off = binary.stuff.find(struct.pack('I', binary.lookup_addr(binary.stuff.find(v[1:] + '\0')))) - 8
        return struct.unpack('I', binary.stuff[off:off+4])[0]
    elif v == '!':
        off = re.search('\x14[\x14\x00]{256}', binary.stuff).start()
        val = binary.lookup_addr(off)
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
            startoff = binary.lookup_off(binary.get_sym(bit[1:]) & ~1)
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
    if loose:
        m = re.search(sstr, binary.stuff)
        if not m:
            raise ValueError('I couldn\'t find (loose) %s' % v)
    else:
        if startoff is not None:
            print binary.stuff[startoff:startoff+64].encode('hex')
            offs = list(re.compile(sstr).finditer(binary.stuff, startoff, startoff+64))
        else:
            offs = list(re.finditer(sstr, binary.stuff))
        if len(offs) == 0:
            print repr(sstr)
            raise ValueError('I couldn\'t find %s' % v)
        elif len(offs) >= 2:
            raise ValueError('I found multiple (%d) %s' % (len(offs), v))
        m = offs[0]
    off = m.start()
    val = binary.lookup_addr(off + soff)
    if aligned and (val & 3):
        raise ValueError('%s is not aligned: %x' % (v, val))
    return val

class basebin:
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

class macho(basebin):
    def __init__(self, stuff):
        xbase = stuff.tell()
        magic, cputype, cpusubtype, \
        filetype, filetype, ncmds, sizeofcmds, \
        flags = struct.unpack('IHHIIIII', stuff.read(0x1c))
        self.sects = sects = []
        self.nsyms = None
        while True:
            xoff = stuff.tell()
            if xoff >= xbase + sizeofcmds: break
            cmd, clen = struct.unpack('II', stuff.read(8))
            if cmd == 1: # LC_SEGMENT
                name = stuff.read(16).rstrip('\0')
                #print name
                vmaddr, vmsize, foff, fsiz = struct.unpack('IIII', stuff.read(16))
                sects.append((vmaddr, foff, fsiz))
            elif cmd == 2: # LC_SYMTAB
                self.symoff, self.nsyms, self.stroff, self.strsize = struct.unpack('IIII', stuff.read(16))
            elif cmd == 11: # LC_DYSYMTAB
                self.ilocalsym, self.nlocalsym = struct.unpack('II', stuff.read(8))
                self.iextdefsym, self.nextdefsym = struct.unpack('II', stuff.read(8))
                self.iundefsym, self.nundefsym = struct.unpack('II', stuff.read(8))
            stuff.seek(xoff + clen)
        self.stuff = stuff

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

    def get_sym(self, sym):
        # Local syms aren't sorted.  So I can't use a binary search.
        if self.nsyms is None:
            raise KeyError, sym
        for off in xrange(self.symoff, self.symoff + 12*self.nsyms, 12):
            n_strx, n_type, n_sect, n_desc, n_value = struct.unpack('IBBhI', self.stuff[off:off+12])
            n_strx += self.stroff
            psym = self.stuff[n_strx:self.stuff.find('\0', n_strx)]
            print psym
            if sym != psym: continue
            # ok we found it
            if n_desc & 8:
                # thumb
                n_value |= 1
            print sym, hex(n_value)
            return n_value
        raise KeyError, sym
            

    def get_extdefsym(self, sym):
        # ImageLoaderMachOClassic::binarySearch
        if self.nsyms is None:
            raise KeyError, sym
        low = self.iextdefsym
        high = self.iextdefsym + self.nextdefsym - 1
        print 'nsyms =', self.nsyms
        while low <= high:
            pivot = (low + high) / 2
            off = self.symoff + 12*pivot
            n_strx, n_type, n_sect, n_desc, n_value = struct.unpack('IBBhI', self.stuff[off:off+12])
            n_strx += self.stroff
            psym = self.stuff[n_strx:self.stuff.find('\0', n_strx)]
            print 'psym=', psym
            result = cmp(sym, psym)
            if result > 0:
                low = pivot + 1
                continue
            elif result < 0:
                high = pivot - 1
                continue
            # ok we found it
            if n_desc & 8:
                # thumb
                n_value |= 1
            print sym, hex(n_value)
            return n_value
        raise KeyError, sym

class dyldcache(basebin):
    def __init__(self, stuff):
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
            self.sects.append((sfm_address, sfm_file_offset, sfm_size))
        
        self.files = {}
        for i in xrange(imagesCount):
            stuff.seek(imagesOffset + 32*i)
            address, modTime, inode, pathFileOffset, pad = struct.unpack('QQQII', stuff.read(32))
            stuff.seek(self.lookup_off(address))
            self.files[stuff[pathFileOffset:stuff.find('\0', pathFileOffset)]] = macho(stuff)

    def get_sym(self, sym):
        for name, macho in self.files.iteritems():
            try:
                result = macho.get_sym(sym)
            except KeyError:
                continue
            else:
                return result
        raise KeyError, sym
        

###
def binary_open(filename):
    fp = open(filename, 'rb')
    magic = fp.read(4)
    stuff = mmap.mmap(fp.fileno(), os.path.getsize(filename), prot=mmap.PROT_READ)
    if magic == 'dyld':
        binary = dyldcache(stuff)
    elif magic == struct.pack('I', 0xfeedface):
        binary = macho(stuff)
    else:
        raise Exception('Unknown magic %r' % magic)
    return binary
    
def do_binary(d):
    filename = d['@binary']
    cachekey = hashlib.sha1(cPickle.dumps((d, os.path.getmtime(filename)), cPickle.HIGHEST_PROTOCOL)).digest()
    if cache.has_key(cachekey):
        d.update(cache[cachekey])
        return

    binary = binary_open(filename)
   
    tocache = {}
    for k, v in d.iteritems():
        if k == '@binary' or not isinstance(v, basestring): continue
        tocache[k] = do_binary_kv(binary, k, v)

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
        
def openconfig():
    return config_data(basepath + '/config.json')

