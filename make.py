#!/usr/bin/env python
# encoding: utf-8
from fabricate import *
import fabricate
fabricate.default_builder.deps # do this before we chdir
import sys, os, re

ROOT = os.path.realpath(os.path.dirname(sys.argv[0]))
os.environ['PYTHONPATH'] = ROOT+'/datautils:'+ROOT+'/goo'

# configgy

m = re.search('bs/(i[A-Z][a-z]+[0-9],[0-9])_([0-9A-Z\._]+)', os.readlink(ROOT + '/config/cur'))
device = m.group(1)
version = m.group(2)
platform, version = m.groups()
is_armv7 = device not in ['iPhone1,1', 'iPhone1,2', 'iPod1,1', 'iPod2,1']

def cify(x):
    return re.sub('[^A-Z0-9]', '_', x.upper())


GCC_FLAGS = ['-std=gnu99', '-gstabs', '-Werror', '-Wimplicit', '-Wuninitialized', '-Wall', '-Wextra', '-Wreturn-type', '-Wno-unused', '-Os']
SDK = '/var/sdk'
BIN = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin'
GCC_BIN = BIN + '/gcc-4.2'
GCC_BASE = [GCC_BIN, GCC_FLAGS, '-isysroot', SDK, '-F'+SDK+'/System/Library/Frameworks', '-F'+SDK+'/System/Library/PrivateFrameworks', '-I', ROOT, '-fno-blocks', '-mapcs-frame', '-fomit-frame-pointer']
GCC = [GCC_BASE, '-arch', ('armv7' if is_armv7 else 'armv6'), '-mthumb', '-DDEVICE_%s' % cify(device), '-DVERSION_%s' % cify(version)]
GCC_UNIVERSAL = [GCC_BASE, '-arch', 'armv6', '-arch', 'armv7', '-mthumb']
GCC_ARMV6 = [GCC_BASE, '-arch', 'armv6', '-mthumb']
GCC_NATIVE = ['gcc', '-arch', 'i386', '-arch', 'x86_64', GCC_FLAGS]
HEADERS = ROOT + '/headers'

def goto(dir):
    os.chdir(os.path.join(ROOT, dir))

def chext(f, ext):
    return f[:f.find('.')] + ext

def F(*frameworks):
    ret = []
    for framework in frameworks:
        ret.append('-framework')
        ret.append(framework)
    return ret

def shelltester():
    goto('shelltester')
    compile_stuff(['shelltester.c'], 'shelltester', strip=False)
    compile_stuff(['ghost.c'], 'ghost', strip=False, ldflags=['-framework', 'CoreFoundation', '-framework', 'CoreGraphics', '-framework', 'ImageIO'])

def install():
    goto('install')
    compile_stuff(['install.m'], 'install.dylib', gcc=GCC_ARMV6, cflags=['-I../headers', '-fblocks'], ldflags=['-framework', 'Foundation', '-framework', 'GraphicsServices', '-L.', '-ltar', '-llzma', '-dynamiclib'])

def locutus():
    goto('locutus')
    cflags = ['-DFNO_ASSERT_MESSAGES', '-fblocks', '-Oz', '-Wno-parentheses', '-miphoneos-version-min=4.0']
    compile_stuff(['locutus_server.m'], 'locutus_server.dylib', cflags=cflags+['-Wno-deprecated-declarations'], ldflags=['-dynamiclib', '-framework', 'Foundation', '-framework', 'UIKit', '-install_name', 'X'*32]+cflags, ldid=False)
    run('sh', '-c', 'xxd -i locutus_server.dylib | sed "s/locutus_server_//g" > locutus_server_.c')
    compile_stuff(['locutus.c', 'inject.c', 'baton.S',  'locutus_server_.c'], 'locutus', cflags=cflags, ldflags=['-lbz2', '-framework', 'CoreFoundation', '-framework', 'CFNetwork']+cflags, ldid=True, ent='ent.plist')
build = locutus

def goo():
    goto('goo')
    #run('python', 'setup.py'

def catalog():
    locutus()
    goo()
    datautils_native()
    sandbox2()
    goto('catalog')
    run('../datautils/make_kernel_patchfile', '../config/cur/kern', '../sandbox2/sandbox.o', 'patchfile')
    run(GCC, '-c', '-o', 'kcode.o', 'kcode.S')

def catalog_dejavu():
    catalog()
    run('python', 'catalog.py', 'two', '../config/cur/cache', '../config/cur/kern', 'patchfile')

def catalog_two():
    catalog()
    run('python', 'catalog.py', 'two', '../config/cur/cache', '../config/cur/kern', 'patchfile')

def launchd():
    goo_catalog_two()
    goto('goo')
    run('python', 'two.py', '../config/cur/cache', 'catalog/two.txt', 'launchd')

def compile_stuff(files, output, ent='', cflags=[], ldflags=[], strip=True, gcc=GCC, ldid=True, combine=False):
    objs = []
    output_ = (output + '_' if strip or ldid else output)
    if combine:
        run(gcc, '-o', output_, files, cflags, ldflags, '-dead_strip', '-combine', '-fwhole-program')
    else:
        for inp in files:
            obj = chext(os.path.basename(inp), '.o')
            objs.append(obj)
            if obj == inp: continue
            run(gcc, '-c', '-o', obj, inp, cflags)
        run(gcc, '-o', output_, objs, ldflags, '-dead_strip')
    if strip or ldid:
        commands = [['cp', output + '_', output]]
        if strip: commands.append(['strip', '-x' if 'dylib' in output else '-ur', output])
        if ldid: commands.append(['ldid', '-S' + ent, output])
        run_multiple(*commands)

def catalog2():
    goto('catalog2')
    
    run('python', 'gen_syscalls.c.py')

    # -pagezero_size 0 is harmless with ld but not ld_classic (-static); not required either way
    if False:
        static = ['-dynamic', '-mdynamic-no-pic']
    else:
        static = ['-static']
    compile_stuff(['catalog2.c', 'syscalls.c', 'iokitUser.c', 'mach_hostUser.c', 'libc.c'], 'catalog2', cflags=[static, '-marm'], ldflags=[static, '-segaddr', '__ZERO', '0', '-segprot', '__ZERO', 'rx', 'rx', '-segaddr', '__TEXT', '0x1000', '-L.'], combine=False)

def chain():
    goto('chain')
    cf = ['-marm', '-DUSE_ASM_FUNCS=0', '-fblocks']
    ldf=['-dynamiclib', '-nostdlib', '-nodefaultlibs', '-lgcc', '-undefined', 'dynamic_lookup', '-read_only_relocs', 'suppress']
    compile_stuff(['chain.c', 'dt.c', 'stuff.c', 'fffuuu.S', 'putc.S', 'annoyance.S', 'bcopy.s', 'bzero.s', 'what.s'], 'chain-kern.dylib', cflags=cf, ldflags=ldf, strip=False)
    compile_stuff(['chain-user.c'], 'chain-user', ldflags=['-framework', 'IOKit', '-framework', 'CoreFoundation'])

def data(native=True):
    goto('data')
    run_multiple(['make', 'clean'], ['make', 'NATIVE=%d' % native])

def datautils(native=False):
    data(native)
    goto('datautils')

    gcc = GCC_NATIVE if native else GCC
    ldid = strip = not native
    def cds(files, output):
        return compile_stuff(files, output, cflags=['-DIMG3_SUPPORT', '-I..', '-O3'], ldflags=['../data/libdata.a'], gcc=gcc, ldid=ldid, strip=strip)

    cds(['make_kernel_patchfile.c'], 'make_kernel_patchfile')
    cds(['apply_patchfile.c'], 'apply_patchfile')
    cds(['dyld_to_pwn.c'], 'dyld_to_pwn')

def datautils_native():
    datautils(True)

def sandbox2():
    goto('sandbox2')
    run(GCC_ARMV6, '-c', '-o', 'sandbox.o', 'sandbox.S')

def nullfs():
    goto('nullfs')
    run(GCC, '-dynamiclib', '-o', 'nullfs.dylib', 'null_subr.c', 'null_vfsops.c', 'null_vnops.c', 'vfs_pasta.c', '-fwhole-program', '-combine', '-nostdinc', '-nodefaultlibs', '-lgcc', '-Wno-error', '-Wno-parentheses', '-Wno-format', '-I.', '-Ixnu', '-Ixnu/bsd', '-Ixnu/libkern', '-Ixnu/osfmk', '-Ixnu/bsd/i386', '-Ixnu/bsd/sys', '-Ixnu/EXTERNAL_HEADERS', '-Ixnu/osfmk/libsa', '-D__i386__', '-DKERNEL', '-DKERNEL_PRIVATE', '-DBSD_KERNEL_PRIVATE', '-D__APPLE_API_PRIVATE', '-DXNU_KERNEL_PRIVATE', '-flat_namespace', '-undefined', 'dynamic_lookup', '-fno-builtin-printf', '-DNULLFS_DIAGNOSTIC')

def mroib():
    goto('mroib')
    run(GCC, '-c', '-o', 'clean.o', 'clean.c', '-marm')
    run(GCC, '-dynamiclib', '-o', 'mroib.dylib', 'power.c', 'timer.c', 'usb.c', 'mroib.c', 'clean.o', '-combine', '-fwhole-program', '-nostdinc', '-nodefaultlibs', '-lgcc', '-undefined', 'dynamic_lookup', '-I.', '-Iincludes', '-DCONFIG_IPHONE_4G')

def dejavu():
    goo_catalog_dejavu()
    goto('dejavu')
    run('python', 'gen_dejavu.raw.py')
    run('t1asm', 'dejavu.raw', 'dejavu.pfb')

def clean():
    autoclean()

main()
