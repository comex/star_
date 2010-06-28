/*#define ADDEND (CONFIG_OVBCOPY - ((CONFIG_BCOPYDUDE + 2) & ~3))
#define J1 ((ADDEND & 0x800000) >> 23)
#define J2 ((ADDEND & 0x400000) >> 22)
#define IMM10H ((ADDEND & 0x3ff000) >> 12)
#define IMM10L ((ADDEND & 0xffc) >> 2)
#define BLXINSN (0xc000f400 | (IMM10L << (16+1)) | (J2 << (16+11)) | (J1 << (16+13)) | IMM10H)*/
.arm
.globl start
start:
    ldr r0, =0xdeadbeef
    ldr r3, [r0]
    add r1, pc, #1
    bx r1
.thumb
    ldr r1, =CONFIG_PATCH1
    ldr r2, =CONFIG_PATCH1_TO
    str r2, [r1]
    ldr r1, =CONFIG_PATCH2
    mov r2, #0
    str r2, [r1]
    ldr r1, =CONFIG_PATCH3
    ldr r2, =CONFIG_PATCH3_TO
    str r2, [r1]
    ldr r1, =CONFIG_PATCH4
    ldr r2, =CONFIG_PATCH4_TO
    str r2, [r1]
    mov r2, #1
    ldr r1, =CONFIG_PATCH5
    str r2, [r1]
    ldr r1, =CONFIG_PATCH6
    str r2, [r1]
    

    # we need to undo our screwing up of bcopydude
    #adr r0, end
    mov r2, #24
loop:
    ldr r3, [r0]
    str r3, [r1]
    add r0, #4
    add r1, #4
    sub r2, #1
    cmp r2, #0
    bge loop
        
    # return to the original caller.
    pop {r4, r5, r7, pc}
