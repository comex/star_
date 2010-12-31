#include <data/common.h>
#include <data/find.h>
#include <data/binary.h>
#include <data/cc.h>
#include <config/placeholder.h>
#include <data/running_kernel.h>
#include <data/dyld_cache_format.h>

// count the number of set bits
static int count_ones(uint32_t number) {
    int result = 0;
    for(; number; number >>= 1) {
        result += (number & 1);
    }
    return result;
}

// ldmib<cond> r11/r11!, {.., .., .., sp, pc}
addr_t find_init_ldmib(struct binary *binary, uint32_t cond) {
    range_t range;
    for(int i = 0; (range = b_dyldcache_nth_segment(binary, i)).start; i++) {
        uint32_t *p = rangeconv(range).start;
        for(addr_t addr = range.start; addr + 4 <= (range.start + range.size); addr += 4) {
            // <3 http://a.qoid.us/01x.py
            // fun fact: on armv6 this is actually UNPREDICTABLE
            uint32_t val = *p++;
            if((val & 0xf9fe000) != 0x99ba000) continue;
            uint32_t actual_cond = ((val & 0xf0000000) >> 28);
            if(actual_cond != cond) continue;
            uint32_t reglist = val & 0x1fff;
            // The famous interview question: calculate the number of 1 bits
            if(count_ones(reglist) != 3) continue;
            return addr; 
        }
    }
    die("didn't find ldmib /anywhere/"); 
}

// ldmia<cond> r0/r0!, {.., .., sp,( lr,)? pc}
// OR
// ldmib<cond> r0/r0!, {.., sp,( lr,)? pc}
// aligned to 0x1000
// each set of two bits in valid_conds is:
//  0 - known false
//  1 - known true
//  2 - unknown

addr_t find_kernel_ldm(struct binary *binary, uint32_t valid_conds) {
    range_t range;
    uint32_t my_valid_conds = valid_conds;
    for(int i = 0; (range = b_dyldcache_nth_segment(binary, i)).start; i++) {
        if(!(binary->dyld_mappings[i].sfm_init_prot & PROT_EXEC)) continue;
        char *p = rangeconv(range).start;
        for(addr_t addr = range.start; addr + 4 <= (range.start + range.size); my_valid_conds = valid_conds, addr = (addr + 0x1000) & ~0xfff) {
            back:;
            uint32_t val = *((uint32_t *) (p + (addr - range.start)));
            uint32_t cond = ((val & 0xf0000000) >> 28);
            
            bool harmless = false;
            if(cond != 15 && 0 == (my_valid_conds & (3 << (2*cond)))) {
                harmless = true;
            } else if(!(val & 0xc000000)) { // data processing
                uint32_t rd = (val & 0xf000) >> 12;
                if(rd != 0 && rd != 15) {
                    harmless = true;
                    if(!(val & (1 << 20))) my_valid_conds = 0x1aaaaaaa; // AL known 1, others unknown
                } else if(rd == 0) {
                    uint32_t op = ((val & 0x1f00000) >> 20);
                    if(op == 17 || op == 19 || op == 21 || op == 23) {
                        harmless = true;
                        my_valid_conds = 0x1aaaaaaa;
                    }
                }
            }

            if(harmless) {
                //printf("@ %08x: %08x is harmless (vc=%08x)\n", addr, val, my_valid_conds);
                addr += 4;
                goto back;
            }

            if(cond == 15 || (1u << (2*cond)) != (my_valid_conds & (3 << (2*cond)))) {
                continue;
            }

            //printf("ready to test %08x @ %08x\n", val, addr);

            bool ldmib;
            if((val & 0xf9fa000) == 0x890a000) {
                ldmib = false;
            } else if((val & 0xf9fa000) == 0x990a000) {
                ldmib = true;
            } else {
                continue;
            }
            //printf("%08x -> %08x (%s)\n", addr, val, ldmib ? "ib" : "ia");
            uint32_t reglist = val & 0x1fff;
            if(count_ones(reglist) != (ldmib ? 1 : 2)) continue;
            printf(":) %08x = %08x\n", addr, val);
            return addr; 
        }
    }
    die("didn't find ldm /anywhere/"); 
}

void do_dyldcache(prange_t pr, struct binary *binary) {
    bool is_armv7 = b_is_armv7(binary);
    preplace32(pr, CONFIG_IS_ARMV7, (uint32_t) is_armv7);
    int align = is_armv7 ? 2 : 4;

    preplace32(pr, CONFIG_K4, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 00 68 b0 bd" : "- 00 00 90 e5 b0 80 bd e8", align));
    preplace32(pr, CONFIG_K5, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 20 60 90 bd" : "- 00 00 84 e5 90 80 bd e8", align));
    preplace32(pr, CONFIG_K6, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 20 44 90 bd" : "- 00 00 84 e0 90 80 bd e8", align));
    preplace32(pr, CONFIG_K7, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 0f bd" : "- 0f 80 bd e8", align));
    preplace32(pr, CONFIG_K9, b_dyldcache_find_anywhere(binary, "+ 0e bd", 2));
    preplace32(pr, CONFIG_K10, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ a7 f1 00 0d 80 bd" : "- 00 d0 47 e2 80 80 bd e8", align));
    preplace32(pr, CONFIG_K11, b_dyldcache_find_anywhere(binary, "+ f0 bd", 2));
    preplace32(pr, CONFIG_K12, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ a0 47 b0 bd" : "- 34 ff 2f e1 b0 80 bd e8", align));
    preplace32(pr, CONFIG_K14, b_dyldcache_find_anywhere(binary, "+ 20 58 90 bd", 2));
    preplace32(pr, CONFIG_K15, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 40 f8 04 4b 90 bd" : "- 00 40 80 e5 90 80 bd e8", align));
    preplace32(pr, CONFIG_K16, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 20 68 90 bd" : "- 00 00 94 e5 90 80 bd e8", align));
    preplace32(pr, CONFIG_K17, b_dyldcache_find_anywhere(binary, is_armv7 ? "+ 25 60 b0 bd" : "- 00 50 84 e5 b0 80 bd e8", align));
    preplace32(pr, CONFIG_K18, b_dyldcache_find_anywhere(binary, "+ 10 bd", 2));
    preplace32(pr, CONFIG_K19, b_dyldcache_find_anywhere(binary, "+ 80 bd", 2));
    preplace32(pr, CONFIG_KINIT, find_init_ldmib(binary, is_armv7 ? 4 /* MI */ : 5 /* PL */));

    // NE, CS, MI, VC, HI, LT, LE, AL = 0b110100110010110
    // >>> ''.join('0' + i for i in '110100110010110')
    preplace32(pr, CONFIG_REMAP_FROM, find_kernel_ldm(binary, 0x14414114));
    //b_dyldcache_load_macho(binary, "/usr/lib/libSystem.B.dylib");
    //preplace32(pr, 0xfeed1001, b_sym(binary, "_sysctlbyname", true));
    //preplace32(pr, 0xfeed1002, b_sym(binary, "_execve", true));
}

void do_dyld(prange_t pr, struct binary *binary) {
    (void) pr; (void) binary;
}

void do_kernel(prange_t pr, struct binary *binary) {
    addr_t sysent = find_data(b_macho_segrange(binary, "__DATA"), "21 00 00 00 00 10 86 00 -", 0, true);
    preplace32(pr, CONFIG_SYSENT_PATCH, sysent + 4);
    addr_t sysent_patch_orig = b_read32(binary, sysent + 4);
    preplace32(pr, CONFIG_SYSENT_PATCH_ORIG, sysent_patch_orig);
    preplace32(pr, CONFIG_TARGET_ADDR, (sysent_patch_orig & 0x00ffffff) | 0x2f000000);

}

int main(int argc, char **argv) {
    struct binary kernel, dyld, cache;
    b_init(&kernel);
    b_init(&dyld);
    b_init(&cache);
    char **p = &argv[1];
    if(!p[0]) goto usage;
    while(p[0]) {
        if(p[0][0] == '-') switch(p[0][1]) {
        case 'C':
            b_load_running_dyldcache(&cache, (void *) 0x30000000);
            p++;
            break;
        case 'c':
            if(!p[1]) goto usage;
            b_load_dyldcache(&cache, p[1], false);
            p += 2;
            break;
        case 'k':
            if(!p[1]) goto usage;
            b_load_macho(&kernel, p[1], false);
            p += 2;
            break;
        case 'd':
            if(!p[1]) goto usage;
            b_load_macho(&dyld, p[1], false);
            p += 2;
            break;
        case 'K': {
            b_running_kernel_load_macho(&kernel);  
            p++;
            break;
        }
#ifdef IMG3_SUPPORT
        case 'i': {
            if(!p[1] || !p[2] || !p[3]) goto usage;
            uint32_t key_bits;
            prange_t key = parse_hex_string(p[2]);
            prange_t iv = parse_hex_string(p[3]);
            prange_t data = parse_img3_file(p[1], &key_bits);
            prange_t kern = decrypt_and_decompress(key_bits, key, iv, data);
            b_prange_load_macho(&kernel, kern, false);
            p += 4;
            break;
        }
#endif
        default:
            goto usage;
        }
        else { // not a -
            if(!p[1]) goto usage;
            mode_t mode;
            prange_t pr = load_file(p[0], true, &mode);
            if(cache.valid) do_dyldcache(pr, &cache);
            if(kernel.valid) do_kernel(pr, &kernel);
            if(dyld.valid) do_dyld(pr, &dyld);
            check_no_placeholders(pr);
            store_file(pr, p[1], mode);
            punmap(pr);
            p += 2;
        }
    }
    return 0;

    usage:
    fprintf(stderr, "Usage: data [-c cache | -C] [-d dyld] [-k kc | -K"
#ifdef IMG3_SUPPORT
    " | -i kernel_img3 key iv"
#endif
    "] infile outfile [infile outfile...]\n");
    return 1;
}
