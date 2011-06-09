#!/usr/bin/env python
# encoding: utf-8
from fabricate import *
import fabricate
def hybrid_hasher(filename):
    try:
        return (mtime_hasher if os.path.getsize(filename) > 1048576 else md5_hasher)(filename)
    except (IOError, OSError):
        return None
fabricate.default_builder.hasher = hybrid_hasher
fabricate.default_builder.deps # do this before we chdir
import sys, os, re, glob, traceback, shutil, tempfile

ROOT = os.path.realpath(os.path.dirname(sys.argv[0]))

# configgy
def set_firmware(firmware=None, lndir=False):
    global iversion, device, version, build_num, is_armv7, BUILD_ROOT, BS
    if firmware is None:
        firmware = os.readlink(ROOT + '/config/cur').strip('/').split('/')[-1]
    BS = ROOT + '/bs/' + firmware
    device, version, build_num = re.match('(i[A-Z][a-z]+[0-9],[0-9x])_([0-9\.]+)_([A-Z0-9]+)', firmware).groups()
    is_armv7 = device not in ['iPhone1,1', 'iPhone1,2', 'iPod1,1', 'iPod2,1']
    bits = version.split('.') + [0, 0]
    iversion = int(bits[0]) * 0x10000 + int(bits[1]) * 0x100 + int(bits[2])
    if lndir:
        BUILD_ROOT = os.path.realpath(ROOT + '/config/build-' + firmware)
        if not os.path.exists(BUILD_ROOT): os.mkdir(BUILD_ROOT)
    else:
        BUILD_ROOT = ROOT
try:
    set_firmware()
except OSError:
    pass

def tmp(x):
    x = os.path.join(os.getcwd(), x)
    if ROOT is not BUILD_ROOT: x = x.replace(ROOT, BUILD_ROOT) 
    return x

GCC_FLAGS = ['-std=gnu99', '-g3', '-Werror', '-Wimplicit', '-Wuninitialized', '-Wall', '-Wextra', '-Wreturn-type', '-Wno-unused', '-Os']
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
    if ROOT is not BUILD_ROOT: shell('mkdir', '-p', os.path.join(BUILD_ROOT, dir))
    os.chdir(os.path.join(ROOT, dir))

def chext(f, ext):
    return f[:f.find('.')] + ext

def install():
    goto('install')
    compile_stuff(['install.m'], tmp('install.dylib'), gcc=GCC_ARMV6, cflags=['-I../headers', '-fblocks'], ldflags=['-framework', 'Foundation', '-framework', 'GraphicsServices', '-L.', '-ltar', '-llzma', '-dynamiclib'])

def locutus():
    goto('locutus')
    cflags = ['-DFNO_ASSERT_MESSAGES', '-fblocks', '-Oz', '-Wno-parentheses', '-miphoneos-version-min=4.0', '-Wno-deprecated-declarations']
    compile_stuff(['locutus_server.m'], tmp('locutus_server.dylib'), gcc=GCC_ARMV6, cflags=cflags, ldflags=['-dynamiclib', '-framework', 'Foundation', '-framework', 'UIKit', '-install_name', 'X'*32]+cflags, ldid=False)
    run('sh', '-c', 'xxd -i "%s" | sed "s/[l_].*dylib/dylib/g" > "%s"' % (tmp('locutus_server.dylib'), tmp('locutus_server_.c')))
    compile_stuff(['locutus.c', 'inject.c', 'baton.S',  tmp('locutus_server_.c')], tmp('locutus'), gcc=GCC_ARMV6, cflags=cflags, ldflags=['-lbz2', '-framework', 'CoreFoundation', '-framework', 'CFNetwork', '-framework', 'Foundation']+cflags, ldid=True, ent='ent.plist')
build = locutus


def catalog():
    make('data', 'universal')
    make('datautils0', 'universal')
    goto('catalog')
    run('../datautils0/universal/make_kernel_patchfile', BS+'/kern', tmp('patchfile'))

def catalog_dejavu(extra_bs=[]):
    goto('catalog')
    locutus()
    catalog()
    run(GCC, '-c', '-o', tmp('kcode_dejavu.o'), 'kcode.S', '-Oz', '-DDEJAVU')
    extra = [i+'/cache' for i in extra_bs]
    run('python', 'catalog.py', 'dejavu', version, BS+'/cache', BS+'/kern', tmp('patchfile'), tmp('kcode_dejavu.o'), tmp('catalog.txt'), *extra)

def catalog_untether():
    catalog()
    run(GCC, '-c', '-o', tmp('kcode_two.o'), 'kcode.S', '-Oz')
    run('python', 'catalog.py', 'untether', version, BS+'/cache', BS+'/kern', tmp('patchfile'), tmp('kcode_two.o'), tmp('two.txt'))

def untether():
    catalog_untether()
    goto('catalog')
    run('python', '../goo/two.py', BS+'/cache', tmp('two.txt'), tmp('untether'))

def pdf(files=None):
    dejavu(files)
    goto('pdf')
    run('python', 'mkpdf.py', tmp('../dejavu/dejavu.pfb'), tmp('out.pdf'))

def compile_stuff(files, output, ent='', cflags=[], ldflags=[], strip=True, gcc=GCC, ldid=True, combine=False, use_tmp=True):
    objs = []
    output_ = (output + '_' if strip or ldid else output)
    if combine:
        run(gcc, '-o', output_, files, cflags, ldflags, '-dead_strip', '-combine', '-fwhole-program')
    else:
        for inp in files:
            obj = chext(os.path.basename(inp), '.o')
            if use_tmp: obj = tmp(obj)
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
    run(GCC_ARMV6, '-c', '-o', tmp('sandbox.o'), 'sandbox.S')

def fs():
    goto('fs')
    crap = [GCC, '-dynamiclib', '-g3',  '-fwhole-program', '-combine', '-nostdinc', '-nodefaultlibs', '-lgcc', '-Wno-error', '-Wno-parentheses', '-Wno-format', '-I.', '-Ixnu', '-Ixnu/bsd', '-Ixnu/libkern', '-Ixnu/osfmk', '-Ixnu/bsd/i386', '-Ixnu/bsd/sys', '-Ixnu/EXTERNAL_HEADERS', '-Ixnu/osfmk/libsa', '-D__i386__', '-DKERNEL', '-DKERNEL_PRIVATE', '-DBSD_KERNEL_PRIVATE', '-D__APPLE_API_PRIVATE', '-DXNU_KERNEL_PRIVATE', '-flat_namespace', '-undefined', 'dynamic_lookup', '-fno-builtin-printf', '-fno-builtin-log', '-DNULLFS_DIAGNOSTIC', '-dead_strip']
    #run(*(crap + ['-o', tmp('nullfs.dylib'), 'kpi_vfs.c', 'null/null_subr.c', 'null/null_vfsops.c', 'null/null_vnops.c']))
    run(*(crap + ['-o', tmp('union.dylib'), 'kpi_vfs.c', 'union/union_subr.c', 'union/union_vfsops.c', 'union/union_vnops.c']))

def mroib():
    goto('mroib')
    run(GCC, '-c', '-o', 'clean.o', 'clean.c', '-marm')
    run(GCC, '-dynamiclib', '-o', 'mroib.dylib', 'power.c', 'timer.c', 'usb.c', 'mroib.c', 'clean.o', '-combine', '-fwhole-program', '-nostdinc', '-nodefaultlibs', '-lgcc', '-undefined', 'dynamic_lookup', '-I.', '-Iincludes', '-DCONFIG_IPHONE_4G')

def dejavu(files=None):
    goto('catalog')
    if files is None: files = [tmp('catalog.txt')]
    catalog_dejavu()
    goto('dejavu')
    run('python', 'gen_dejavu.raw.py', tmp('dejavu.raw'), tmp('../locutus/locutus'), *files)
    run('t1asm', tmp('dejavu.raw'), tmp('dejavu.pfb'))

def white():
    make('data', 'universal')
    make('white', 'universal', 'universal', 'universal/white_loader')
    make('data', 'armv6')
    goto('white')
    run_multiple([GCC_ARMV6, '-o', 'white_loader', 'white_loader.c', '../data/armv6/libdata.a', '-DMINIMAL', '-Wno-parentheses'],
                 ['strip', '-ur', 'white_loader'],
                 ['ldid', '-Sent.plist', 'white_loader'])

def starstuff():
    fs()
    white()
    untether()
    goto('starstuff')
    compile_stuff(['mount_nulls.c'], 'mount_nulls', ldid=False, gcc=GCC_ARMV6, use_tmp=False)
    #run('../white/universal/white_loader', '-k', BS+'/kern', '-p', '../fs/union.dylib', 'union_prelink.dylib')
    run('touch', tmp('union_prelink.dylib'))
    shell('mkdir', '-p', tmp('root/boot'))
    for a in ['Applications', 'bin', 'Library', 'private/etc', 'sbin', 'System', 'usr']:
        shell('mkdir', '-p', tmp('root/private/var/null/'+a))
    for a, b in [('mount_nulls', ROOT+'/starstuff/mount_nulls'), ('union_prelink.dylib', tmp('union_prelink.dylib')), ('untether', tmp('../catalog/untether')), ('white_loader', ROOT+'/white/white_loader')]:
        shell('ln', '-nfs', b, tmp('root/boot/'+a))
    run('gnutar', 'chvf', tmp('starstuff.tar'), '-C', tmp('root'), '.', '--owner', '0', '--group', 0, '--exclude', '.ignore')
    xz = '%s/starstuff/starstuff_%s_%s.tar.xz' % (ROOT, device, build_num)
    run('sh', '-c', 'xz -c "%s" > "%s"' % (tmp('starstuff.tar'), xz))

def stage():
    armv6_devices = ['iPhone1,2', 'iPod1,1', 'iPod2,1']
    install() 
    goto('.')
    #shell('sh', '-c', 'rm -f pdf/*.pdf starstuff/*.xz')
    goto('bs')
    available = set(i[i.find('_')+1:] for i in glob.glob('*_*'))
    succeeded = []
    failed = []
    for version in available:
        for basetype in ['iPod', 'iPhone', 'iPad']:
            goto('bs')
            firmwares = glob.glob(basetype + '*_' + version)
            arch_firmwares = [filter(lambda a: a in armv6_devices == is_armv7, firmwares) for is_armv7 in [False, True]]

            for i, stage in enumerate([
                ['iPhone3,1', 'iPhone3,3', 'iPod4,1', 'iPad2,1'],
                ['iPhone2,1', 'iPod3,1', 'iPad1,1', 'iPhone1,2', 'iPod2,1'],
            ]):
                eligible = []
                outpdf = '%s/pdf/%s_%s_%d.pdf' % (ROOT, basetype, version, i)
                for firmware in firmwares:
                    if '4.1' in firmware or '4.0' in firmware: continue # XXX
                    device = firmware[:firmware.find('_')]
                    if device in stage:
                        print '** Building %s for %s' % (firmware, outpdf)
                        set_firmware(firmware, True)
                        try:
                            starstuff()
                            if any(af[:1] == [firmware] for af in arch_firmwares):
                                catalog_dejavu()
                                goto('catalog')
                                eligible.append(tmp('catalog.txt'))
                            succeeded.append(firmware)
                        except Exception, e:
                            print '** Failed: %s' % str(e)
                            failed.append(firmware)
                if eligible == []: continue
                pdf(eligible)
                shutil.copy(tmp('out.pdf'), outpdf)
    print '** Done...'
    print 'succeeded:', succeeded
    print 'failed:', failed

def foo():
    run('touch', 'foo')

def lndir():
    set_firmware(lndir=True)

def clean():
    goto('.')
    for d in ['data', 'datautils0', 'white']:
        make(d, '', 'clean')
    autoclean()

main()
