#!/opt/local/bin/python2.6

import mmap, hashlib, os, re, struct

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
   
