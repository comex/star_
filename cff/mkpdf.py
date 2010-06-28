import zlib
z = zlib.compress(open('out.cff').read())[2:-4]
pdf = open('out.pdf.template').read()
pdf = pdf.replace('XXX', 'x\x01' + z)
open('out.pdf', 'w').write(pdf)

