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
    if input is None: input = [filename]
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
        run(gcc, '-o', output + '_', objs, ldflags)
        if ldid:
            run_multiple(['cp', output + '_', output],
                         ['strip', '-Sx', output],
                         ['ldid', '-S' + ent, output])
        else:
            run_multiple(['cp', output + '_', output],
                         ['strip', '-Sx', output])
    else:
        run(gcc, '-o', output, objs, ldflags)

def pf2():
    goto('pf2')
    compile_stuff(['pf2.c', '../sandbox2/sandbox.S'], 'pf2')

def chain():
    goto('chain')
    cf = ['-marm', '-DDEBUG=0', '-DDEBUG_VERBOSE=0', '-DHAVE_SERIAL=0', '-DUSE_ASM_FUNCS=1', '-fblocks']
    ldf=['-dynamiclib', '-nostdlib', '-nodefaultlibs', '-lgcc', '-undefined', 'dynamic_lookup', '-read_only_relocs', 'suppress']
    compile_stuff(['start.s', 'chain.c', 'dt.c', 'stuff.c', 'fffuuu.S', 'bcopy.s', 'bzero.s', 'what.s'], 'chain-kern.dylib', cflags=cf, ldflags=ldf, strip=False)
    compile_stuff(['chain-user.c'], 'chain-user')

data_common_files = ['binary.c', 'find.c', 'common.c']
data_files = data_common_files + ['data.c']
data_upgrade_files = data_files + ['cc.c', 'lzss.c']
white_loader_files = data_common_files + ['white_loader.c']

def data():
    goto('data')
    compile_stuff(data_upgrade_files, 'data', 'ent.plist', cflags='-DIMG3_SUPPORT')

def upgrade_data():
    data_upgrade()
    goto('upgrade-data')
    compile_stuff(['lol_mkdir.c'], 'lol_mkdir', '../data/ent.plist')
    run('./build-deb.sh')

def data_native():
    goto('data')
    compile_stuff(data_upgrade_files, 'data', gcc=GCC_NATIVE, ldid=False, cflags='-DIMG3_SUPPORT')

def white_loader():
    goto('data')
    compile_stuff(white_loader_files, 'white_loader')

def pf():
    goo_pf()
    pf2()

def clean():
    autoclean()

main()
