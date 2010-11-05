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
f(4) # number of load commands
f(0) # sizeofcmds; overwrite this
f(0x1) # flags: NOUNDEFS

# Load commands

# LC_SEGMENT
f(1) 
f(56 + 0*68)
f('__LINKEDIT' + '\0'*6)
f(0x1000) # vmaddr
f(0x2000) # vmsize
f(0) # fileoff
f(0x2000) # filesize
f(3) # maxprot = VM_PROT_READ | VM_PROT_WRITE
f(3) # initprot = VM_PROT_READ | VM_PROT_WRITE
f(0) # no sections
f(4) # flags=SG_NORELOC

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
f(4) # flags=SG_NORELOC

# LC_SEGMENT
f(1) 
f(56 + 0*68)
f('__TEXT' + '\0'*10)
f(baseaddr) # vmaddr
f(beforesize) # vmsize
f(0) # fileoff
f(0) # filesize
f(5) # maxprot = VM_PROT_READ | VM_PROT_WRITE
f(5) # initprot = VM_PROT_READ | VM_PROT_WRITE
f(0) # no sections
f(4) # flags=SG_NORELOC


# LC_UNIXTHREAD
f(5)
f(16 + 17*4)
f(1) # ARM_THREAD_STATE
f(17) # ARM_THREAD_STATE_COUNT

# In the future I should use all of these...
for i in xrange(13): f(0) # R0-R12
f(0) # SP
f(0) # LR
f(0x2002) # PC
f(0) # CPSR

# overwrite sizeofcmds
fp.seek(0x14)
fp.write(struct.pack('I', OFF - 0x1c))
fp.seek(OFF)

# segments starting at odd offsets are not allowed.
fp.seek(0x1000)
heapoff = fp.tell()
fp.write(heap)

fp.seek(0x2ffc)
fp.write('\x00\x00\x00\x00')

fp.close()
os.chmod('one', 0o755)

