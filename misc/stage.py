#!/usr/bin/env python
import os, sys, shutil, time
os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])) + '/..')
os.system('rm -f i*.pdf i*.pdf_FAILED')
os.system('make clean')
for plat in eval('{%s}' % open('config/configdata.py').read()).keys():
    if plat.startswith('.') or plat.endswith('_self'): continue
    outfn = plat + '.pdf'
    if os.system('config/config.py "%s" 2>/dev/null' % plat) != 0:
        print 'failed to config for', plat
        open(outfn + '_FAILED', 'w')
    elif os.system('make') != 0:
        print 'failed to make for', plat
        open(outfn + '_FAILED', 'w')
    else:
        shutil.copy('cff/out.pdf', outfn)
    time.sleep(1)
