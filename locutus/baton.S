.data
.thumb
.globl _baton, _baton_end, _baton_path
_baton:
    nop
    nop
    nop
    nop

    mov sp, pc

    adr r0, zero
    ldr r4, pthread_set_self
    blx r4

    adr r0, thread
    mov r1, #0
    adr r2, entry
    add r2, #1
    ldr r4, pthread_create
    blx r4

    ldr r4, mach_thread_self
    blx r4
    ldr r4, thread_terminate
    bx r4
.align 2
entry:
    adr r0, _baton_path
    mov r1, #1
    ldr r2, dlopen
    bx r2
.align 2
dlopen: .long _dlopen
pthread_set_self: .long ___pthread_set_self
pthread_create: .long _pthread_create
mach_thread_self: .long _mach_thread_self
thread_terminate: .long _thread_terminate
_baton_path: .ascii "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
thread: .long 0
zero: .long 0
_baton_end:
