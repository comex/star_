#include <config/config.h>
.syntax unified
.thumb
.globl _patch_start, _patch_end
_patch_start:
// <- 
    push {r0-r5, lr}
    ldr r4, [r3, #0x14]
    cmp r4, #0
    beq actually_eval
    ldr r3, IOMalloc
    mov r0, #0x40
    blx r3
    ldr r3, vn_getpath_fsenter
    mov r5, r0
    mov r1, r0
    mov r0, #0x40
    sub r2, sp, #4
    str r0, [r2]
    mov r0, r4
    blx r3
    cmp r0, #0
    bne continue
    ;# vn_getpath failed for some reason, so free and quit
    mov r0, r5
    ldr r3, IOFree
    blx r3
    b actually_eval
continue:
    mov r0, r5
    adr r1, private_var
    mov r2, #19 ;# len('/private/var/mobile')
    ldr r3, memcmp
    blx r3
    mov r4, r0
    ldr r3, IOFree
    mov r0, r5
    blx r3

    cmp r4, #0
    beq actually_eval

    ;# it's not in /var/mobile but we have a path, let it through
    pop {r0}
    mov r1, #0
    str r1, [r0]
    strb r1, [r0, #5]
    pop {r1-r5, pc}

actually_eval:
    pop {r0-r5, lr}
    .long CONFIG_SB_EVALUATE_ORIG1
    .long CONFIG_SB_EVALUATE_ORIG2
    ldr pc, orig_addr

private_var: .ascii "/private/var/mobile"
.align 2
orig_addr: .long CONFIG_SB_EVALUATE_JUMPTO
memcmp: .long CONFIG_MEMCMP
IOMalloc: .long CONFIG_IOMALLOC
IOFree: .long CONFIG_IOFREE
vn_getpath_fsenter: .long CONFIG_VN_GETPATH_FSENTER
_patch_end:
