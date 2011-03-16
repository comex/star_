#include "inject.h"
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <common/common.h>

#define ARM_THREAD_STATE 1
//#define ARM_THREAD_STATE_COUNT 17
struct arm_thread_state {
    uint32_t r[13];
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t cpsr;
};


#define address_cast(x) ((mach_vm_address_t) (uintptr_t) (x))

extern char baton[], baton_end[], baton_path[64];
static /*const*/ mach_vm_size_t stack_size = 32*1024;

kern_return_t inject(pid_t pid, const char *path) {
    mach_vm_size_t baton_size = baton_end - baton;
    kern_return_t kr = 0;

    task_t task;
    _assert_zero(task_for_pid(mach_task_self(), (int) pid, &task));
    
    mach_vm_address_t stack_address = 0;
    _assert_zero(mach_vm_allocate(task, &stack_address, stack_size, VM_FLAGS_ANYWHERE));
    mach_vm_address_t baton_address = stack_address + stack_size - baton_size;

    printf("baton_address = %x\n", (int) baton_address);

    strlcpy(baton_path, path, 64);

    _assert_zero(mach_vm_protect(task, stack_address, stack_size, false, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE));

#if 0
    struct vm_region_basic_info_64 info;
    mach_msg_type_number_t cnt = VM_REGION_BASIC_INFO_COUNT_64;
    mach_vm_address_t addr = stack_address;
    mach_vm_size_t size;
    mach_port_t o;
    _assert_zero(mach_vm_region(task, &addr, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t) &info, &cnt, &o));
    printf("%x+%x %d,%d\n", (int) addr, (int) size, info.protection, info.max_protection);
    abort();
#endif
    
    _assert_zero(mach_vm_write(task, baton_address, (vm_offset_t) baton, baton_size));
    
    static union {
        struct arm_thread_state arm;
        natural_t nat;
    } state = { { .cpsr = 0x20 } };
    state.arm.pc = baton_address;

    thread_act_t thread;
    _assert_zero(thread_create_running(task, ARM_THREAD_STATE, &state.nat, ARM_THREAD_STATE_COUNT, &thread));

    //mach_vm_deallocate(task, stack_address, stack_size);
    //mach_port_deallocate(mach_task_self(), exc);
    //mach_port_deallocate(mach_task_self(), thread);
    //mach_port_deallocate(mach_task_self(), task);

    return 0;    
}
