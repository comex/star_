#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <mach/mach.h>
#include "elf.h"
#include <config/config.h>

struct proc;
// copied from xnu

typedef int32_t sy_call_t(struct proc *, void *, int *);
typedef void    sy_munge_t(const void *, void *);

struct sysent {     /* system call table */
    int16_t     sy_narg;    /* number of args */
    int8_t      sy_resv;    /* reserved  */
    int8_t      sy_flags;   /* flags */
    sy_call_t   *sy_call;   /* implementing function */
    sy_munge_t  *sy_arg_munge32; /* system call arguments munger for 32-bit process */
    sy_munge_t  *sy_arg_munge64; /* system call arguments munger for 64-bit process */
    int32_t     sy_return_type; /* system call return types */
    uint16_t    sy_arg_bytes;   /* Total size of arguments in bytes for
                     * 32-bit system calls
                     */
};
#define _SYSCALL_RET_INT_T      1   

// end copied

// search for 01 00 00 00 0c 00 00 00

struct sysent my_sysent = { 1, 0, 0, NULL, NULL, NULL, _SYSCALL_RET_INT_T, 5 * sizeof(uint32_t) };

int main() {
    assert(sizeof(struct sysent) == 0x18);
    mach_port_t task;

    assert(!task_for_pid(mach_task_self(), 0, &task));

    Elf32_Ehdr ehdr;
    Elf32_Phdr phdr;
    int fd = open("kcode.elf", O_RDONLY);
    assert(fd > 0);
    assert(read(fd, &ehdr, sizeof(ehdr)) == sizeof(ehdr));
    assert(ehdr.e_phentsize == sizeof(phdr));
    lseek(fd, ehdr.e_shoff, SEEK_SET);
    Elf32_Half phnum = ehdr.e_phnum;
    while(phnum--) {
        assert(read(fd, &phdr, sizeof(phdr)) == sizeof(phdr));
        if(phdr.p_type == PT_LOAD) {
            if(!my_sysent.sy_call) my_sysent.sy_call = (void *) (phdr.p_vaddr);
            void *buf = malloc(phdr.p_filesz);
            assert(pread(fd, buf, phdr.p_filesz, phdr.p_offset) == phdr.p_filesz);
            vm_address_t address = (vm_address_t) phdr.p_vaddr;
            vm_size_t memsz = (phdr.p_memsz + 0xfff) & ~0xfff;
            assert(!vm_allocate(task, &address, memsz, false));
            assert(address == (vm_address_t) phdr.p_vaddr);
            assert(!vm_write(task, (vm_address_t) phdr.p_vaddr, (pointer_t) buf, phdr.p_filesz));
            free(buf);
            assert(!vm_protect(task, (vm_address_t) phdr.p_vaddr, memsz, true, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE));
            assert(!vm_protect(task, (vm_address_t) phdr.p_vaddr, memsz, false, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE));
            vm_machine_attribute_val_t flush = MATTR_VAL_ICACHE_FLUSH;
            assert(!vm_machine_attribute(task, (vm_address_t) phdr.p_vaddr, phdr.p_memsz, MATTR_CACHE, &flush));
        }
    }

    assert(!vm_write(task, (vm_address_t) (CONFIG_SYSENT + 8 * sizeof(struct sysent)), (pointer_t) &my_sysent, sizeof(struct sysent)));
    
    return 0;
}
