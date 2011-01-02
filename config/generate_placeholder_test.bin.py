import re, os, sys, struct
os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])))

defs = {}

for line in open('config_asm.h'):
    m = re.match('#define ([^ ]*) 0x(.*)', line)
    if m:
        defs[m.group(1)] = int(m.group(2), 16)

pt = open('placeholder_test.bin', 'wb')

for cfg in set(re.findall('(CONFIG_[A-Z0-9_]+)', open('../datautils/deplaceholder.c').read())):
    pt.write(struct.pack('I', defs[cfg]))
