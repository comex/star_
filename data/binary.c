#include "common.h"
#include "binary.h"
#include "loader.h"
#include "nlist.h"
#include "fat.h"
#include "dyld_cache_format.h"
#ifdef __APPLE__
#include <mach/mach.h>
#endif

const int desired_cputype = 12; // ARM
const int desired_cpusubtype = 9; // v7=9, v6=6

// global data:
void *load_base;

int dyld_fd;
struct dyld_cache_header dyld_hdr;
uint32_t dyld_mapping_count;
struct shared_file_mapping_np *dyld_mappings;

struct mach_header *mach_hdr;

struct nlist *symtab;
uint32_t nsyms;
char *strtab;
uint32_t strsize;

#define CMD_ITERATE(hdr, cmd) for(struct load_command *cmd = (void *)((hdr) + 1), *end = (void *)((char *)(hdr) + (hdr)->sizeofcmds); cmd < end; cmd = (void *)((char *)(cmd) + cmd->cmdsize))

static void *reserve_memory() {
    void *result = mmap(NULL, 0x10000000, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(result == MAP_FAILED) {
        edie("reserve_memory: could not do so: %s\n");
    }
    return result;
}

void macho_load_symbols() {
    bool dysymtab = false;
    uint32_t iextdefsym, nextdefsym;
    CMD_ITERATE(mach_hdr, cmd) {
        if(cmd->cmd == LC_SYMTAB) {
            struct symtab_command *scmd = (void *) cmd;
            if(scmd->nsyms >= 0x1000000) {
                die("macho_load_symbols: ridiculous number of symbols (%u)\n", scmd->nsyms);
            }
            // possible crash
            nsyms = scmd->nsyms;
            strsize = scmd->strsize;
            symtab = macho_offconv(scmd->symoff);
            strtab = macho_offconv(scmd->stroff);
        } else if(cmd->cmd == LC_DYSYMTAB) {
            struct dysymtab_command *dcmd = (void *) cmd;
            iextdefsym = dcmd->iextdefsym;
            nextdefsym = dcmd->nextdefsym;
            dysymtab = true;
        } else if(cmd->cmd == LC_DYLD_INFO_ONLY) {
            fprintf(stderr, "macho_load_symbols: warning: file is fancy, symbols might be missing\n");
        }
    }
    if(symtab && dysymtab) {
        if(iextdefsym >= nsyms) {
            die("macho_load_symbols: bad iextdefsym (%u)\n", iextdefsym);
        }
        if(nextdefsym > nsyms - iextdefsym) {
            die("macho_load_symbols: bad nextdefsym (%u)\n", nextdefsym);
        }
        symtab += iextdefsym;
        nsyms = nextdefsym;
    }
}

void load_dyld_cache(const char *path, bool pre_loaded) {
    int fd = open(path, O_RDONLY);
    if(fd == -1) { 
        edie("load_dyld_cache(%s): could not open: %s\n", path);
    }
    if(read(fd, &dyld_hdr, sizeof(dyld_hdr)) != sizeof(dyld_hdr)) {
        die("load_dyld_cache(%s): truncated\n", path);
    }
    if(memcmp(dyld_hdr.magic, "dyld", 4)) {
        die("load_dyld_cache(%s): not a dyld cache\n", path);
    }
    if(dyld_hdr.mappingCount > 1000) {
        die("load_dyld_cache(%s): insane mapping count\n", path);
    }
    dyld_mapping_count = dyld_hdr.mappingCount;
    size_t sz = dyld_hdr.mappingCount * sizeof(struct shared_file_mapping_np);
    dyld_mappings = malloc(sz);
    if(pread(fd, dyld_mappings, sz, dyld_hdr.mappingOffset) != sz) {
        edie("load_dyld_cache(%s): could not read mappings: %s\n", path);
    }
    if(pre_loaded) {
        load_base = (void *) 0x30000000;
    } else {
        load_base = reserve_memory();
        for(int i = 0; i < dyld_mapping_count; i++) {
            struct shared_file_mapping_np mapping = dyld_mappings[i];
            if(mmap(addrconv((addr_t) mapping.sfm_address), (size_t) mapping.sfm_size, PROT_READ, MAP_SHARED | MAP_FIXED, fd, (off_t) mapping.sfm_file_offset) == MAP_FAILED) {
                edie("load_dyld_cache(%s): could not map segment %d of this crappy binary format\n", path, i);
            }
        }
    }
    dyld_fd = fd;
}

range_t dyld_nth_segment(int n) {
    if(n < dyld_mapping_count) {
        return (range_t) {(addr_t) dyld_mappings[n].sfm_address, (size_t) dyld_mappings[n].sfm_size};
    } else {
        return (range_t) {0, 0};
    }
}

void dyld_choose_file(const char *filename) {
    if(dyld_hdr.imagesCount > 1000) {
        die("dyld_load_syms: insane images count\n");
    }
    for(int i = 0; i < dyld_hdr.imagesCount; i++) {
        struct dyld_cache_image_info info;
        if(pread(dyld_fd, &info, sizeof(info), dyld_hdr.imagesOffset + i * sizeof(struct dyld_cache_image_info)) != sizeof(info)) {
            die("dyld_load_syms: could not read image\n");
        }
        char name[128];
        if(pread(dyld_fd, name, sizeof(name), info.pathFileOffset) <= 0) {
            die("dyld_load_syms: could not read image name\n");
        }
        if(strncmp(name, filename, 128)) {
            continue;
        }
        // we found it; I suppose I should check whether this is a real address
        mach_hdr = addrconv((addr_t) info.address);
        return;
    }
}

void load_macho(const char *path) {
    int fd = open(path, O_RDONLY);
    if(fd == -1) { 
        edie("load_macho(%s): could not open: %s\n", path);
    }
    void *fhdr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if(fhdr == MAP_FAILED) {
        edie("load_macho(%s): could not map file header: %s\n", path);
    }
    uint32_t magic = *(uint32_t *)fhdr;
    uint32_t fat_offset;
    if(magic == MH_MAGIC) {
        // thin file
        mach_hdr = fhdr;
        if(mach_hdr->cputype != desired_cputype || (mach_hdr->cpusubtype != 0 && desired_cpusubtype != 0 && mach_hdr->cpusubtype != desired_cpusubtype)) {
            die("load_macho: thin file doesn't have the right architecture\n");
        }
        fat_offset = 0;
    } else if(magic == FAT_MAGIC) {
        if(desired_cpusubtype == 0) {
            die("load_macho(%s): fat, but we don't even know what we want\n", path);
        }
        struct fat_header *fathdr = fhdr;
        struct fat_arch *arch = (void *)(fathdr + 1);
        uint32_t nfat_arch = fathdr->nfat_arch;
        if(sizeof(struct fat_header) + nfat_arch * sizeof(struct fat_arch) >= 4096) {
            die("load_macho(%s): fat header is too big\n", path);
        }
        while(nfat_arch--) {
            if(arch->cputype == desired_cputype && (arch->cpusubtype == 0 || arch->cpusubtype == desired_cpusubtype)) {
                munmap(fhdr, 4096);
                fat_offset = arch->offset;
                mach_hdr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, fat_offset);
                if(mach_hdr == MAP_FAILED) {
                    edie("load_macho(%s): could not map mach-o header from fat file: %s\n", path);
                }
                break;
            }
            arch++;
        }
    } else {
        die("load_macho: (%08x) what is this I don't even", magic);
    }

    if(mach_hdr->sizeofcmds > 4096 - sizeof(*mach_hdr)) {
        die("load_macho: sizeofcmds is too big\n");
    }

    symtab = NULL;

    load_base = reserve_memory();

    CMD_ITERATE(mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *scmd = (void *) cmd;
            if(scmd->vmsize == 0) scmd->filesize = 0; // __CTF
            if(scmd->filesize != 0) {
                if(mmap(addrconv(scmd->vmaddr), scmd->filesize, PROT_READ, MAP_SHARED | MAP_FIXED, fd, fat_offset + scmd->fileoff) == MAP_FAILED) {
                    edie("load_macho: could not map segment %.16s at %u+%u,%u: %s\n", scmd->segname, scmd->fileoff, fat_offset, scmd->filesize);
                }
            }
        }
    }
    
    macho_load_symbols();
}

void load_running_kernel() {
#ifdef __APPLE__
    kern_return_t kr;

    mach_port_name_t kernel_task;
    kr = task_for_pid(mach_task_self(), 0, &kernel_task);
    if(kr) {
        die("load_running_kernel: task_for_pid failed.  u probably need kernel patches. kr=%d\n", kr);
    }

    kr = vm_allocate(mach_task_self(), (vm_address_t *) &mach_hdr, 0x1000, true);
    if(kr) {
        die("load_running_kernel: vm_allocate mach_hdr failed\n");
    }
    addr_t mh_addr;
    vm_size_t size;
    for(addr_t hugebase = 0x80000000; hugebase; hugebase += 0x40000000) {
        for(addr_t pagebase = 0x1000; pagebase < 0x10000; pagebase += 0x1000) {
            // vm read, compare to MH_MAGIC, hurf durf
            mh_addr = (vm_address_t) (hugebase + pagebase);
            size = 0x1000;
            // This will return either KERN_PROTECTION_FAILURE if it's a good address, and KERN_INVALID_ADDRESS otherwise.
            // But if we use a shorter size, it will read if it's a good address, and /crash/ otherwise.
            // So we do two.
            kr = vm_read_overwrite(kernel_task, (vm_address_t) mh_addr, size, (vm_address_t) mach_hdr, &size);
            if(kr == KERN_INVALID_ADDRESS) {
                continue;
            } else if(kr && kr != KERN_PROTECTION_FAILURE) {
                die("load_running_kernel: unexpected error from vm_read_overwrite: %d\n", kr);
            }
            // ok, it's valid, but is it the actual header?
            size = 0xfff;
            kr = vm_read_overwrite(kernel_task, (vm_address_t) mh_addr, size, (vm_address_t) mach_hdr, &size);
            if(kr) {
                die("load_running_kernel: second vm_read_overwrite failed: %d\n", kr);
            }
            if(mach_hdr->magic == MH_MAGIC) {
                printf("found running kernel at %p\n", (void *) mh_addr);
                goto ok;
            }
        }
    }
    die("load_running_kernel: didn't find the kernel anywhere\n");

    ok:;
    if(mach_hdr->sizeofcmds > size - sizeof(*mach_hdr)) {
        die("load_running_kernel: sizeofcmds is too big\n");
    }
    addr_t maxaddr = mh_addr;
    CMD_ITERATE(mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *scmd = (void *) cmd;
            addr_t newmax = scmd->vmaddr + scmd->filesize;
            if(newmax > maxaddr) maxaddr = newmax;
        }
    }

    // Well, uh, this sucks.  But there's some block on reading.  In fact, it's probably a bug that this works.
    size_t read_size = maxaddr - mh_addr;
    load_base = malloc(read_size);
    char *p = load_base;
    load_base = (char *)load_base - 0x1000;
#ifdef PROFILING
    clock_t a = clock();
#endif
    while(read_size > 0) {
        vm_size_t this_size = (vm_size_t) read_size;
        if(this_size > 0xfff) this_size = 0xfff;
        kr = vm_read_overwrite(kernel_task, (vm_address_t) mh_addr, this_size, (vm_address_t) p, &this_size);
        if(kr) {
            die("load_running_kernel: vm_read_overwrite failed: %d\n", kr);
        }
        mh_addr += this_size;
        p += this_size;
        read_size -= this_size;
    }
#ifdef PROFILING
    clock_t b = clock();
    printf("it took %d clocks to read the kernel\n", (int)(b - a));
#endif
    macho_load_symbols();
/*
    vm_address_t mine = 0x10000000;
    vm_prot_t curprot, maxprot;

    printf("%x %x %x\n", maxaddr, mh_addr, maxaddr - mh_addr);

    for(addr_t i = 0xc0000000; i < 0xc0100000; i += 0x1000) {
        vm_size_t outsize;
        vm_address_t data;
        kr = vm_read(kernel_task, (vm_address_t) i, 0xfff, &data, &outsize);
        printf("%08x -> %d\n", i, kr);
    }
    die("...\n");

    kr = vm_remap(mach_task_self(), &mine, 0x1000, 0, true, kernel_task, (mach_vm_address_t) mh_addr, false, &curprot, &maxprot, VM_INHERIT_NONE);
    if(kr) {
        die("load_running_kernel: vm_remap returned %d\n", mine, kr);
    }
    printf("curprot=%d maxprot=%d mine=%x\n", curprot, maxprot, mine);

    load_base = (void *) (mine - 0x1000);
*/
#else
    die("load_running_kernel: not on Apple\n");
#endif
}
 

range_t macho_segrange(const char *segname) {
    CMD_ITERATE(mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *scmd = (void *) cmd;
            if(!strncmp(scmd->segname, segname, 16)) {
                return (range_t) {scmd->vmaddr, scmd->filesize};
            }
        }
    }
    die("macho_segrange: no such segment %s\n", segname);
}

void *macho_offconv(uint32_t fileoff) {
    CMD_ITERATE(mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *scmd = (void *) cmd;
            if(fileoff >= scmd->fileoff && fileoff < scmd->fileoff + scmd->filesize) {
                void *result = addrconv(scmd->vmaddr + fileoff - scmd->fileoff);
                if(!is_valid_address(result)) {
                    die("macho_offconv: invalid offset %u\n", fileoff);
                }
                return result;
            }
        }
    }
    die("offconv: file offset %u not in segment\n", fileoff);
}

// return value is |1 if to_execute is set and it is a thumb symbol
addr_t sym(const char *name, bool to_execute) {
    if(!symtab) {
        die("sym: we wanted %s but there is no symbol table\n", name);
    }
    // I stole dyld's codez
    const struct nlist *base = symtab;
    for(uint32_t n = nsyms; n > 0; n /= 2) {
        const struct nlist *pivot = base + n/2;
        uint32_t strx = pivot->n_un.n_strx;
        if(strx >= strsize) {
            die("sym: insane strx: %u\n", strx);
        }
        const char *pivot_str = strtab + strx;
        int cmp = strncmp(name, pivot_str, strsize - strx);
        if(cmp == 0) {
            // we found it
            addr_t result = pivot->n_value;
            if(to_execute && (pivot->n_desc & N_ARM_THUMB_DEF)) {
                result |= 1;
            }
            return result;
        } else if(cmp > 0) {
            base = pivot + 1; 
            n--;
        }
    }
    die("sym: symbol %s not found\n", name);
}

void check_range_has_addr(range_t range, addr_t addr) {
    if(addr < range.start || addr >= (range.start | range.size)) {
        die("Bad address 0x%08x\n", addr);
    }
}

