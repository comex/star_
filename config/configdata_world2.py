'.armv7': {
    '#dyld': {
       # str r0, [r4]; pop {r4, r7, pc}
       'k5': '@ + 20 60 90 bd',
       # skip k10 k18 k12 k13 k14 
       # pop {r4-r7, pc}
       'k11': '@ + f0 bd',
       # str r4, [r0]; pop {r4, r7, pc}
       'k15': '@ + 04 60 90 bd',
       # pop {r7, pc}
       'k18': '@ + 80 bd',
       # mov r12, r0; pop {r0-r3, r7, lr}; add sp, #8; bx r12
       'k20': '@ - 00 c0 a0 e1 8f 40 bd e8 08 d0 8d e2 1c ff 2f e1',
       # mov r0, r4; pop {r4, r7, pc}
       'k21': '@ + 20 46 90 bd',
       # blx r4; pop {r4, r7, pc}
       'k22': '@ + a0 47 90 bd',
       # ldr r0, [r4, #8]; pop {r4, r5, r7, pc}
       'k23': '@ + a0 68 b0 bd',
       # ldr r0, [r0]; pop {r7, pc}
       'k24': '@ + 00 68 80 bd',
    }
},
'.armv7_4.x': {
    '<': '.armv7',
},
