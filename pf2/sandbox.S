#include <config/config.h>
.syntax unified
.thumb
.globl _patch_start, _patch_end
_patch_start:
// <- 
    push {r4, lr}
    adr lr, next+1
    .long CONFIG_DERIVE_VNODE_PATH_ORIG1
    .long CONFIG_DERIVE_VNODE_PATH_ORIG2
    ldr pc, orig_addr
next:
    mov r4, r0
    ldr r0, [r0, #0x20]
    adr r1, private_var
    mov r2, #13
    ldr r3, strncmp
    blx r3
    cmp r0, #0
    beq done
    adr r0, dev_zero
    str r0, [r4, #0x20]
done:
    mov r0, r4
    pop {r0, pc}
private_var: .asciz "/private/var/"
dev_zero: .asciz "/dev/zero"
orig_addr: .long CONFIG_DERIVE_VNODE_PATH_JUMPTO
strncmp: .long CONFIG_STRNCMP
_patch_end:
