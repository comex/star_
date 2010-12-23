#!/usr/bin/env python
# encoding: utf-8
from fabricate import *
import fabricate
fabricate.default_builder.deps # do this before we chdir
import sys, os
ROOT = os.path.realpath(os.path.dirname(sys.argv[0]))
os.environ['PYTHONPATH'] = ROOT + '/goo'

GCC_FLAGS = ['-std=gnu99', '-gdwarf-2', '-Werror', '-Wimplicit', '-Wuninitialized', '-Wall', '-Wextra', '-Wreturn-type', '-Wno-unused', '-Os']
SDK = '/var/sdk'
BIN = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin'
GCC_BIN = BIN + '/gcc-4.2'
GCC_BASE = [GCC_BIN, GCC_FLAGS, '-isysroot', SDK, '-F'+SDK+'/System/Library/Frameworks', '-F'+SDK+'/System/Library/PrivateFrameworks', '-I', ROOT, '-fno-blocks', '-mapcs-frame', '-fomit-frame-pointer']
GCC = [GCC_BASE, '-arch', 'armv6', '-mthumb']
GCC_UNIVERSAL = [GCC_BASE, '-arch', 'armv6', '-arch', 'armv7', '-mthumb']
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

def pmap():
    goto('star/pmap')
    for x in ['pmap2', 'pmaparb', 'shelltester']:
        run(GCC_UNIVERSAL, '-o', x, x + '.c', '-I', headers, F('IOKit', 'CoreFoundation', 'IOSurface'))

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

def installui():
    goto('installui')
    if not os.path.exists('dumpedUIKit'):
        run('./mkUIKit.sh')
        
    files = ['dddata.o', 'installui.o']
    for o in files:
        run(GCC, '-I', HEADERS, '-I', '.', '-c', '-o', o, chext(o, '.m'))
    run(GCC, '-dynamiclib', '-o', 'installui.dylib', files, F('Foundation', 'UIKit', 'IOKit', 'CoreGraphics'), '-lz')

def goo():
    pass

def goo_pf():
    goo()
    goto('goo/pf')
    run('python', 'transe.py')
    run('python', '../one.py', 'transeboot.txt')

def compile_stuff(files, output, ent='', cflags=[], ldflags=[], strip=True, gcc=GCC, ldid=True):
    objs = []
    for inp in files:
        obj = chext(os.path.basename(inp), '.o')
        objs.append(obj)
        if obj == inp: continue
        run(gcc, '-c', '-o', obj, inp, cflags)
    if strip:
        run(gcc, '-o', output + '_', objs, ldflags, '-dead_strip')
        if ldid:
            run_multiple(['cp', output + '_', output],
                         ['strip', '-Sx', output],
                         ['ldid', '-S' + ent, output])
        else:
            run_multiple(['cp', output + '_', output],
                         ['strip', '-Sx', output])
    else:
        run(gcc, '-o', output, objs, ldflags, '-dead_strip')

def pf2():
    goto('pf2')
    compile_stuff(['pf2.c', '../sandbox2/sandbox.S'], 'pf2')

def chain():
    goto('chain')
    cf = ['-marm', '-DUSE_ASM_FUNCS=1', '-fblocks']
    ldf=['-dynamiclib', '-nostdlib', '-nodefaultlibs', '-lgcc', '-undefined', 'dynamic_lookup', '-read_only_relocs', 'suppress']
    compile_stuff(['start.s', 'chain.c', 'dt.c', 'stuff.c', 'fffuuu.S', 'putc.S', 'annoyance.S', 'bcopy.s', 'bzero.s', 'what.s'], 'chain-kern.dylib', cflags=cf, ldflags=ldf, strip=False)
    compile_stuff(['chain-user.c'], 'chain-user', ldflags=['-framework', 'IOKit', '-framework', 'CoreFoundation'])

data_files = ['binary.c', 'find.c', 'common.c', 'cc.c', 'lzss.c', 'running_kernel.c']

def data(gcc=GCC, ldid=True):
    goto('data')
    compile_stuff(data_files + ['deplaceholder.c'], 'deplaceholder', cflags='-DIMG3_SUPPORT', gcc=gcc, ldid=ldid)
    compile_stuff(data_files + ['kernel_patcher.c'], 'kernel_patcher', 'ent.plist', cflags='-DIMG3_SUPPORT', gcc=gcc, ldid=ldid)
    compile_stuff(['dyld_to_pwn.c'], 'dyld_to_pwn', gcc=gcc, ldid=ldid)

def upgrade_data():
    data_upgrade()
    goto('upgrade-data')
    compile_stuff(['lol_mkdir.c'], 'lol_mkdir', '../data/ent.plist')
    run('./build-deb.sh')

def data_native():
    data(GCC_NATIVE, False)

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

def clean():
    autoclean()

main()
