# This whole file is just for debugging
.globl _fffuuu_start, _fffuuu_end
.text
.arm
_fffuuu_start:
    cpsid if

    mov r6, r3 ;#!
    mov r7, r1

    mov r4, #0x80000020 ;# buffer

    mov r2, lr
    bl print_hex_r2
    mrc p15, 0, r2, c6, c0, 2 ;# DFAR=0 IFAR=2
    bl print_hex_r2
    mrc p15, 0, r2, c5, c0, 1 ;# DFSR=0 IFSR=1
    bl print_hex_r2
    #mrc p15, 0, r2, c1, c0, 0 ;# SCTLR
    #bl print_hex_r2
    mrs r2, SPSR
    bl print_hex_r2
    mov r2, r6
    bl print_hex_r2
    mov r2, r7
    bl print_hex_r2

#if 0
    mov r10, #0x4
    mov r11, #0x5
    cpsid i, #0x12
    mov r10, sp
    mov r11, lr
    cpsid i, #0x17
    mov r2, r10
    bl print_hex_r2
    mov r2, r11
    bl print_hex_r2
    mov r2, sp
    bl print_hex_r2
#endif

#if 0
    # mov r5, #19
    ldr r6, [r6, #0x34]
    mov r5, #32
panic_loop:
    # rsb r2, r5, #19
    # bl print_hex_r2
    ldr r2, [r6], #4
    bl print_hex_r2
    mov r1, #10 ;# \n
    strb r1, [r4], #1

    subs r5, #1
    bne panic_loop
#endif

    mov r1, #46 ;# .
    strb r1, [r4], #1

    # flush the dcache for the data I'm sending
    mov r0, #0x80000000
flush_loop:
    mcr p15, 0, r0, c7, c14, 1
    cmp r0, r4
    add r0, #0x10
    bcc flush_loop

    # Put c0000000 -> 86100000 into the current page table
    mrc p15, 0, r0, c2, c0, 1
    bic r0, #0xff ;# close enough
    # p->v
    add r0, #0x40000000
    # pick the right entry
    add r0, #(4*0xc00)
    ldr r1, pte
    str r1, [r0]
    # is this necessary? flush dcache
    mcr p15, 0, r0, c7, c14, 1
    # Invalidate unified, data TLBs; DSB
    mcr p15, 0, r0, c8, c7, 0
    mcr p15, 0, r0, c8, c6, 0
    mcr p15, 0, r0, c7, c10, 4

    # ok now actually start chatting
    mov r0, #0xc0000000
    add r0, #(0x900 + 3*0x20)

wait_loop:
    ldr r3, [r0]
    tst r3, #0x80000000
    bne wait_loop

    ;# # of bytes
    sub r3, r4, #0x80000020
    mov r4, #0x40000020
    str r4, [r0, #0x14]

    lsr r4, r3, #6
    add r4, #1
    lsl r4, #19

    orr r4, r3
    str r4, [r0, #0x10]
    ldr r1, [r0, #0]
    orr r1, r1, #0x84000000
    str r1, [r0, #0]
    
spin:
    b spin

print_hex_r2:
    mov r3, #8
loop:
    # actually convert to hex...
    lsr r1, r2, #28
    cmp r1, #0xa
    addlt r1, #48
    addge r1, #87
    
    strb r1, [r4], #1

    lsl r2, #4
    subs r3, #1
    bne loop
    
    mov r1, #32 ;# space
    strb r1, [r4], #1

    bx lr

pte: .long 0x86100c02 
_fffuuu_end:
