import sys, os, re
import binary

tmp = '/tmp/tmp%d' % os.getpid()

b = binary.binary_open(sys.argv[1])
for startaddr, startoffset, size, prot in b.sects:
    addr = (startaddr + 0xfff) & ~0xfff
    while addr < (startaddr + size):
        f = open('%s.S' % (tmp,), 'w')
        print >> f, '.long 0x%x; .long 0x%x; .long 0x%x; .long 0x%x' % (b.deref(addr), b.deref(addr+4), b.deref(addr+8), b.deref(addr+12))
        f.close()
        assert 0 == os.system('arm-none-eabi-gcc -c -o %s.o %s.S' % (tmp, tmp))
        stuff = os.popen('arm-none-eabi-objdump -d %s.o' % (tmp,)).read()
        things = {}
        try:
            for thing in '048c':
                things[thing] = re.search(re.compile('^\s*' + thing + ':\s*(.*)$', re.M), stuff).group(1).replace('\t', ' ')
        except:
            pass
        else:
            print things

        addr += 0x1000
    
