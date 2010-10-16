#include <config/config_asm.h>
.thumb
.syntax unified
.globl _patch_start, _patch_end
_patch_start:
// <- 
    push {r0-r4, lr}
    sub sp, #0x44
    ldr r2, dvp_struct_offset
    ldr r4, [r3, r2]
    cmp r4, #0
    beq actually_eval
    ldr r3, vn_getpath
    mov r1, sp
    movs r0, #0x40
    add r2, sp, #0x40
    str r0, [r2]
    mov r0, r4
    blx r3
    cmp r0, #28
    beq enospc
    cmp r0, #0
    bne actually_eval
enospc:
    # that error's okay...

    mov r0, sp
    adr r1, var_mobile
    movs r2, #19 ;# len(var_mobile)
    ldr r3, memcmp
    blx r3
    cmp r0, #0
    bne allow

    mov r0, sp
    adr r1, preferences_com_apple
    movs r2, #49 ;# len(preferences_com_apple)
    ldr r3, memcmp
    blx r3
    cmp r0, #0
    beq actually_eval

    mov r0, sp
    adr r1, preferences
    movs r2, #39 ;# len(preferences)
    ldr r3, memcmp
    blx r3
    cmp r0, #0
    bne actually_eval

allow:
    # it's not in /var/mobile but we have a path, let it through
    add sp, #0x44
    pop {r0}
    movs r1, #0
    str r1, [r0]
    strb r1, [r0, #5]
    pop {r1-r4, pc}

actually_eval:
    add sp, #0x44
    ldr r0, [sp, #5*4]
    mov lr, r0
    ldr r1, orig_addr
    mov r9, r1
    ldr r0, is_armv7
    ldr r0, [r0]
    cmp r0, #0
    pop {r0-r4}
    add sp, #4
    beq armlol
    .long CONFIG_SB_EVALUATE_ORIG1
    .long CONFIG_SB_EVALUATE_ORIG2
    bx r9
armlol:
    bx pc
.arm
    .align 2
    .long CONFIG_SB_EVALUATE_ORIG1
    .long CONFIG_SB_EVALUATE_ORIG2
    bx r9
    

.align 2
var_mobile: .ascii "/private/var/mobile"
.align 2
preferences_com_apple: .ascii "/private/var/mobile/Library/Preferences/com.apple"
.align 2
preferences: .ascii "/private/var/mobile/Library/Preferences"
.align 2
orig_addr: .long CONFIG_SB_EVALUATE_JUMPTO
memcmp: .long CONFIG_MEMCMP
vn_getpath: .long CONFIG_VN_GETPATH
dvp_struct_offset: .long CONFIG_DVP_STRUCT_OFFSET
is_armv7: .long CONFIG_IS_ARMV7
_patch_end:
