from goo import *
import dmini

def load_r0_r0():
    set_fwd('PC', dmini.cur.find_basic('+ 00 68 80 bd'))
    exhaust_fwd('R7')
    heapadd(fwd('R7'), fwd('PC'))

def load_r0_from(address):
    set_fwd('PC', dmini.cur.find_multiple('+ 20 68 90 bd', '- 00 00 94 e5 90 80 bd e8'))
    set_fwd('R4', address)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def store_r0_to(address):
    set_fwd('PC', dmini.cur.find_multiple('+ 20 60 90 bd', '- 00 00 84 e5 90 80 bd e8'))
    set_fwd('R4', address)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def store_val_to(val, to):
    set_fwd('PC', dmini.cur.find_multiple('+ 25 60 b0 bd', '- 00 50 84 e5 b0 80 bd e8'))
    set_fwd('R5', val)
    set_fwd('R4', to)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R5'), fwd('R7'), fwd('PC'))

def add_r0_by(addend):
    set_fwd('PC', dmini.cur.find_multiple('+ 20 44 90 bd', '- 00 00 84 e0 90 80 bd e8'))
    set_fwd('R4', addend)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def set_r0_to(r0):
    set_fwd('PC', dmini.cur.find_multiple('+ 20 46 90 bd', '- 04 00 a0 e1 90 80 bd e8'))
    set_fwd('R4', r0)
    exhaust_fwd('R7')
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def set_r0to3(r0, r1=0, r2=0, r3=0):
    try:
        set_fwd('PC', dmini.cur.find_multiple('+ 0f bd', '- 0f 80 bd e8'))
    except dmini.cur.DminiError:
        set_r0_to(r0)
        set_r1to3(r1, r2, 3)
    else:
        heapadd(r0, r1, r2, r3, fwd('PC'))

def set_r1to3(r1, r2, r3):
    exhaust_fwd('R7')
    set_fwd('PC', dmini.cur.find_basic('+ 8e bd'))
    heapadd(r1, r2, r3, fwd('R7'), fwd('PC'))

def set_sp_to(sp):
    set_fwd('PC', dmini.cur.find_multiple('+ a7 f1 00 0d 80 bd', '- 00 d0 47 e2 80 80 bd e8'))
    set_fwd('R7', sp)
    clear_fwd() # pop {r7, pc} but that's not in this stack

# Make some registers available but do nothing.
def make_avail():
    set_fwd('PC', dmini.cur.find_basic('+ f0 bd'))
    heapadd(*(fwd(i, True) for i in ['R4', 'R5', 'R6', 'R7', 'PC']))

# Make only r4 available.
def make_r4_avail():
     set_fwd('PC', dmini.cur.find_basic('+ 10 bd'))
     exhaust_fwd('R4')
     heapadd(fwd('R4'), fwd('PC'))

def funcall(funcaddr, *args, **kwargs):
    if isinstance(funcaddr, basestring): # actually a symbol
        funcaddr = dmini.cur.sym(funcaddr)
        
    # This wastes a lot of space!  I should make the old behavior an option.
    while len(args) < 4: args += (dontcare,)
    if args[0] is None:
        set_r1to3(args[1], args[2], args[3])
    else:
        set_r0to3(args[0], args[1], args[2], args[3])
    if kwargs.get('load_r0'):
        load_r0_r0()
        del kwargs['load_r0']
    assert kwargs == {}

    try:
        m = marker()
    except:
        # lame way
        assert len(args) <= 7
        set_fwd('PC', dmini.cur.find_multiple('+ a0 47 b0 bd', '- 34 ff 2f e1 b0 80 bd e8'))
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
        set_fwd('PC', dmini.cur.find_multiple('+ a0 47 a7 f1 04 0d 90 bd', '??'))
        set_fwd('R4', funcaddr)
        set_fwd('R7', m + 4)
        heapadd(*args[4:])
        m.mark()
        heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def store_to_r0(value):
    set_fwd('R4', value)
    exhaust_fwd('R7')
    set_fwd('PC', dmini.cur.find_multiple('+ 40 f8 04 4b 90 bd', '- 00 40 80 e5 90 80 bd e8'))
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def store_deref_plus_offset(deref, offset, value):
    # [[deref],offset] = value
    load_r0_from(deref)
    add_r0_const(offset)
    store_to_r0(value)


