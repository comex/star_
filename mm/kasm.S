.section .init
.thumb
stub:
    push {lr}
    blx mem_init
    pop {pc}

