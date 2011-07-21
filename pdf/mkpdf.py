import zlib, sys, re, struct
u = open(sys.argv[1], 'rb').read()
z = zlib.compress(u, 9)

m = re.search('currentfile eexec[\r\n]*', u)
encstart = m.end()
assert u[encstart:encstart+2] == '\x80\x02'
encend = encstart + 6 + struct.unpack('<I', u[encstart+2:encstart+6])[0]
zeroes = u.find('0000000', encend)

fmt = {}
fmt['length'] = len(z)
fmt['length1'] = encstart
fmt['length2'] = zeroes - encstart
fmt['length3'] = len(u) - zeroes
fmt['stream'] = z

pdf = open('out.pdf.template', 'rb').read().format(**fmt)
open(sys.argv[2], 'wb').write(pdf)

