.thumb
.globl start
start:
    ldr r0, [r5, #0x18]
    ldm r0, {r0-r3}
    blx r3
    bx r0
