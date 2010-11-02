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

heap = open(sys.argv[1], 'rb').read()
initial_pc, = struct.unpack('I', heap[:4])
heap = heap[4:]

heapsize = len(heap)
heapoff = 0x1000

fp = open('one', 'wb')
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
f(2) # MH_EXECUTE
f(10) # number of load commands
f(0) # sizeofcmds; overwrite this
f(0x1 | 0x4 | 0x80) # flags: NOUNDEFS DYLDLINK TWOLEVEL

# Load commands
# LC_SEGMENT
f(1) 
f(56 + 0*68)
f('__PAGEZERO' + '\0'*6)
f(0) # vmaddr
f(4096) # vmsize
f(0) # fileoff
f(0) # filesize
f(0) # maxprot = VM_PROT_READ | VM_PROT_WRITE
f(0) # initprot = VM_PROT_READ | VM_PROT_WRITE
f(0) # no sections
f(0) # flags=0
# LC_SEGMENT
f(1) 
f(56 + 0*68)
f('__TEXT' + '\0'*10)
f(0x20000000) # vmaddr
f(4096) # vmsize
f(0) # fileoff
f(0) # filesize
f(5) # maxprot = VM_PROT_READ | VM_PROT_WRITE
f(5) # initprot = VM_PROT_READ | VM_PROT_WRITE
f(0) # no sections
f(0) # flags=0

# LC_SEGMENT
f(1) 
f(56 + 1*68)
f('__LINKEDIT' + '\0'*6)
f(baseaddr) # vmaddr
f(beforesize) # vmsize
f(0) # fileoff
f(0) # filesize
f(5) # maxprot = VM_PROT_READ | VM_PROT_WRITE
f(5) # initprot = VM_PROT_READ | VM_PROT_WRITE
f(1) # no sections
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
f(56 + 0*68)
f('__DATA' + '\0'*10)
f(heapaddr) # vmaddr
f(heapsize) # vmsize
f(heapoff) # fileoff
f(heapsize) # filesize
f(3) # maxprot = VM_PROT_READ | VM_PROT_WRITE
f(3) # initprot = VM_PROT_READ | VM_PROT_WRITE
f(0) # no sections
f(0) # flags=0

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


dylinker = '/usr/lib/dyld\0\0\0'
# LC_LOAD_DYLINKER
f(0xe)
f(12 + len(dylinker))
f(12)
f(dylinker)

# LC_UUID
f(0x1b)
f(8 + 16)
f('\xde\xad\xbe\xef' * 4)

# LC_THREAD
f(5)
f(16 + 17*4)
f(1) # ARM_THREAD_STATE
f(17) # ARM_THREAD_STATE_COUNT

# In the future I should use all of these...
for i in xrange(13): f(0) # R0-R12
f(0xdeadbeec) # SP
f(0) # LR
f(0xdeadbeec) # PC
f(0) # CPSR

dylib = '/usr/lib/libSystem.B.dylib\0\0'
# LC_LOAD_DYLIB
f(0xc)
f(24 + len(dylib))
f(24)
f(0)
f(0x007d0400)
f(0x00010000)
f(dylib)

# LC_CODE_SIGNATURE
f(0x1d)
f(16)
f(0) # off
f(0) # size

# overwrite sizeofcmds
fp.seek(0x14)
fp.write(struct.pack('I', OFF - 0x1c))
fp.seek(OFF)

# segments starting at odd offsets are not allowed.
fp.seek(0x1000)
heapoff = fp.tell()
fp.write(heap)

fp.close()
os.chmod('one', 0o755)
