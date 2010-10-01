#include <config/config.h>
.syntax unified
.thumb
.globl _patch_start, _patch_end
_patch_start:
// <- 
    push {r0-r3, lr}

    ldrb r0, [r3]
    cmp r0, #0x2f ;#'/'
    bne actually_match
    
    mov r0, r3
    mov r2, #20 ;# len('/private/var/mobile/')
    adr r1, private_var
    ldr r3, strncmp
    blx r3
    cmp r0, #0
    beq actually_match

    #ldr r1, [sp, #12]
    #adr r0, fmt
    #ldr r2, IOLog
    #blx r2

    # rude, dude
    pop {r0-r3, lr}
    mov r0, #0
    str r0, [r10]
    strb r0, [r10, #4]
    mov r0, r10
    sub sp, r7, #0x18
    pop {r8, r10, r11}
    pop {r4-r7, pc}

actually_match:
    pop {r0-r3, lr}
    .long CONFIG_MATCH_STRING_ORIG1
    .long CONFIG_MATCH_STRING_ORIG2
    ldr pc, orig_addr

private_var: .asciz "/private/var/mobile/"
.align 2
orig_addr: .long CONFIG_MATCH_STRING_JUMPTO
strncmp: .long CONFIG_STRNCMP

#IOLog: .long CONFIG_IOLOG
#fmt: .asciz "-> %s\n"
_patch_end:
