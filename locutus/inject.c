#include "inject.h"
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

extern bool NSAddLibrary(const char *);

#define ARM_THREAD_STATE 1
//#define ARM_THREAD_STATE_COUNT 17
struct arm_thread_state {
    uint32_t r[13];
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t cpsr;
};

static const mach_vm_size_t stack_size = 32*1024;

#define DEBUG 1

#define FAIL(x) do { if(DEBUG) fprintf(stderr, "fail on line %d: %x\n", __LINE__, (x)); abort(); } while(0)
#define TRY(x) do { if(kr = x) { FAIL(kr);} } while(0)
#define address_cast(x) ((mach_vm_address_t) (uintptr_t) (x))

kern_return_t inject(pid_t pid, const char *path, mach_vm_address_t *stack_address) {
    kern_return_t kr = 0;

    task_t task;
    TRY(task_for_pid(mach_task_self(), (int) pid, &task));
    
    mach_vm_address_t stack_address = 0;
    TRY(mach_vm_allocate(task, &stack_address, stack_size, VM_FLAGS_ANYWHERE));

    mach_vm_address_t stack_end = stack_address + stack_size - 0x100;

    TRY(mach_vm_write(task, stack_address, address_cast(path), strlen(path) + 1));

    thread_act_t thread;
    TRY(thread_create(task, &thread));

    mach_port_t exc;
    mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &exc);
    TRY(mach_port_insert_right(mach_task_self(), exc, exc, MACH_MSG_TYPE_MAKE_SEND));
    TRY(thread_set_exception_ports(thread, EXC_MASK_BAD_ACCESS, exc, EXCEPTION_DEFAULT, ARM_THREAD_STATE));

    union {
        struct arm_thread_state arm;
        natural_t nat;
    } state;

    memset(&state, 0, sizeof(state));

    state.arm.r[0] = 360;
    state.arm.r[1] = (uint32_t) dlopen;
    state.arm.r[2] = (uint32_t) stack_address;
    state.arm.r[3] = 128*1024;
    // the other args are 0 anyway
    state.arm.sp = (uint32_t) stack_end;
    state.arm.pc = (uint32_t) syscall;
    state.arm.lr = (uint32_t) 0xdeadbeef;

    TRY(thread_set_state(thread, ARM_THREAD_STATE, &state.nat, ARM_THREAD_STATE_COUNT));
    TRY(thread_resume(thread));

    // handle the exception
    char msg[8192];
    TRY(mach_msg_overwrite(NULL, MACH_RCV_MSG, 0, sizeof(msg), exc, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL, (void *) &msg, sizeof(msg)));
    TRY(thread_terminate(thread));

    

    mach_vm_deallocate(task, stack_address, stack_size);
    mach_port_deallocate(mach_task_self(), exc);
    mach_port_deallocate(mach_task_self(), thread);
    mach_port_deallocate(mach_task_self(), task);

    return 0;    
}

return inject_deall
