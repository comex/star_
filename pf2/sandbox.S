#include <config/config.h>
.syntax unified
.thumb
.globl _patch_start, _patch_end
_patch_start:
// <- 
    push {r0-r4, lr}
    ldr r4, [r3, #CONFIG_DVP_STRUCT_OFFSET]
    cmp r4, #0
    beq actually_eval
    ldr r3, vn_getpath_fsenter
    sub r1, sp, #0x44
    mov r0, #0x40
    sub r2, sp, #4
    str r0, [r2]
    mov r0, r4
    sub sp, #0x50
    blx r3
    add sp, #0x50
    cmp r0, #0
    beq actually_eval

    sub r0, sp, #0x44
    adr r1, private_var
    mov r2, #19 ;# len('/private/var/mobile')
    ldr r3, memcmp
    blx r3
    cmp r0, #0
    beq actually_eval

    ;# it's not in /var/mobile but we have a path, let it through
    pop {r0}
    mov r1, #0
    str r1, [r0]
    strb r1, [r0, #5]
    pop {r1-r4, pc}

actually_eval:
    pop {r0-r4, lr}
    .long CONFIG_SB_EVALUATE_ORIG1
    .long CONFIG_SB_EVALUATE_ORIG2
    ldr pc, orig_addr

private_var: .ascii "/private/var/mobile"
.align 2
orig_addr: .long CONFIG_SB_EVALUATE_JUMPTO
memcmp: .long CONFIG_MEMCMP
vn_getpath_fsenter: .long CONFIG_VN_GETPATH_FSENTER
_patch_end:
