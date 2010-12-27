.globl _putc_start, _putc_end
_putc_start:
.text
.arm
    ldr r1, buf_base
    ldr r2, [r1, #-4]
    cmp r2, #0x100
    bxhi lr
    strb r0, [r1, r2]
    add r2, #1
    str r2, [r1, #-4]
    cmp r0, #10
    bxne lr
    
    # zero the size
    mov r0, #0
    str r0, [r1, #-4]

flush_loop:
    mcr p15, 0, r1, c7, c14, 1
    cmp r0, r2
    add r0, #0x10
    add r1, #0x10
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
    # ISB; Invalidate unified, data TLBs; DSB
    mcr p15, 0, r0, c7, c5, 4
    mcr p15, 0, r0, c8, c7, 0
    mcr p15, 0, r0, c8, c6, 0
    mcr p15, 0, r0, c7, c10, 4
    
    # ok now actually start chatting
    ldr r0, caddr

wait_loop:
    ldr r3, [r0]
    tst r3, #0x80000000
    bne wait_loop

    ldr r3, buf_base
    sub r3, #0x40000000
    str r3, [r0, #0x14]

    # # of bytes
    lsr r3, r2, #6
    add r3, #1
    lsl r3, #19
    orr r2, r3
    str r2, [r0, #0x10]

    ldr r1, [r0]
    orr r1, #0x84000000
    str r1, [r0]
end:
    bx lr

.align 2
caddr: .long 0xc0000000 + (0x900 + 3*0x20)
pte: .long 0x86100c02 
buf_base: .long 0x80000120
_putc_end:
