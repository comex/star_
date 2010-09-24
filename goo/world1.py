from goo import *

import config
cfg = config.openconfig()
cache = cfg['#dyld'] if cfg.has_key('#dyld') else cfg['#cache']
syms = config.binary_open(cache['@binary'])

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

def store_val(val, address):
    set_fwd('PC', cache['k17'])
    set_fwd('R5', val)
    set_fwd('R4', address)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R5'), fwd('R7'), fwd('PC'))

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
    # this should be better specified
    if isinstance(funcname, basestring):
        funcaddr = syms[funcname]
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
    mynewstackbase = heapaddr - 0x20
    # mynewstackbase [R7] [PC=k12] <- dirty zone <- [R4/arg4] [R5/arg5] ([R7/arg6] [PC=k18]) [R7=addr to go back to] [PC=k10]
    #                                                                   
    if len(args) <= 7 and cache.has_key('k12'):
        set_fwd('R4', funcaddr)
        set_fwd('R7', mynewstackbase)
        set_fwd('PC', cache['k10'])
        exhaust_fwd('R5', 'R7')
        store_val(cache['k12'], to=(mynewstackbase + 1*4))
        marker, markeroff = stackunkpair()
        if len(args) > 4:
            store_val(args[4], to=(mynewstackbase + 2*4))
        if len(args) > 5:
            store_val(args[5], to=(mynewstackbase + 3*4))
        if len(args) > 6:
            store_val(args[6], to=(mynewstackbase + 4*4))
            store_val(cache['k18'], to=(mynewstackbase + 5*4)
            store_val(markeroff, to=(mynewstackbase + 6*4))
            store_val(cache['k10'], to=(mynewtackbase + 7*4)
        else:
            store_val(markeroff, to=(mynewstackbase + 4*4))
            store_val(cache['k10'], to=(mynewtackbase + 5*4)
        # this is from k10
        heapadd(marker*0 + fwd('R7'), fwd('PC'))
    elif len(args) <= 6 and cache.has_key('k22'):
        # xxx this is identical but with no R5
        assert False
    else:
        assert False

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
