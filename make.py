#!/usr/bin/env python
# encoding: utf-8
from fabricate import *
import fabricate
fabricate.default_builder.deps # do this before we chdir
import sys, os
ROOT = os.path.realpath(os.path.dirname(sys.argv[0]))
os.environ['PYTHONPATH'] = 'ROOT/datautils:ROOT/goo:ROOT/config'.replace('ROOT', ROOT)

GCC_FLAGS = ['-std=gnu99', '-gstabs', '-Werror', '-Wimplicit', '-Wuninitialized', '-Wall', '-Wextra', '-Wreturn-type', '-Wno-unused', '-Os']
SDK = '/var/sdk'
BIN = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin'
GCC_BIN = BIN + '/gcc-4.2'
GCC_BASE = [GCC_BIN, GCC_FLAGS, '-isysroot', SDK, '-F'+SDK+'/System/Library/Frameworks', '-F'+SDK+'/System/Library/PrivateFrameworks', '-I', ROOT, '-fno-blocks', '-mapcs-frame', '-fomit-frame-pointer']
GCC = [GCC_BASE, '-arch', 'armv6', '-mthumb']
GCC_UNIVERSAL = [GCC_BASE, '-arch', 'armv6', '-arch', 'armv7', '-mthumb']
GCC_ARMV7 = [GCC_BASE, '-arch', 'armv7', '-mthumb']
GCC_NATIVE = ['gcc', GCC_FLAGS]
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

def compile_to_bin(output, input=None, flags=[]):
    # requires machdump
    ofile = output + '.o'
    binfile = output + '.bin'
    run(GCC, '-o', ofile, input, flags, '-nostdlib', '-nodefaultlibs', '-nostartfiles')
    run(ROOT + '/machdump/machdump', ofile, binfile)

def config():
    goto('.')
    run('python', 'config/generate_config.py')

def shelltester():
    goto('shelltester')
    compile_stuff(['shelltester.c'], 'shelltester', strip=False)
    compile_stuff(['ghost.c'], 'ghost', strip=False, ldflags=['-framework', 'CoreFoundation', '-framework', 'CoreGraphics', '-framework', 'ImageIO'])

def machdump():
    goto('machdump')
    run(GCC_NATIVE, '-o', 'machdump', 'machdump.c')

def install():
    goto('install')
    files = ['install.o', 'copier.o']
    for o in files:
        run(GCC_UNIVERSAL, '-I', HEADERS, '-c', '-o', o, chext(o, '.c'))
    run(GCC_UNIVERSAL, '-dynamiclib', '-o', 'install.dylib', files, F('CoreFoundation', 'GraphicsServices'), '-L.', '-ltar', '-llzma')
    run('python', 'wad.py', 'install.dylib', 'Cydia-whatever.txz')

def locutus():
    goto('locutus')
    run('mig', 'locutus.defs')
    compile_stuff(['locutus.c', 'inject.c'], 'locutus', cflags=['-fblocks', '-Wno-parentheses'], ldid=False)

def goo():
    config()
    goto('goo')
    #run('python', 'setup.py'

def goo_just_sysctl():
    goo()
    datautils_native()
    goto('goo/just_sysctl')
    run('python', 'just_sysctl.py', '-d', '../../config/cur/dyld')
    run('../../datautils/dyld_to_pwn', '../../config/cur/dyld', 'just_sysctl_output.txt', 'just_sysctl_launchd')

def goo_catalog():
    locutus()
    goo()
    datautils_native()
    sandbox2()
    goto('goo/catalog')
    run('../../datautils/make_kernel_patchfile', '../../config/cur/kern', '../../sandbox2/sandbox.bin', 'patchfile')
    compile_to_bin('kcode', ['kcode.S'])
    run('python', 'catalog.py', '-c ../../config/cur/cache', '-k ../../config/cur/kern', 'patchfile')
    run('python', 'stub.py', '-c', '../../config/cur/cache')

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
        if strip: commands.append(['strip', '-ur', output])
        if ldid: commands.append(['ldid', '-S' + ent, output])
        run_multiple(*commands)

def catalog2():
    config()
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
    compile_stuff(['chain.c', 'dt.c', 'stuff.c', 'fffuuu.S', 'putc.S', 'annoyance.S', 'bcopy.s', 'bzero.s', 'what.s'], 'chain-kern.dylib', gcc=GCC_ARMV7, cflags=cf, ldflags=ldf, strip=False)
    compile_stuff(['chain-user.c'], 'chain-user', ldflags=['-framework', 'IOKit', '-framework', 'CoreFoundation'])

def data(native=True):
    goto('data')
    run_multiple(['make', 'clean'], ['make', 'GCC=gcc' if native else 'all'])

def datautils(native=False):
    config()
    data(native)
    goto('datautils')

    gcc = GCC_NATIVE if native else GCC
    ldid = strip = not native
    def cds(files, output):
        return compile_stuff(files, output, cflags=['-DIMG3_SUPPORT', '-I..', '-O3'], ldflags=['-L../data', '-ldata'], gcc=gcc, ldid=ldid, strip=strip)

    cds(['dmini.c'], 'dmini')
    cds(['make_kernel_patchfile.c'], 'make_kernel_patchfile')
    cds(['apply_patchfile.c'], 'apply_patchfile')
    cds(['dyld_to_pwn.c'], 'dyld_to_pwn')

def datautils_native():
    datautils(True)

def sandbox2():
    goto('sandbox2')
    compile_to_bin('sandbox', ['sandbox.S'], ['-D_patch_start=start'])

def nullfs():
    goto('nullfs')
    run(GCC, '-dynamiclib', '-o', 'nullfs.dylib', 'null_subr.c', 'null_vfsops.c', 'null_vnops.c', 'vfs_pasta.c', '-fwhole-program', '-combine', '-nostdinc', '-nodefaultlibs', '-lgcc', '-Wno-error', '-Wno-parentheses', '-Wno-format', '-I.', '-Ixnu', '-Ixnu/bsd', '-Ixnu/libkern', '-Ixnu/osfmk', '-Ixnu/bsd/i386', '-Ixnu/bsd/sys', '-Ixnu/EXTERNAL_HEADERS', '-Ixnu/osfmk/libsa', '-D__i386__', '-DKERNEL', '-DKERNEL_PRIVATE', '-DBSD_KERNEL_PRIVATE', '-D__APPLE_API_PRIVATE', '-DXNU_KERNEL_PRIVATE', '-flat_namespace', '-undefined', 'dynamic_lookup', '-fno-builtin-printf', '-DNULLFS_DIAGNOSTIC')

def mroib():
    goto('mroib')
    run(GCC, '-c', '-o', 'clean.o', 'clean.c', '-marm')
    run(GCC, '-dynamiclib', '-o', 'mroib.dylib', 'power.c', 'timer.c', 'usb.c', 'mroib.c', 'clean.o', '-combine', '-fwhole-program', '-nostdinc', '-nodefaultlibs', '-lgcc', '-undefined', 'dynamic_lookup', '-I.', '-Iincludes', '-DCONFIG_IPHONE_4G')

def dejavu():
    goo_catalog()
    goto('dejavu')
    run('python', 'gen_dejavu.raw.py')
    run('t1asm', 'dejavu.raw', 'dejavu_.pfb')
    run(GCC_NATIVE, '-o', 'crazy', 'crazy.c')
    run('./crazy', 'dejavu_.pfb', 'dejavu.pfb')

def clean():
    autoclean()

main()
