.thumb
.syntax unified
start:
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
    movs r1, #0x18
    strb r1, [r0, #4]
    pop {r1-r4, pc}

actually_eval:
    add sp, #0x44
    ldr r0, [sp, #5*4]
    mov lr, r0
    ldr r1, orig_addr
    mov r9, r1
    ldr r0, is_armv7
    cmp r0, #0
    pop {r0-r4}
    add sp, #4
    beq armlol
    .long c_sb_evaluate_orig1
    .long c_sb_evaluate_orig2
    bx r9
armlol:
    bx pc
.arm
    .align 2
    .long c_sb_evaluate_orig1
    .long c_sb_evaluate_orig2
    bx r9
    

.align 2
var_mobile: .ascii "/private/var/mobile"
.align 2
preferences_com_apple: .ascii "/private/var/mobile/Library/Preferences/com.apple"
.align 2
preferences: .ascii "/private/var/mobile/Library/Preferences"
.align 2
orig_addr: .long c_sb_evaluate_jumpto
memcmp: .long c_memcmp
vn_getpath: .long c_vn_getpath
dvp_struct_offset: .long c_dvp_struct_offset
is_armv7: .long c_is_armv7
