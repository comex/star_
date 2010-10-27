#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <mach/mach.h>
// inefficient, hacky, works
int main(int argc, char **argv) {
    vm_address_t addr = (vm_address_t) strtoll(argv[1], NULL, 16);
    assert(addr);
    mach_port_name_t kernel_task;
    assert(!task_for_pid(mach_task_self(), 0, &kernel_task));
    uint32_t val = 0x00100008 | 0x100;
    assert(!vm_write(kernel_task, addr, (vm_offset_t) &val, sizeof(val)));
    // I don't want to leave the kernel in this state no matter what.
    int ret, fail = 0;
    fail |= ret = mkdir("/private/var", 0755);
    printf("mkdir 1: %d\n", ret);
    fail |= ret = mkdir("/private/var/db", 0755);
    printf("mkdir 2: %d\n", ret);
    fail |= ret = mkdir("/private/var/db/.launchd_use_gmalloc", 0755);
    printf("mkdir 3: %d\n", ret);
    val &= ~0x100;
    assert(!vm_write(kernel_task, (vm_address_t) addr, (vm_offset_t) &val, sizeof(val)));
    assert(!ret);
    return 0;
}
