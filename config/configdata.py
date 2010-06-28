'.armv7': {
    '<': '.base',
    'arch': 'armv7',
    'cache': {
        # movs r2, #0; movs r0, r2; pop {pc}
        'k1': '@ + 00 22 10 46 00 bd',
        # add r0, sp; pop {r3, r6, pc}
        'k2': '@ + 68 44 48 bd',
        # subs r0, r3; pop {r4, r7, pc}
        'k3': '@ + c0 1a 90 bd',
        # ldr r0, [r0]; pop {r4, r5, r7, pc}
        'k4': '@ + 00 68 b0 bd',
        # str r0, [r4]; pop {r4, r7, pc}
        'k5': '@ + 20 60 90 bd',
        # add r0, r4; pop {r4, r7, pc}
        'k6': '@ + 20 44 90 bd',
        # pop {r0-r3, pc} (ARM!)
        'k7': '@ - 0f 80 bd e8',
        # pop {r1, r2, pc}
        'k8': '@ + 06 bd bf 06',
        # pop {r1, r2, r3, pc}
        'k9': '@ + 0e bd 84 05',
        # sub sp, r7, #0; pop {r7, pc}
        'k10': '@ + a7 f1 00 0d 80 bd',
        # pop {r4-r7, pc}
        'k11': '@ + f0 bd',
        # blx r4; pop {r4, r7, pc}
        'k12': '@ + a0 47 90 bd',
    }
}
