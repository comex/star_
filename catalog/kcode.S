.syntax unified
.arm
start:
    ldr sp, [sp, #(0x18 - 0x70)]
    sub sp, #0x18

    adr r0, inspiring
    bl _IOLog

    mov r8, #0; b end

    adr r5, stuff
    ldr r8, count
pf_loop:

    ldm r5, {r0, r2}
    add r1, r5, #8
    bl _memcpy
    ldm r5, {r0, r1}
    mov r2, #0
    bl _flush_dcache
    ldm r5!, {r0, r1}
    add r5, r1
    mov r2, #0
    bl _invalidate_icache
    
    subs r8, #1
    bne pf_loop

end:
    bl _IOLockAlloc
    str r0, [r6, #0x2d8]
    str r8, [r6, #0x238]
    str r8, [r6, #0x2d4]
    # fix up linked lists
    add r0, r6, #0x218
    str r0, [r6, #0x21c]
    add r0, r6, #0x220
    str r0, [r6, #0x220]
    str r0, [r6, #0x224]
#if 0

    ;# stop and start (94, 93)
    ldr r5, [r6]
    mov r0, r6
    ldr r1, [r6, #0x294]
    mov lr, pc
    ldr pc, [r5, #(94*4)]

    mov r0, r6
    ldr r1, [r6, #0x294]
    mov lr, pc
    ldr pc, [r5, #(93*4)]
#endif

    pop {r8, r10, r11}
    pop {r4-r7, pc}

inspiring: .asciz "I exist!\n"
.align 2

count: .long 0
stuff:
