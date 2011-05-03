#!/usr/bin/env python
# encoding: utf-8
from fabricate import *
import fabricate
fabricate.default_builder.deps # do this before we chdir
import sys, os, re

ROOT = os.path.realpath(os.path.dirname(sys.argv[0]))
os.environ['PYTHONPATH'] = ROOT+'/datautils:'+ROOT+'/goo'

# configgy

m = re.search('bs/(i[A-Z][a-z]+[0-9],[0-9])_([0-9\.]+)_([A-Z0-9]+)', os.readlink(ROOT + '/config/cur'))
os.environ['DEVICE'] = device = m.group(1)
os.environ['VERSION'] = version = m.group(2)
os.environ['BUILD_NUM'] = build_num = m.group(3)
is_armv7 = device not in ['iPhone1,1', 'iPhone1,2', 'iPod1,1', 'iPod2,1']
#os.environ['ARMV7'] = str(int(is_armv7))
#def cify(x):
#    return re.sub('[^A-Z0-9]', '_', x.upper())
iversion = version.split('.') + [0, 0]
iversion = int(iversion[0]) * 0x10000 + int(iversion[1]) * 0x100 + int(iversion[2])


GCC_FLAGS = ['-std=gnu99', '-gstabs', '-Werror', '-Wimplicit', '-Wuninitialized', '-Wall', '-Wextra', '-Wreturn-type', '-Wno-unused', '-Os']
SDK = '/var/sdk'
BIN = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin'
GCC_BIN = BIN + '/gcc-4.2'
GCC_BASE = [GCC_BIN, GCC_FLAGS, '-isysroot', SDK, '-F'+SDK+'/System/Library/Frameworks', '-F'+SDK+'/System/Library/PrivateFrameworks', '-I', ROOT, '-fblocks', '-mapcs-frame', '-fomit-frame-pointer', '-DVERSION=0x%x' % iversion]
GCC = [GCC_BASE, '-arch', ('armv7' if is_armv7 else 'armv6'), '-mthumb']
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
    cflags = ['-DFNO_ASSERT_MESSAGES', '-fblocks', '-Oz', '-Wno-parentheses', '-miphoneos-version-min=4.0', '-Wno-deprecated-declarations']
    compile_stuff(['locutus_server.m'], 'locutus_server.dylib', gcc=GCC_ARMV6, cflags=cflags, ldflags=['-dynamiclib', '-framework', 'Foundation', '-framework', 'UIKit', '-install_name', 'X'*32]+cflags, ldid=False)
    run('sh', '-c', 'xxd -i locutus_server.dylib | sed "s/locutus_server_//g" > locutus_server_.c')
    compile_stuff(['locutus.c', 'inject.c', 'baton.S',  'locutus_server_.c'], 'locutus', gcc=GCC_ARMV6, cflags=cflags, ldflags=['-lbz2', '-framework', 'CoreFoundation', '-framework', 'CFNetwork', '-framework', 'Foundation']+cflags, ldid=True, ent='ent.plist')
build = locutus

def goo():
    goto('goo')
    #run('python', 'setup.py'

def catalog():
    locutus()
    goo()
    make('data', 'universal')
    make('datautils0', 'universal')
    goto('catalog')
    run('../datautils0/universal/make_kernel_patchfile', '../config/cur/kern', 'patchfile')

def catalog_dejavu():
    catalog()
    run(GCC, '-c', '-o', 'kcode_dejavu.o', 'kcode.S', '-Oz', '-DDEJAVU')
    run('python', 'catalog.py', 'dejavu', '../config/cur/cache', '../config/cur/kern', 'patchfile', 'kcode_dejavu.o')

def catalog_two():
    catalog()
    run(GCC, '-c', '-o', 'kcode_two.o', 'kcode.S', '-Oz')
    run('python', 'catalog.py', 'two', '../config/cur/cache', '../config/cur/kern', 'patchfile', 'kcode_two.o')

def launchd():
    catalog_two()
    goto('catalog')
    run('python', '../goo/two.py', '../config/cur/cache', 'two.txt', 'launchd')

def pdf():
    dejavu()
    goto('pdf')
    run('python', 'mkpdf.py', '../dejavu/dejavu.pfb', 'out.pdf')

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

def chain():
    goto('chain')
    cf = ['-marm', '-DUSE_ASM_FUNCS=0', '-fblocks']
    ldf=['-dynamiclib', '-nostdlib', '-nodefaultlibs', '-lgcc', '-undefined', 'dynamic_lookup', '-read_only_relocs', 'suppress']
    compile_stuff(['chain.c', 'dt.c', 'stuff.c', 'fffuuu.S', 'putc.S', 'annoyance.S', 'bcopy.s', 'bzero.s', 'what.s'], 'chain-kern.dylib', cflags=cf, ldflags=ldf, strip=False)
    compile_stuff(['chain-user.c'], 'chain-user', ldflags=['-framework', 'IOKit', '-framework', 'CoreFoundation'])

def make(path, build, *targets):
    goto(path)
    run('make', 'BUILD='+build, *targets)

def sandbox2():
    goto('sandbox2')
    run(GCC_ARMV6, '-c', '-o', 'sandbox.o', 'sandbox.S')

def nullfs():
    goto('nullfs')
    run_multiple([GCC, '-dynamiclib', '-o', 'nullfs.dylib', 'null_subr.c', 'null_vfsops.c', 'null_vnops.c', 'vfs_pasta.c', '-fwhole-program', '-combine', '-nostdinc', '-nodefaultlibs', '-lgcc', '-Wno-error', '-Wno-parentheses', '-Wno-format', '-I.', '-Ixnu', '-Ixnu/bsd', '-Ixnu/libkern', '-Ixnu/osfmk', '-Ixnu/bsd/i386', '-Ixnu/bsd/sys', '-Ixnu/EXTERNAL_HEADERS', '-Ixnu/osfmk/libsa', '-D__i386__', '-DKERNEL', '-DKERNEL_PRIVATE', '-DBSD_KERNEL_PRIVATE', '-D__APPLE_API_PRIVATE', '-DXNU_KERNEL_PRIVATE', '-flat_namespace', '-undefined', 'dynamic_lookup', '-fno-builtin-printf', '-DNULLFS_DIAGNOSTIC'],
                  ['strip', '-ur', 'nullfs.dylib'])

def mroib():
    goto('mroib')
    run(GCC, '-c', '-o', 'clean.o', 'clean.c', '-marm')
    run(GCC, '-dynamiclib', '-o', 'mroib.dylib', 'power.c', 'timer.c', 'usb.c', 'mroib.c', 'clean.o', '-combine', '-fwhole-program', '-nostdinc', '-nodefaultlibs', '-lgcc', '-undefined', 'dynamic_lookup', '-I.', '-Iincludes', '-DCONFIG_IPHONE_4G')

def dejavu():
    catalog_dejavu()
    goto('dejavu')
    run('python', 'gen_dejavu.raw.py')
    run('t1asm', 'dejavu.raw', 'dejavu.pfb')

def white():
    make('data', 'universal')
    make('white', 'universal', 'universal', 'universal/white_loader')
    make('data', 'armv6')
    goto('white')
    run_multiple([GCC_ARMV6, '-o', 'white_loader', 'white_loader.c', '../data/armv6/libdata.a', '-DMINIMAL', '-Wno-parentheses'],
                 ['strip', '-ur', 'white_loader'],
                 ['ldid', '-Sent.plist', 'white_loader'])

def starstuff():
    nullfs()
    white()
    launchd()
    goto('starstuff')
    run('../white/universal/white_loader', '-k', '../config/cur/kern', '-p', '../nullfs/nullfs.dylib', 'nullfs_prelink.dylib')
    run('gnutar', 'chvf', 'starstuff.tar', '-C', 'root', '.', '--owner', '0', '--group', 0)
    run('sh', '-c', 'xz < starstuff.tar > starstuff_%s_%s.tar.xz' % (device, build_num))

def foo():
    run('touch', 'foo')

def clean():
    goto('.')
    for d in ['data', 'datautils0', 'white']:
        make(d, '', 'clean')
    autoclean()

main()
