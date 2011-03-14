import sys, struct
patchfp = open(sys.argv[1], 'rb')

def read(f, size):
    result = f.read(size)
    if len(result) != size: raise Exception('truncated')
    return result

print '#define P32(a, b) *((volatile unsigned int *) a) = b;'
print '#define P8(a, b) *((volatile unsigned char *) a) = b;'

while True:
    namelen = patchfp.read(4)
    if len(namelen) == 0: break
    if len(namelen) != 4: raise Exception('truncated')
    name = read(patchfp, struct.unpack('I', namelen)[0])
    addr, = struct.unpack('I', read(patchfp, 4))
    data = read(patchfp, struct.unpack('I', read(patchfp, 4))[0])
    if name == 'sysent patch':
        sysent_patch, = struct.unpack('I', data)
    elif name == 'sysent patch orig':
        sysent_patch_orig, = struct.unpack('I', data)
    elif name == 'scratch':
        scratch, = struct.unpack('I', data)
    if addr == 0 or len(data) == 0 or name.startswith('+'): # in place only
        continue
    
    print '// %s' % name
    for b in xrange(0, len(data) - 3, 4):
        print 'P32(0x%x, 0x%x);' % (addr + b, struct.unpack('I', data[b:b+4])[0])
    for b in xrange(b + 4, len(data)):
        print 'P8(0x%x, 0x%x);' % (addr + b, ord(data[b]))
