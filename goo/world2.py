from world1 import *

def set_r0_to(r0):
    set_fwd('R4', r0)
    set_fwd('PC', cache['k21'])
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def set_r0to3(r0, r1, r2, r3):
    m = car()
    set_r0_to(m)
    set_fwd('PC', cache['k20'])
    heapadd(r0, r1, r2, r3, fwd('R7'), fwd('LR'), dontcare, dontcare)
    m._val = fwd('PC')
