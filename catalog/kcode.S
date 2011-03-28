.syntax unified
.arm
start:
    sub r7, sp, #(56 - 0xc)
    add sp, pc, #0xa00
    ldr r6, count
    adr r5, stuff
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
    
    subs r6, #1
    bne pf_loop

    mrc p15, 0, sp, c13, c0, 4
    ldr sp, [sp, #0x54]
sp_loop:
    ldr r0, [sp], #4
    cmp r0, r7
    bne sp_loop

    add sp, #(9*4)
    pop {r4-r7, pc}

count: .long 0
stuff:
