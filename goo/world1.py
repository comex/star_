from goo import *

import config
cfg = config.openconfig()
cache = cfg['#dyld'] if cfg.has_key('#dyld') else cfg['#cache']
syms = config.binary_open(cache['@binary'])

# useful addresses for debugging (iPad1,1_3.2):
# 0x30a8846e end of c_ch
# 0x30a88a70 index
# 0x30a8876e drop
# 0x30a8a074 just before popout

#POP.W   {R8,R10,R11}
#POP     {R4-R7,PC}

# stage1: memcpy stack <- r1+something

def load_r0_base_sp_off(off): 
    # note blah: "add r0, sp, #600" is based off of the SP now
    # sp_off() is based off of the SP-off of R4, which (with make_r4_avail) is 8 less
    # this is a hack.
    make_r4_avail()
    exhaust_fwd('R7')
    set_fwd('PC', cache['k13'])
    set_fwd('R4', (off - 8 - 600) - sp_off())
    heapadd(fwd('R1'), dontcare, cache['k14'],
            fwd('R4'), fwd('R7'), fwd('PC'))

def load_r0_r0():
    set_fwd('PC', cache['k4'])
    exhaust_fwd('R4', 'R5', 'R7')
    heapadd(fwd('R4'), fwd('R5'), fwd('R7'), fwd('PC'))

def load_r0_from(address):
    set_fwd('PC', cache['k16'])
    set_fwd('R4', address)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def store_r0_to(address):
    set_fwd('PC', cache['k5'])
    set_fwd('R4', address)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def add_r0_const(addend):
    set_fwd('PC', cache['k6'])
    set_fwd('R4', addend)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def set_r0to3(r0, r1, r2, r3):
    set_fwd('PC', cache['k7'])
    heapadd(r0, r1, r2, r3, fwd('PC'))

def set_r1to3(r1, r2, r3):
    set_fwd('PC', cache['k9'])
    heapadd(r1, r2, r3, fwd('PC'))

def set_sp(sp):
    set_fwd('PC', cache['k10'])
    set_fwd('R7', sp)
    clear_fwd() # pop {r7, pc} but that's not in this stack

# Make some registers available but do nothing.
def make_avail():
    set_fwd('PC', cache['k11'])
    heapadd(*(fwd(i, True) for i in ['R4', 'R5', 'R6', 'R7', 'PC']))

# Make only r4 available.
def make_r4_avail():
     set_fwd('PC', cache['k18'])
     exhaust_fwd('R4')
     heapadd(fwd('R4'), fwd('PC'))

def funcall(funcname, *args, **kwargs):
    if isinstance(funcname, basestring):
        funcaddr = syms[funcname]
        #if funcname != '_execve': funcaddr |= 1
        #if funcaddr & 1 == 0:
        #    print funcname, 'non-thumb'
    else:
        funcaddr = funcname
    while len(args) < 4: args += (dontcare,)
    if args[0] is None:
        set_r1to3(args[1], args[2], args[3])
    else:
        set_r0to3(args[0], args[1], args[2], args[3])
    if kwargs.get('load_r0'):
        load_r0_r0()
        del kwargs['load_r0']
    assert kwargs == {}
    if len(args) <= 7:
        set_fwd('PC', cache['k12'])
        set_fwd('R4', funcaddr)
        exhaust_fwd('R5', 'R7')
        heapadd(fwd('R4'), fwd('R5'), fwd('R7'), fwd('PC'))
        if len(args) > 4:
            set_fwd('R4', args[4])
        if len(args) > 5:
            set_fwd('R5', args[5])
        if len(args) > 6:
            set_fwd('R7', args[6])
    else:
        die
        #set_fwd('PC', cache['k17'])
        #set_fwd('R4', funcaddr)
        #unk = stackunkwrapper(fwd('R4'))
        #unkptr = stackunkptr(unk)
        #set_fwd('R7', unkptr + 4)
        #heapadd(*args[4:])
        #heapadd(unk, fwd('R7'), fwd('PC'))

def store_to_r0(value):
    set_fwd('R4', value)
    exhaust_fwd('R7')
    set_fwd('PC', cache['k15'])
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))


def store_deref_plus_offset(deref, offset, value):
    # [[deref],offset] = value
    load_r0_from(deref)
    add_r0_const(offset)
    store_to_r0(value)
