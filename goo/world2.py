from world1 import *
import world1

def load_r0_from(address):
    set_fwd('PC', cache['k23'])
    set_fwd('R4', address - 8)
    exhaust_fwd('R5', 'R7')
    heapadd(fwd('R4'), fwd('R5'), fwd('R7'), fwd('PC'))

def set_r0_to(r0):
    set_fwd('R4', r0)
    exhaust_fwd('R7')
    set_fwd('PC', cache['k21'])
    heapadd(fwd('R4'), fwd('R7'), fwd('PC'))

def set_r0to3(r0, r1, r2, r3):
    m = car()
    set_r0_to(m)
    set_fwd('PC', cache['k20'])
    exhaust_fwd('R7', 'LR')
    heapadd(r0, r1, r2, r3, fwd('R7'), fwd('LR'), dontcare, dontcare)
    m._val = fwd('PC')

def load_r0_r0():
    set_fwd('PC', cache['k24'])
    exhaust_fwd('R7')
    heapadd(fwd('R7'), fwd('PC'))

world1.set_r0to3 = set_r0to3
world1.load_r0_r0 = load_r0_r0
