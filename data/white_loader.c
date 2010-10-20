#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach/mach.h>
#include <sys/mman.h>
#include <sys/file.h>
#include "common.h"
#include "binary.h"

// copied from xnu

struct proc;
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

const uint32_t SLIDE_START = 0xf0000000;

mach_port_t kernel_task;

static struct binary *to_load, *kern;
uint32_t slide;
uint32_t reloc_base;

uint32_t sysent; // :<

kern_return_t kr_assert_(kern_return_t kr, const char *name, int line) {
    if(kr) {
        fprintf(stderr, "kr_assert: result=%08x on line %d:\n%s\n", kr, line, name);
        assert(false);
    }
    return kr;
}
#define kr_assert(x) kr_assert_((x), #x, __LINE__)

uint32_t lookup_sym(char *sym) {
    if(!strcmp(sym, "_sysent")) return sysent;
    return b_sym(kern, sym, true);
}

void do_kern(const char *filename) {
    kern = malloc(sizeof(*kern));
    b_init(kern);
    b_load_macho(kern, filename);

    CMD_ITERATE(kern->mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            if(sysent) continue; 
            struct segment_command *seg = (void *) cmd;
            struct section *sections = (void *) (seg + 1);
            for(int i = 0; i < seg->nsects; i++) {
                struct section *sect = &sections[i];
                if(!strncmp(sect->sectname, "__data", 16)) {
                    uint32_t *things = b_addrconv(kern, sect->addr);
                    for(int i = 0; i < sect->size / 4; i++) {
                        if(things[i] == 0x861000) {
                            sysent = sect->addr + 4*i + 4;
                            goto nextlc;
                        }
                    }
                }
            }
        }
        nextlc:;
    }

    assert(sysent);
}

void relocate(uint32_t reloff, uint32_t nreloc) {
    struct relocation_info *things = b_macho_offconv(to_load, reloff);
    for(int i = 0; i < nreloc; i++) {
        assert(!things[i].r_pcrel);
        assert(things[i].r_length == 2);
        assert(things[i].r_type == 0);
        uint32_t thing = reloc_base + things[i].r_address;
        uint32_t *p = b_addrconv(to_load, thing);
        if(things[i].r_extern) {
            uint32_t sym = things[i].r_symbolnum;
            *p += lookup_sym(to_load->strtab + to_load->symtab[sym].n_un.n_strx);
        } else {
            // *shrug*
            printf("%08x: %08x -> %08x\n", thing, *p, *p + slide);
            *p += slide;
        }
    }
}

void do_kcode(const char *filename, uint32_t prelink_slide, const char *prelink_output) {
    if(!prelink_output) {
        kr_assert(task_for_pid(mach_task_self(), 0, &kernel_task));
    }
    to_load = malloc(sizeof(*to_load));
    b_init(to_load);
    b_load_macho(to_load, filename);

    CMD_ITERATE(to_load->mach_hdr, cmd) {
        switch(cmd->cmd) {
        case LC_SYMTAB:
        case LC_DYSYMTAB:
        case LC_SEGMENT:
        case LC_ID_DYLIB:
        case LC_UUID:
            break;
        default:
            fprintf(stderr, "unrecognized load command %08x\n", cmd->cmd);
            assert(false);
        }
    }
    assert(to_load->symtab);
    assert(to_load->dysymtab);

    if(prelink_output) {
        slide = prelink_slide;
        goto it_worked;
    } else if(to_load->mach_hdr->flags & MH_PREBOUND) {
        CMD_ITERATE(to_load->mach_hdr, cmd) {
            if(cmd->cmd == LC_SEGMENT) {
                struct segment_command *seg = (void *) cmd;
                if(seg->vmsize == 0) continue;
                vm_address_t address = seg->vmaddr;
                printf("allocate %08x %08x\n", (int) address, (int) seg->vmsize);
                kern_return_t kr = vm_allocate(kernel_task,
                                               &address,
                                               seg->vmsize,
                                               VM_FLAGS_FIXED);

                if(kr) {
                    fprintf(stderr, "allocation failed :(\n");
                    assert(false);
                }
                assert(address == seg->vmaddr);
            }
        }
    } else {
        // try to reserve some space
        for(slide = SLIDE_START; slide < SLIDE_START + 0x01000000; slide += 0x10000) {
            CMD_ITERATE(to_load->mach_hdr, cmd) {
                if(cmd->cmd == LC_SEGMENT) {
                    struct segment_command *seg = (void *) cmd;
                    if(seg->vmsize == 0) continue;
                    vm_address_t address = seg->vmaddr + slide;
                    printf("allocate %08x %08x\n", (int) address, (int) seg->vmsize);
                    kern_return_t kr = vm_allocate(kernel_task,
                                                   &address,
                                                   seg->vmsize,
                                                   VM_FLAGS_FIXED);
                    if(!kr) {
                        assert(address == seg->vmaddr + slide);
                        continue;
                    }
                    // Bother, it didn't work.  So we need to increase the slide...
                    // But first we need to get rid of the gunk we did manage to allocate.
                    CMD_ITERATE(to_load->mach_hdr, cmd2) {
                        if(cmd2 == cmd) break;
                        if(cmd2->cmd == LC_SEGMENT) {
                            struct segment_command *seg2 = (void *) cmd2;
                            printf("deallocate %08x %08x\n", (int) (seg->vmaddr + slide), (int) seg->vmsize);
                            kr_assert(vm_deallocate(kernel_task,
                                                    seg->vmaddr + slide,
                                                    seg->vmsize));
                        }
                    }
                    goto try_another_slide;
                }
            }
            // If we got this far, it worked!
            goto it_worked;
            try_another_slide:;
        }
    }
    // But if we got this far, we ran out of slides to try.
    fprintf(stderr, "we couldn't find anywhere to put this thing and that is ridiculous\n");
    assert(false);
    it_worked:;
    printf("slide=%x\n", slide);

    if(!(to_load->mach_hdr->flags & MH_PREBOUND)) {
        relocate(to_load->dysymtab->locreloff, to_load->dysymtab->nlocrel);
        relocate(to_load->dysymtab->extreloff, to_load->dysymtab->nextrel);

        uint32_t *indirect = b_macho_offconv(to_load, to_load->dysymtab->indirectsymoff);

        CMD_ITERATE(to_load->mach_hdr, cmd) {
            if(cmd->cmd == LC_SEGMENT) {
                struct segment_command *seg = (void *) cmd;
                seg->vmaddr += slide;
                if(!reloc_base) reloc_base = seg->vmaddr;
                printf("%.16s %08x\n", seg->segname, seg->vmaddr);
                struct section *sections = (void *) (seg + 1);
                for(int i = 0; i < seg->nsects; i++) {
                    struct section *sect = &sections[i];
                    sect->addr += slide;
                    printf("   %.16s\n", sect->sectname);
                    uint8_t type = sect->flags & SECTION_TYPE;
                    switch(type) {
                    case S_NON_LAZY_SYMBOL_POINTERS:
                    case S_LAZY_SYMBOL_POINTERS: {
                        uint32_t indirect_table_offset = sect->reserved1;
                        uint32_t *things = b_addrconv(to_load, sect->addr);
                        for(int i = 0; i < sect->size / 4; i++) {
                            uint32_t sym = indirect[indirect_table_offset+i];
                            switch(sym) {
                            case INDIRECT_SYMBOL_LOCAL:
                                things[i] += slide;
                                break;
                            case INDIRECT_SYMBOL_ABS:
                                break;
                            default:
                                things[i] = lookup_sym(to_load->strtab + to_load->symtab[sym].n_un.n_strx);
                            }
                            //printf("%p -> %x\n", &things[i], things[i]);
                        }
                        break;
                    }
                    case S_ZEROFILL:
                    case S_MOD_INIT_FUNC_POINTERS:
                    case S_MOD_TERM_FUNC_POINTERS:
                    case S_REGULAR:
                    case S_SYMBOL_STUBS:
                    case S_CSTRING_LITERALS:
                    case S_4BYTE_LITERALS:
                    case S_8BYTE_LITERALS:
                    case S_16BYTE_LITERALS:
                        break;
                    default:
                        fprintf(stderr, "unrecognized section type %02x\n", type);
                        assert(false);
                    }
                    
                    // XXX: are these relocations unique, or listed also in the dysymtab?
                    // until I get one, I won't bother finding out
                    assert(sect->nreloc == 0);

                    relocate(sect->reloff, sect->nreloc);
                }
            }
        }
    }

    if(prelink_output) {
        to_load->mach_hdr->flags |= MH_PREBOUND;
        b_macho_store(to_load, prelink_output);
    }

    CMD_ITERATE(to_load->mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            int32_t fs = seg->filesize;
            if(seg->vmsize < fs) fs = seg->vmsize;
            vm_offset_t of = (vm_offset_t) b_addrconv(to_load, seg->vmaddr);
            vm_address_t ad = seg->vmaddr;
            struct section *sections = (void *) (seg + 1);
            for(int i = 0; i < seg->nsects; i++) {
                struct section *sect = &sections[i];
                if((sect->flags & SECTION_TYPE) == S_ZEROFILL) {
                    void *data = calloc(1, sect->size);
                    kr_assert(vm_write(kernel_task,
                                       (vm_address_t) sect->addr,
                                       (vm_offset_t) data,
                                       sect->size));
                    free(data);
                }
            }
            while(fs > 0) {
                // complete headbang.
                printf("(%.16s) reading %x %08x -> %08x\n", seg->segname, fs, (uint32_t) of, (uint32_t) ad);
                uint32_t tocopy = 0xfff;
                if(fs < tocopy) tocopy = fs;
                kr_assert(vm_write(kernel_task,
                                   ad,
                                   of,
                                   tocopy));
                fs -= tocopy;
                of += tocopy;
                ad += tocopy;
            }
            if(seg->vmsize > 0) {
                kr_assert(vm_protect(kernel_task,
                                     seg->vmaddr,
                                     seg->vmsize,
                                     true,
                                     seg->maxprot));
                kr_assert(vm_protect(kernel_task,
                                     seg->vmaddr,
                                     seg->vmsize,
                                     false,
                                     seg->initprot));

                vm_machine_attribute_val_t val = MATTR_VAL_CACHE_FLUSH;
                kr_assert(vm_machine_attribute(kernel_task,
                                               seg->vmaddr,
                                               seg->vmsize,
                                               MATTR_CACHE,
                                               &val));
            }
        }
    }

    // okay, now do the fancy syscall stuff
    // how do I safely dispose of this file?
    int lockfd = open("/tmp/.syscall-11", O_RDWR | O_CREAT);
    assert(lockfd > 0);
    assert(!flock(lockfd, LOCK_EX));

    struct sysent orig_sysent;
    vm_size_t whatever;
    kr_assert(vm_read_overwrite(kernel_task,
                                sysent + 11 * sizeof(struct sysent),
                                sizeof(struct sysent),
                                (vm_offset_t) &orig_sysent,
                                &whatever));

    CMD_ITERATE(to_load->mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            struct section *sections = (void *) (seg + 1);
            for(int i = 0; i < seg->nsects; i++) {
                struct section *sect = &sections[i];

                if((sect->flags & SECTION_TYPE) == S_MOD_INIT_FUNC_POINTERS) {
                    void **things = b_addrconv(to_load, sect->addr);
                    for(int i = 0; i < sect->size / 4; i++) {
                        struct sysent my_sysent = { 1, 0, 0, things[i], NULL, NULL, _SYSCALL_RET_INT_T, 0 };
                        printf("--> %p\n", things[i]);
                        kr_assert(vm_write(kernel_task,
                                           (vm_address_t) sysent + 11 * sizeof(struct sysent),
                                           (vm_offset_t) &my_sysent,
                                           sizeof(struct sysent)));
                        syscall(11);
                    }
                }
            }
        }
    }

    kr_assert(vm_write(kernel_task,
                       sysent + 11 * sizeof(struct sysent),
                       (vm_offset_t) &orig_sysent,
                       sizeof(struct sysent)));

    assert(!flock(lockfd, LOCK_UN));
}

void unload_kcode(uint32_t addr) {
    kr_assert(task_for_pid(mach_task_self(), 0, &kernel_task));
    vm_size_t whatever;
    
    struct mach_header *hdr = malloc(0x1000);
    if(vm_read_overwrite(kernel_task,
                         (vm_address_t) addr,
                         0x1000,
                         (vm_offset_t) hdr,
                         &whatever) == KERN_INVALID_ADDRESS) {
        fprintf(stderr, "invalid address %08x\n", addr);
        assert(false);
    }
    kr_assert(vm_read_overwrite(kernel_task,
                                (vm_address_t) addr,
                                0xfff,
                                (vm_offset_t) hdr,
                                &whatever));
    CMD_ITERATE(hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            struct section *sections = (void *) (seg + 1);
            for(int i = 0; i < seg->nsects; i++) {
                struct section *sect = &sections[i];

                if((sect->flags & SECTION_TYPE) == S_MOD_TERM_FUNC_POINTERS) {
                    void **things = malloc(sect->size);
                    kr_assert(vm_read_overwrite(kernel_task,
                                                (vm_address_t) sect->addr,
                                                sect->size,
                                                (vm_offset_t) things,
                                                &whatever));
                    for(int i = 0; i < sect->size / 4; i++) {
                        struct sysent my_sysent = { 1, 0, 0, things[i], NULL, NULL, _SYSCALL_RET_INT_T, 0 };
                        printf("--> %p\n", things[i]);
                        kr_assert(vm_write(kernel_task,
                                           (vm_address_t) sysent + 11 * sizeof(struct sysent),
                                           (vm_offset_t) &my_sysent,
                                           sizeof(struct sysent)));
                        syscall(11);
                    }
                    free(things);
                }
            }
        }
    }

    CMD_ITERATE(hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            if(seg->vmsize > 0) {
                kr_assert(vm_deallocate(kernel_task,
                                        seg->vmaddr,
                                        seg->vmsize));
            }
        }
    }
}

int main(int argc, char **argv) {
    if(argc <= 1) goto usage;
    if(argv[1][0] != '-' || argv[1][1] == '\0' || argv[1][2] != '\0') goto usage;
    switch(argv[1][1]) {
    case 'l':
        if(argc <= 3) goto usage;
        do_kern(argv[2]);
        do_kcode(argv[3], 0, NULL);
        return 0;
    case 'p':
        if(argc <= 5) goto usage;
        do_kern(argv[2]);
        do_kcode(argv[3], (uint32_t) strtoll(argv[4], NULL, 16), argv[5]);
        return 0;
    case 'u':
        if(argc <= 3) goto usage;
        do_kern(argv[2]);
        unload_kcode((uint32_t) strtoll(argv[3], NULL, 16));
        return 0;
    }

    usage:
    printf("Usage: loader -l kern kcode.dylib                         load\n" \
           "              -p kern kcode.dylib f0001000 out.dylib      prelink\n" \
           "              -u kern f0001000                            unload\n" \
           );
}

#if 0
    void *foo = malloc(4096);
    printf("%p\n", foo);
    mach_vm_address_t addr;
    vm_prot_t cp, mp;
    kr_assert(vm_remap(mach_task_self(), &addr, 4096, 0xfff, true, kernel_task, 0x8075d000, false, &cp, &mp, VM_INHERIT_NONE));
    printf("%d %d\n", cp, mp);
    printf("%x %x\n", *((uint32_t *) addr), *((uint32_t *) (addr + 4)));
    return 0; 
#endif
