.syntax unified
.arm
start:
    ldr r6, [sp, #(0x14 - 0x68)]
    ldr sp, [sp, #(0x18 - 0x68)]

    sub sp, #0x18
    b end
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

    pop {r8, r10, r11}
    pop {r4-r7, pc}

count: .long 0
stuff:
