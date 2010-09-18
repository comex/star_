import mmap, os, struct, re, sys
import config
cfg = config.openconfig()

filename = cfg['#kern']['@binary']
fp = open(filename, 'rb')
stuff = mmap.mmap(fp.fileno(), os.path.getsize(filename), prot=mmap.PROT_READ)
m = config.macho(filename, stuff)

out = open('nm.ld', 'w')

out.write('''SECTIONS {
    . = 0xf0000000;
    .init : { *(.init) }
    /DISCARD/ : { *(.comment); *(.ARM.attributes) }
}
''')
      
for a, b in m.get_syms().items():
    if a.startswith('_') and re.match('^[a-zA-Z0-9_@\$]{2,}$', a):
        print >> out, '%s = 0x%08x;' % (a[1:], b)
