'all': {
    'iPhone3,1_4.0.1': { '<': 'iPhone3,1_4.0', },
    'iPhone1,x_4.0': {
        '<': '.armv6_4.x',
        '#kern': {
            'vram_baseaddr': 0xca67d000,     
        },
    },
    'iPhone1,x_4.0.1': {
        '<': '.armv6_4.x',
        '#kern': {
            'vram_baseaddr': 0xca67d000, 
        },
    },
    'iPhone2,1_4.0': {
        '<': '.armv7_4.x',
        '#kern': {
            'vram_baseaddr': 0xcd32d000, 
        },
    },
    'iPhone2,1_4.0.1': {
        '<': '.armv7_4.x',
        '#kern': {
            'vram_baseaddr': 0xcd32d000,    
        },
    },
    'iPod2,1_4.0': {
        '<': '.armv6_4.x',
        '#kern': {
            'vram_baseaddr': 0xca68d000, 
        },
    },
    'iPhone2,1_3.1.3': {
        '<': '.armv7_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed731000,       
        },
    },
    'iPhone2,1_3.1.2': {
        '<': '.armv7_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed731000,       
        },
    },
    'iPod2,1_3.1.2': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xeb849000,
        },
    },
    'iPod2,1_3.1.3': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed731000,
        },
    },
    'iPhone1,x_3.1.3': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': [0xeb811000, 0xeb809000],
        },
    },
    'iPhone1,x_3.1.2': {
        '<': 'iPhone1,x_3.1.3',
        '#kern': {
            'vram_baseaddr': [0xeb811000, 0xeb809000],
        },
    },
    'iPod3,1_3.1.3': {
        '<': '.armv7_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed615000, 
        },
    },
    'iPod3,1_3.1.2': {
        '<': '.armv7_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xed615000,       
        },
    },
    'iPod1,1_3.1.2': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xeb919000,   
        },
    },
    'iPod1,1_3.1.3': {
        '<': '.armv6_3.1.x',
        '#kern': {
            'vram_baseaddr': 0xeb919000,   
        },
    },
    'iPod3,1_4.0': {
        '<': '.armv7_4.x',
        '#kern': {
            'vram_baseaddr': 0xcd33d000, 
        },
    },
    'iPad1,1_3.2': {
        '<': '.armv7',
        '#kern': {
            'vram_baseaddr': [0xed6ed000, 0xed6f5000],
        },
        'kill_sb': 1,
    },
    'iPad1,1_3.2.1': {
        '<': 'iPad1,1_3.2',
    },

    'iPad1,1_3.2_self': {
        '<': 'iPad1,1_3.2',
        '#cache': { '@binary': '/System/Library/Caches/com.apple.dyld/dyld_shared_cache_armv7', },
        '#kern': { '@binary': '/var/mobile/ipadkern', },
        '#launchd': { '@binary': '/sbin/launchd', },
    },

    'iPhone3,1_4.0': {
        '<': '.armv7_4.x',
        '#kern': {
            'vram_baseaddr': 0xd2675000,
            # ???
            'patch3':       '70 46 13 22 .. 4b 98 47 00 .. -',
            'root_ios_id': 2,
        },
    },
    'iPod2,1_4.1': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPhone2,1_4.1': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPhone1,2_4.1': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPod3,1_4.1': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPhone3,1_4.1': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
    'iPod4,1_4.1': {
        '<': '.armv7',
        '#kern': {
        
        },
    },
},
'world1': {
    '.base': {
        '#kern': {
            'patch_cs_enforcement_disable': '~ @ 00 00 00 00 00 00 00 - 00 00 00 00 01 00 00 00 80',
            'patch_cs_enforcement_disable_to': 1,

            # for pmap.c
            #'kernel_pmap':  '-_kernel_pmap',
            #'mem_size':     '-_mem_size',

            'patch_kernel_pmap_nx_enabled': '*(-_kernel_pmap)+0x420',
            'patch_kernel_pmap_nx_enabled_to': 0,

            'sysent': '21 00 00 00 00 10 86 00 -',
            'sysent_patch': '<sysent>+4',
            'sysent_patch_orig': '*<sysent_patch>',

            'derive_vnode_path': r'!bof:!stringref:path',
            'derive_vnode_path_orig1': '*<derive_vnode_path>',
            'derive_vnode_path_orig2': '*(<derive_vnode_path>+4)',
            'derive_vnode_path_jumpto': '<derive_vnode_path>+9', # xxx thumb on armv6?
            
            'strncmp': '+_strncmp',
            'flush_dcache': '+_flush_dcache',
            'invalidate_icache': '+_invalidate_icache',
            'copyin': '+_copyin',

            'scratch': '!scratch',
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

            # ldmibmi r12, {sp, pc}
            # actually the tail half of a ldr.w r12, [r1, r0] + ldr r1, [pc, #620] 
            'k-1': '@ % - 00 a0 9b 49',

            # and.w r0, #1; pop {r7, pc}
            'k3': '@ + 00 f0 01 00 80 bd',

            # ldr.w r0, [r4, r0, lsl #2]; pop {r4, r5, r7, pc}
            'k2': '@ + 54 f8 20 00',
        },

        '#kern': {
            # vm_map and AMFI
            'patch1':       '- 02 0f .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03 1e 93',
            'patch1_to':    0x46c00f02,
            'patch3':       '23 78 9c 45 05 d1 .. .. .. .. .. .. .. 4b 98 47 00 .. -',
            'patch3_to':    0x1c201c20,

            # search for bic.*0xc00, or vnode_authorize
            #'patch_nosuid': 'd6/d8 f8 88 30 db 6b - 13 f0 08 0f',
            #'patch_nosuid_to': 0x0f00f013, # tst r3, #0

            # the actual check for 0
            'patch_tfp0': '85 68 00 23 .. 93 .. 93 - 5c b9',
            'patch_tfp0_to': 0x46c0e00b,
        },
    },

    '.armv7_4.x': {
        '<': '.armv7',
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
    '.armv7_4.x': {
        '<': '.armv7',
    },
},
'world3': {
    '.armv7': { '#cache': {} },
    '.armv7_4.x': { '<': '.armv7', },
},
