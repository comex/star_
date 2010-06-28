.thumb
.globl start
start:
;#    ldr r7, [r4, #8]
;#    ldr r1, =CONFIG_PATCH1
;#    ldr r2, =CONFIG_PATCH1_TO
;#    str r2, [r1]
;#    ldr r1, =CONFIG_PATCH2
;#    mov r2, #0
;#    str r2, [r1]
;#    ldr r1, =CONFIG_PATCH3
;#    ldr r2, =CONFIG_PATCH3_TO
;#    str r2, [r1]
;#    ldr r1, =CONFIG_PATCH4
;#    ldr r2, =CONFIG_PATCH4_TO
;#    str r2, [r1]
;#    mov r2, #1
;#    ldr r1, =CONFIG_PATCH5
;#    str r2, [r1]
;#    ldr r1, =CONFIG_PATCH6
;#    str r2, [r1]
    nop
    nop
    bx lr
