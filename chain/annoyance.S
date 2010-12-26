.thumb
.section __DATA,__data
.syntax unified
.align 4
.globl _annoyance_start, _annoyance_end, _annoyance_space, _annoyance_iolog, _annoyance_come_from, _annoyance_return_to

_annoyance_start:

    push {r0-r12, lr}
    push {r2-r12, lr}
    mov r2, r0
    mov r3, r1
    adr r0, str
    ldr r1, _annoyance_come_from
    ldr lr, _annoyance_iolog
    blx lr
    pop {r2-r12, lr}
    pop {r0-r12, lr}

_annoyance_space: nop; nop; nop; nop; nop; nop; nop; nop

    ldr pc, _annoyance_return_to

_annoyance_iolog: .long 0xdeadbeef
_annoyance_come_from: .long 0xdeadbeef
_annoyance_return_to: .long 0xdeadbeef
str: .asciz "@%x r0:%x r1:%x r2:%x r3:%x r4:%x r5:%x r6:%x r7:%x r8:%x r9:%x r10:%x r11:%x r12:%x lr:%x\n"

_annoyance_end:

