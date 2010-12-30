#include <data/common.h>
#include <data/find.h>
#include <data/binary.h>
#include <data/cc.h>
#include <data/placeholder.h>

#define patch_range(name, addr, pr...) \
    ({ prange_t pr_ = pr; \
       memcpy((char *) output.start + range_to_off_range((range_t) {binary, addr, pr_.size}).start, pr_.start, pr_.size); })

#define patch(name, addr, typeof_to, to...) \
    ({ typeof_to to_[] = to; \
       patch_range(name, addr, (prange_t) {&to_[0], sizeof(to_)}); })

uint32_t find_dvp_struct_offset(struct binary *binary) {
    range_t range = b_macho_segrange(binary, "__PRELINK_TEXT");
    addr_t derive_vnode_path = find_bof(range, find_int32(range, find_string(range, "path", 0, true), true), b_is_armv7(binary));
    uint8_t byte = read8(binary, find_data((range_t){binary, derive_vnode_path, 1024}, !b_is_armv7(binary) ? "00 00 50 e3 02 30 a0 11 - .. 00 94 e5" : "- .. 69 6a 46", 0, true));
    if(b_is_armv7(binary)) {
        return (4 | (byte >> 6)) << 2;
    } else {
        return byte;
    }
}

addr_t find_scratch(struct binary *binary) {
    range_t range = b_macho_segrange(binary, "__PRELINK_TEXT");
    prange_t pr = rangeconv(range);
    for(size_t o = 0; o < pr.size - 0x1000; o += 4) {
        for(size_t p = 0; p < 0x1000; p++) {
            uint8_t c = ((char *) pr.start)[o + p];
            if(!((p != 0 && c == 0) || c == 0x14)) goto nope;
        }
        return range.start + o;
        nope:;
    }
    die("didn't find scratch");
}

addr_t find_sysctl(struct binary *binary, const char *name) {
    addr_t cs = find_string(b_macho_segrange(binary, "__TEXT"), name, 0, true);
    addr_t csref = find_int32(b_macho_segrange(binary, "__DATA"), cs, true);
    return b_read32(binary, csref - 8);
}

void do_kernel(prange_t output, prange_t sandbox, struct binary *binary) {
    bool is_armv7 = b_is_armv7(binary);

//#define IN_PLACE_PATCH
    // patches
#ifdef IN_PLACE_PATCH
    patch(PATCH_KERNEL_PMAP_NX_ENABLED,
          b_read32(binary, b_sym(binary, "_kernel_pmap", false)) + 0x420,
          uint32_t, {0});
#else
    // the second ref to mem_size
    patch(PATCH_KERNEL_PMAP_NX_ENABLED,
          find_data(b_macho_segrange(binary, "__TEXT"), is_armv7 ? "03 68 - c3 f8 20 24" : "84 23 db 00 - d5 50 22 68", 0, true),
          uint32_t, {is_armv7 ? 0xc420f8c3 : 0x682250d0});

    /*patch(PATCH_PROC_ENFORCE,
          find_sysctl(binary, "proc_enforce"),
          uint32_t, {0});*/

    patch(PATCH_LUNCHD,
          find_string(b_macho_segrange(binary, "__DATA"), "/sbin/launchd", 0, true),
          char, "/sbin/lunchd");
#endif

    // vm_map_enter (patch1) - allow RWX pages
    patch(PATCH1,
          find_data(b_macho_segrange(binary, "__TEXT"), is_armv7 ? "- 02 0f .. .. 63 08 03 f0 01 05 e3 0a 13 f0 01 03" : "- .. .. .. .. .. 08 1e 1c .. 0a 01 22 .. 1c 16 40 .. 40", 0, true),
          uint32_t, {is_armv7 ? 0x46c00f02 : 0x46c046c0});

    // AMFI (patch3) - disable the predefined list of executable stuff
    patch(PATCH3,
          find_data(b_macho_segrange(binary, "__PRELINK_TEXT"), is_armv7 ? "23 78 9c 45 05 d1 .. .. .. .. .. .. .. 4b 98 47 00 .. -" : "13 20 a0 e3 .. .. .. .. 33 ff 2f e1 00 00 50 e3 00 00 00 0a .. 40 a0 e3 - 04 00 a0 e1 90 80 bd e8", 0, true),
          uint32_t, {is_armv7 ? 0x1c201c20 : 0xe3a00001});
    // PE_i_can_has_debugger (patch4) - so AMFI allows non-ldid'd binaries (and some other stuff is allowed)
    patch(PATCH4,
          b_sym(binary, "_PE_i_can_has_debugger", false),
          uint32_t, {0x47702001});


    // task_for_pid 0
    // this is necessary so a reboot isn't required after using the screwed up version
    patch(PATCH_TFP0,
          find_data(b_macho_segrange(binary, "__TEXT"), is_armv7 ? "85 68 00 23 .. 93 .. 93 - 5c b9 .. .. 29 46 04 22" : "85 68 .. 93 .. 93 - 00 2c 0b d1", 0, true),
          uint32_t, {is_armv7 ? 0x46c0e00b : 0xe00b2c00});
        
    // cs_enforcement_disable
    patch(PATCH_CS_ENFORCEMENT_DISABLE,
          resolve_ldr(binary, find_data(b_macho_segrange(binary, "__TEXT"), is_armv7 ? "1d ee 90 3f d3 f8 4c 33 d3 f8 9c 20 + .. .. .. .. 19 68 00 29" : "9c 22 03 59 99 58 + .. .. 1a 68 00 2a", 0, true)),
          uint32_t, {1});

    // sandbox
    range_t range = b_macho_segrange(binary, "__PRELINK_TEXT");
    addr_t sb_evaluate = find_bof(range, find_int32(range, find_string(range, "bad opcode", false, true), true), is_armv7);
    
    preplace32(sandbox, CONFIG_IS_ARMV7, (uint32_t) is_armv7);
    preplace32(sandbox, CONFIG_VN_GETPATH, b_sym(binary, "_vn_getpath", true));
    preplace32(sandbox, CONFIG_MEMCMP, b_sym(binary, "_memcmp", true));
    preplace32(sandbox, CONFIG_SB_EVALUATE_ORIG1, b_read32(binary, sb_evaluate));
    preplace32(sandbox, CONFIG_SB_EVALUATE_ORIG2, b_read32(binary, sb_evaluate + 4));
    preplace32(sandbox, CONFIG_SB_EVALUATE_JUMPTO, sb_evaluate + (is_armv7 ? 9 : 8));
    preplace32(sandbox, CONFIG_DVP_STRUCT_OFFSET, find_dvp_struct_offset(binary));
    
    addr_t scratch = find_scratch(binary);

    patch(SB_EVALUATE,
          sb_evaluate,
          uint32_t, {(is_armv7 ? 0xf000f8df : 0xe51ff004), scratch | 1});

    check_no_placeholders(sandbox);
    patch_range(SANDBOX,
                scratch,
                sandbox);

    fprintf(stderr, "scratch = %x\n", scratch);
}


int main(int argc, char **argv) {
    struct binary binary;
    b_init(&binary);
    b_load_macho(&binary, argv[1], false);
    mode_t mode;
    prange_t kernel = load_file(argv[1], true, &mode);
    prange_t sandbox = load_file(argv[2], true, NULL);
    do_kernel(kernel, sandbox, &binary);
    store_file(kernel, argv[3], mode);
    return 0;
}
