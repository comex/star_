'.base': {
    '#kern': {
        'vnode_patch':  '@ - 08 00 10 00', # must be 1st result
        'ovbcopy':      '+_ovbcopy',
        'scratch':      '!',

        'patch2_to':    0,
        'patch4':       '-_PE_i_can_has_debugger',
        'patch4_to':    0x47702001,
        'patch5_to':    1,
        'patch6_to':    1,

        # for pmap.c
        'kernel_pmap':  '-_kernel_pmap',
        'mem_size':     '-_mem_size',
    },
},
'.armv7_3.2+': {
    '<': '.base',
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
    },
    '#kern': {
        'storedude': '+ 43 6a 00 20 13 60 70 47',

        'patch1':       '% 02 0f .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03 1e 93',
        'patch1_to':    0x46c00f02,
        'patch3':       '61 1c 13 22 .. 4b 98 47 00 .. -',
        'patch3_to':    0x1c201c20,
        
        # It would be better to patch setup_kmem itself but this is easier.
        'patchkmem0':   '1b 68 00 2b - .. .. .. .. .. .. 04 f1 08 05',
        'patchkmem0_to': 0xc046c046,
        # Note: the 1a 68 in tense3 is only because it was already patched
        # It is actually 1b 68, but I'm going to be lazy here
        'patchkmem1':   '.. 68 00 2b - .. .. a3 68 2a 4a',
        'patchkmem1_to': 0x68a3c046,
    },
},

'iPad1,1_3.2': {
    '<': '.armv7_3.2+',
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
    '#kern': {
        '@binary': '/Users/comex/share/ipadkern',
        'vram_baseaddr': 0xed6ed000 + 1024*768*4*2,
        
        # From old configdata.
        'patch2':       0xc025dc8c,
        'patch5':       0xc023fac0,
    },
},

'iPhone3,1_4.0': {
    '<': '.armv7_3.2+',
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
    '#kern': {
        '@binary': '/Users/comex/share/tense3',
        
        'patch3':       '70 46 13 22 .. 4b 98 47 00 .. -',
    },
},
