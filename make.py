#!/usr/bin/env python
# encoding: utf-8
from fabricate import *
import fabricate
fabricate.default_builder.deps # do this before we chdir
import sys, os
ROOT = os.path.realpath(os.path.dirname(sys.argv[0]))
sys.path.append(ROOT + '/config')
os.environ['PYTHONPATH'] = ':'.join([(ROOT + '/config'), (ROOT + '/goo')])

import config
cfg = config.openconfig()

GCC_FLAGS = ['-std=gnu99', '-gdwarf-2', '-Werror', '-Wimplicit', '-Os']
SDK = '/var/sdk'
BIN = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin'
GCC_BIN = BIN + '/gcc-4.2'
GCC_BASE = [GCC_BIN, GCC_FLAGS, '-isysroot', SDK, '-F'+SDK+'/System/Library/Frameworks', '-F'+SDK+'/System/Library/PrivateFrameworks', '-I', ROOT, '-fno-blocks', '-mapcs-frame', '-fomit-frame-pointer']
GCC = [GCC_BASE, '-arch', cfg['arch'], '-mthumb']
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

def compile_to_bin(filename):
    # requires machdump
    ofile = chext(filename, '.o')
    binfile = chext(filename, '.bin')
    run(GCC, '-c', '-o', ofile, filename)
    run(ROOT + '/machdump/machdump', ofile, binfile)
    if os.path.exists(ofile): os.unlink(ofile)

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

def compile_arm(files, output, ent='', cflags=[], ldflags=[]):
    objs = []
    for inp in files:
        obj = chext(os.path.basename(inp), '.o')
        objs.append(obj)
        if obj == inp: continue
        run(GCC, '-c', '-o', obj, inp, cflags)
    run(GCC, '-o', output + '_', objs, ldflags)
    run_multiple(['cp', output + '_', output],
                 ['strip', '-Sx', output],
                 ['ldid', '-S' + ent, output])

def pf2():
    goto('pf2')
    compile_arm(['pf2.c', '../sandbox2/sandbox.S'], 'pf2')

def chain():
    goto('chain')
    dbg = ['-DDEBUG=1', '-DACTUALLY_JUST_USE_PRAM=1']
    for c in ['chain.c', 'stuff.c']:
        run(GCC, dbg, '-c', '-o', chext(c, '.oo'), c)
        run('sh', '-c', 'sed "s/__TEXT/__LTXT/g; s/__DATA/__LDTA/g" ' + chext(c, '.oo') + ' >' + chext(c, '.o'))

    compile_arm(['chain-pf2.c', 'chain.o', 'stuff.o', 'fffuuu.S', 'bcopy.s', 'bzero.s'], 'chain', cflags=[dbg], ldflags=['-segaddr', '__LTXT', '0x08000000', '-segaddr', '__LDTA', '0x08002000'])

data_common_files = ['binary.c', 'find.c', 'common.c']
data_files = data_common_files + ['data.c', 'one.c', 'pf2.c']
data_upgrade_files = data_files + ['cc.c', 'lzss.c']
white_loader_files = data_common_files + ['white_loader.c']
def data_prereq():
    # config for insane first
    goo_pf()
    pf2()
    goto('data')
    run('sh', '-c', 'cp ../goo/pf/one.dylib one.bin; xxd -i one.bin > one.c')
    run('sh', '-c', 'cp ../pf2/pf2 pf2.bin; xxd -i pf2.bin > pf2.c')

def data():
    data_prereq()
    compile_arm(data_files, 'data', 'ent.plist')

def data_upgrade():
    data_prereq()
    compile_arm(data_upgrade_files, 'data', 'ent.plist', cflags='-DIMG3_SUPPORT')

def upgrade_data():
    data_upgrade()
    goto('upgrade-data')
    compile_arm(['lol_mkdir.c'], 'lol_mkdir', '../data/ent.plist')
    run('./build-deb.sh')

def data_native():
    data_prereq()
    for obj in data_objs:
        run(GCC_NATIVE, '-c', '-o', obj, chext(obj, '.c'))
    run(GCC_NATIVE, '-o', 'data_native', data_objs)

def white_loader():
    goto('data')
    compile_arm(white_loader_files, 'white_loader')

def pf():
    goo_pf()
    pf2()

def clean():
    autoclean()

main()
