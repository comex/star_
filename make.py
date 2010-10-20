#!/usr/bin/env python
from fabricate import *
import fabricate
fabricate.default_builder.deps # do this before we chdir
import sys, os
ROOT = os.path.realpath(os.path.dirname(sys.argv[0]))
sys.path.append(ROOT + '/config')
os.environ['PYTHONPATH'] = ':'.join([(ROOT + '/config'), (ROOT + '/goo')])

import config
cfg = config.openconfig()

SDK = '/var/sdk'
BIN = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin'
GCC_BIN = BIN + '/gcc-4.2'
GCC_BASE = [GCC_BIN, '-Werror', '-Os', '-Wimplicit', '-isysroot', SDK, '-F'+SDK+'/System/Library/Frameworks', '-F'+SDK+'/System/Library/PrivateFrameworks', '-I', ROOT, '-fno-blocks']
GCC = [GCC_BASE, '-arch', cfg['arch'], '-mthumb']
GCC_UNIVERSAL = [GCC_BASE, '-arch', 'armv6', '-arch', 'armv7']
GCC_SUMMONED = ['/Users/comex/arm-none-eabi/bin/arm-none-eabi-gcc', '-mthumb', '-march='+cfg['arch'], '-Os']
STRIP_SUMMONED = '/Users/comex/arm-none-eabi/bin/arm-none-eabi-strip'
GCC_NATIVE = 'gcc'
GCC_FLAGS = ['-std=gnu99']
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
    run(GCC, '-mthumb', '-c', '-o', ofile, filename)
    run(ROOT + '/machdump/machdump', ofile, binfile)
    if os.path.exists(ofile): os.unlink(ofile)

def pmap():
    goto('star/pmap')
    for x in ['pmap2', 'pmaparb', 'shelltester']:
        run(GCC_UNIVERSAL, '-o', x, x + '.c', '-I', headers, '-std=gnu99', F('IOKit', 'CoreFoundation', 'IOSurface'))



def machdump():
    goto('machdump')
    run(GCC_NATIVE, '-o', 'machdump', 'machdump.c')

def install():
    goto('install')
    files = ['install.o', 'copier.o']
    for o in files:
        run(GCC_UNIVERSAL, '-I', HEADERS, '-std=gnu99', '-c', '-o', o, chext(o, '.c'))
    run(GCC_UNIVERSAL, '-dynamiclib', '-o', 'install.dylib', files, F('CoreFoundation', 'GraphicsServices'), '-L.', '-ltar', '-llzma')
    run('python', 'wad.py', 'install.dylib', 'Cydia-whatever.txz')

def installui():
    goto('installui')
    if not os.path.exists('dumpedUIKit'):
        run('./mkUIKit.sh')
        
    files = ['dddata.o', 'installui.o']
    for o in files:
        run(GCC, '-I', HEADERS, '-I', '.', '-std=gnu99', '-c', '-o', o, chext(o, '.m'))
    run(GCC, '-dynamiclib', '-o', 'installui.dylib', files, F('Foundation', 'UIKit', 'IOKit', 'CoreGraphics'), '-lz')

def goo():
    pass

def goo_pf():
    goo()
    goto('goo/pf')
    run('python', 'transe.py')
    run('python', '../one.py', 'transeboot.txt')

def compile_arm(objs, output, ent=''):
    for obj in objs:
        run(GCC, '-g', '-std=gnu99', '-c', '-o', obj, chext(obj, '.c'))
    run(GCC, '-g', '-std=gnu99', '-o', output + '_', objs)
    run_multiple(['cp', output + '_', output],
                 ['strip', '-Sx', output],
                 ['ldid', '-S' + ent, output])

def pf2():
    goto('pf2')
    compile_arm(['pf2.c', '../sandbox2/sandbox.S'], 'pf2')

data_common_objs = ['binary.o', 'find.o', 'common.o']
data_objs = data_common_objs + ['data.o', 'one.o', 'pf2.o']
white_loader_objs = data_common_objs + ['white_loader.o']
def data_prereq():
    # config for insane first
    goo_pf()
    pf2()
    goto('data')
    run('bash', '-c', 'cp ../goo/pf/one.dylib one.bin; xxd -i one.bin > one.c')
    run('bash', '-c', 'cp ../pf2/pf2 pf2.bin; xxd -i pf2.bin > pf2.c')

def data():
    data_prereq()
    compile_arm(data_objs, 'data', 'ent.plist')

def data_native():
    data_prereq()
    for obj in data_objs:
        run(GCC_NATIVE, '-g', '-std=gnu99', '-c', '-o', obj, chext(obj, '.c'))
    run(GCC_NATIVE, '-g', '-std=gnu99', '-o', 'data_native', data_objs)

def white_loader():
    goto('data')
    compile_arm(white_loader_objs, 'white_loader')

def pf():
    goo_pf()
    pf2()

def clean():
    autoclean()

main()
