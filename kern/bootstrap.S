.arm
.globl start
start:
    ldr r0, [r5, #0x20]
    ldm r0, {r0-r3, pc}
.thumb
    blx r3
    bx r0
