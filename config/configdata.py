'.armv7': {
    'arch': 'armv7',
    '#cache': {
        # ldr r0, [r0]; pop {r4, r5, r7, pc}
        'k4': '@ + 00 68 b0 bd',
        # str r0, [r4]; pop {r4, r7, pc}
        'k5': '@ + 20 60 90 bd',
        # add r0, r4; pop {r4, r7, pc}
        'k6': '@ + 20 44 90 bd',
        # pop {r0-r3, pc} (ARM!)
        'k7': '@ - 0f 80 bd e8',
        # pop {r1, r2, pc}
        'k8': '@ + 06 bd',
        # pop {r1, r2, r3, pc}
        'k9': '@ + 0e bd',
        # sub sp, r7, #0; pop {r7, pc}
        'k10': '@ + a7 f1 00 0d 80 bd',
        # pop {r4-r7, pc}
        'k11': '@ + f0 bd',
        # blx r4; pop {r4, r7, pc}
        'k12': '@ + a0 47 90 bd',
    }
},

'iPad1,1_3.2': {
    '<': '.armv7',
    '#cache': {
        # movs r2, #0; movs r0, r2; pop {pc}
        'k1': '@ + 00 22 10 46 00 bd',
        # add r0, sp; pop {r3, r6, pc}
        'k2': '@ + 68 44 48 bd',
        # subs r0, r3; pop {r4, r7, pc}
        'k3': '@ + c0 1a 90 bd',

        '@binary': '../dsc/iPad1,1_3.2.cache',
        '@syms': '../dsc/syms/iPad1,1_3.2.db',
    },
},

'iPhone3,1_4.0': {
    '<': '.armv7',
    '#cache': {
        # add r5, sp, #896; lsrs r6, #11; pop {r1-r3, pc}
        'k13': '@ + e0 ad f6 0a 0e bd', 
        # mov r0, r5; pop {r4, r5, pc}
        'k14': '@ + 28 46 30 bd',
        # add r0, r4; pop {r4, r7, pc}
        'k15': '@ + 22 40 90 bd',
        '@binary': '../dsc/iPhone3,1_4.0.cache',
        '@syms': '../dsc/syms/iPhone3,1_4.0.db',
    },
},
