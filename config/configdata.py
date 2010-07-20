'.base': {
    '#kern': {
        'vnode_patch':  '@ - 08 00 10 00', # must be 1st result
        'ovbcopy':      '+_ovbcopy',
        'scratch':      '!',

        'patch4':       '-_PE_i_can_has_debugger',
        'patch4_to':    0x47702001,

        # Do this just long enough to setuid(0) and sysctl
        # TODO arm on armv6?
        'patch_suser': '-_suser',
        'patch_suser_to': 0x47702000,
        'patch_suser_orig': '*(-_suser)',

        'patch_proc_enforce': '$proc_enforce',
        'patch_proc_enforce_to': 0,

        #'patch_sandbox': '- f0 b5 03 af 2d e9 00 0d c1 b0 82 46',
        #'patch_sandbox_to': 0x47702000,
        #'patch_sandbox_orig': 0xaf03b5f0,

        'mac_policy_list': '*(-_mac_label_get-4)',

        # for pmap.c
        'kernel_pmap':  '-_kernel_pmap',
        'mem_size':     '-_mem_size',
    },
},
'.armv7': {
    '<':            '.base',
    'arch':         'armv7',

    '#launchd': {
        # mov r0, #1; bx lr
       -1:              '@ + 01 20 70 47',
        # ldr r0, [r0] -> _launch_data_new_errno
        0:              '+ 00 68 .. .. .. .. 22 46 01 34',
        # lsr r0, r0, #2 -> _setrlimit
        1:              '60 69 29 46 + 80 08',
        # add r0, #3 -> __exit
        2:              'b0 f1 ff 3f .. .. + 03 30 16 f0',
        # ldmia r0, {r0-r3} -> _audit_token_to_au32
        3:              '8d e8 0f 00 0c f1 14 00 + 0f c8',
        # str r2, [sp, #4] -> _launch_data_unpack
        4:              '02 98 00 93 59 46 13 46 + 01 92',
        # str r3, [sp, #8] -> _launch_data_dict_iterate
        5:              '6a 46 01 93 01 33 + 02 93',
        # pop {r4, r7, pc}
        6:              '@ + 90 bd',
        # sub.w sp, r7, #0xc; pop {r4-r7, pc}
        7:              '@ + a7 f1 0c 0d f0 bd',
    },
},

'.armv7_3.2+': {
    '<': '.armv7',
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
        # blx r4; pop {r4, r5, r7, pc}
        'k12': '@ + a0 47 b0 bd',
        # blx r4; sub sp, r7, #4; pop {r4, r7, pc}'
        'k17': '@ + a0 47 a7 f1 04 0d 90 bd',

        # add r5, sp, #168; pop {r0, r1, r3, pc}
        'k13': '@ + 2a ad 0b bd',
        # ldr r0, [r5, r0]; pop {r4, r5, r7, pc}
        'k14': '@ + 28 58 b0 bd',

        # str r4, [r0]; pop {r4, r7, pc}
        'k15': '@ + 40 f8 04 4b 90 bd',

        # ldr r0, [r4]; pop {r4, r7, pc}
        'k16': '@ + 20 68 90 bd',

        # for installui.m
        'ft_path_builder_create_path_for_glyph': '+_ft_path_builder_create_path_for_glyph',
    },
    '#kern': {
        'storedude': '+ 43 6a 00 20 13 60 70 47',

        'patch1':       '- 02 0f .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03 1e 93',
        'patch1_to':    0x46c00f02,
        'patch3':       '61 1c 13 22 .. 4b 98 47 00 .. -',
        'patch3_to':    0x1c201c20,
        
        # It would be better to patch setup_kmem itself but this is easier.
        'patchkmem0':   '1b 68 00 2b - .. .. .. .. .. .. 04 f1 08 05',
        'patchkmem0_to': 0x46c046c0,
        # Note: the 1a 68 in tense3 is only because it was already patched
        # It is actually 1b 68, but I'm going to be lazy here
        'patchkmem1':   '.. 68 - 00 2b .. .. a3 68 2a 4a',
        'patchkmem1_to': 0x46c046c0,

        'patch_cs_enforcement_disable': '~ @ 00 00 00 00 00 00 00 - 00 00 00 00 01 00 00 00 80',
        'patch_cs_enforcement_disable_to': 1,

        # search for bic.*0xc00, look 
        'patch_nosuid': 'c8 f8 4c 30 d6 f8 88 30 db 6b - 13 f0 08 0f',
        'patch_nosuid_to': 0x0f00f013, # tst r3, #0

        'e1': '+ 01 98 a7 f1 18 0d bd e8 00 0d f0 bd',
        'e2': '+ ca f8 0c 80 bd e8 00 05 f0 bd',
        'e3': '+ .. .. .. .. 00 90 07 48 e0 47 a7 f1 00 0d 80 bd',
        'e4': '+ a7 f1 04 0d 90 bd',
    },
},

'iPad1,1_3.2': {
    '<': '.armv7_3.2+',
    '#kern': {
        'vram_baseaddr': 0xed6ed000 + 1024*768*4*2,
        'vram_baseaddr_atboot': 0xed6ed000 + 1024*768*4,
        
        # From old configdata.
        #'patch2':       0xc025dc8c, _mac_proc_enforce
        #'patch5':       0xc023fac0, _cs_enforcement_disable
    },
    '#cache': {
        'magic_offset': -960,
    }
},
'iPad1,1_3.2_self': {
    '<': 'iPad1,1_3.2',
    '#cache': { '@binary': '/System/Library/Caches/com.apple.dyld/dyld_shared_cache_armv7', },
    '#kern': { '@binary': '/var/mobile/ipadkern', },
    '#launchd': { '@binary': '/sbin/launchd', },
},

'iPhone3,1_4.0': {
    '<': '.armv7_3.2+',
    '#cache': {
        'magic_offset': -964,
    },
    '#kern': {
        'vram_baseaddr': 0xd35e9000 + 640*960*4*3,
        # ???
        'vram_baseaddr_atboot': 0xd35e9000 + 640*960*4,
        'patch3':       '70 46 13 22 .. 4b 98 47 00 .. -',
    },
},
'iPhone3,1_4.0.1': { '<': 'iPhone3,1_4.0', },
