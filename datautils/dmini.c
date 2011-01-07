#include <data/common.h>
#include <data/find.h>
#include <data/binary.h>
#include <data/cc.h>
#include <config/placeholder.h>
#include <data/running_kernel.h>
#include <data/dyld_cache_format.h>

static struct binary binary;

// count the number of set bits
static int count_ones(uint32_t number) {
    int result = 0;
    for(; number; number >>= 1) {
        result += (number & 1);
    }
    return result;
}

static int position_of_nth_one(uint32_t number, int n) {
    for(int pos = 0; pos < 32; pos++) {
        if((number & (1 << pos)) && !n--) return pos;
    }
    die("no nth one (%08x, n=%d)", number, n);
}


// similar to below, but no alignment restriction
// must include r0, lr, and pc, but not r7

void find_second_ldm(struct binary *binary, uint32_t valid_conds, int reg, addr_t *addrp, int *num_before_r0, int *num_after_r0) {
    //printf("reg=%d\n", reg);
    range_t range;
    for(int i = 0; (range = b_nth_segment(binary, i)).binary; i++) {
        if(!(binary->dyld_mappings[i].sfm_init_prot & PROT_EXEC)) continue;
        uint32_t *p = rangeconv(range).start;
        for(addr_t addr = range.start; addr + 4 <= range.start + range.size; p++, addr += 4) {
            uint32_t val = *p; 
            uint32_t cond = ((val & 0xf0000000) >> 28);
            if(cond == 15 || (1u << (2*cond)) != (valid_conds & (3 << (2*cond)))) {
                continue;
            }

            int offset;
            if((val & 0xfdfc081) == 0x890c001) {
                offset = 0;
            } else if((val & 0xfdfc081) == 0x990c001) {
                offset = 1;
            } else {
                continue;
            }

            *addrp = addr;
            *num_before_r0 = offset;
            *num_after_r0 = count_ones(val & 0x3f7e);
            return;
        }
    }
    die("didn't find second ldm /anywhere/"); 
}

// ldmi[ab]<cond> r[05]!?, ...
// - aligned to 0x1000
// - includes pc, but not r7
// - PC is at position 3-5
// each set of two bits in valid_conds is:
//  0 - known false
//  1 - known true
//  2 - unknown

void find_kernel_ldm(struct binary *binary, uint32_t valid_conds, addr_t *addrp, uint32_t *condsp, int *regp) {
    range_t range;
    uint32_t my_valid_conds = valid_conds;
    for(int i = 0; (range = b_nth_segment(binary, i)).binary; i++) {
        if(!(binary->dyld_mappings[i].sfm_init_prot & PROT_EXEC)) continue;
        char *p = rangeconv(range).start;
        addr_t addr = (range.start + 0xfff) & ~0xfff;
        while(addr + 4 <= range.start + range.size) {
            if((addr & 0xfff) == 0) {
                my_valid_conds = valid_conds;
            }
            uint32_t val = *((uint32_t *) (p + addr - range.start));
            uint32_t cond = ((val & 0xf0000000) >> 28);

            if(cond != 15 && 0 == (my_valid_conds & (3 << (2*cond)))) {
                goto harmless;
            } else if(cond != 15 && !(val & 0xc000000) && (val & 0xe100000) != 0xc100000) { // data processing, but not LDC
                uint32_t rd = (val & 0xf000) >> 12;
                if(rd != 0 && rd != 13 && rd != 15) {
                    if(!(val & (1 << 20))) my_valid_conds = 0x1aaaaaaa; // AL known 1, others unknown
                    goto harmless;
                } else if(rd == 0) {
                    uint32_t op = ((val & 0x1f00000) >> 20);
                    if(op == 17 || op == 19 || op == 21 || op == 23) {
                        my_valid_conds = 0x1aaaaaaa;
                        goto harmless;
                    }
                }
            }

            if(cond == 15 || (1u << (2*cond)) != (my_valid_conds & (3 << (2*cond)))) {
                goto nope;
            }


            // 0xfdf to be strict about user registers, 0xf9f otherwise
            int offset;
            if((val & 0xfd0c080) == 0x890c000) {
                // ldmia
                offset = 0;
            } else if((val & 0xfd0c080) == 0x990c000) {
                // ldmib
                offset = 1;
            } else {
                goto nope;
            }

            uint32_t rn = (val & 0xf0000) >> 16;
            uint32_t reglist = val & 0x7f7f;
            int ones = count_ones(reglist) + offset; // ones = offset of PC
            
            printf("addr=%x rn=%u ones=%d val=%x\n", addr, rn, ones, val);
            if(rn != 0 && rn != 5) goto nope;

            //printf("rn=%u ones=%d\n", rn, ones);
            if(ones < 3 || ones > 5) goto nope;

            *addrp = addr;
            *condsp = my_valid_conds;
            *regp = position_of_nth_one(reglist, 2);
            return;

            nope: /*printf("%08x nope\n", addr);*/ addr = (addr + 0x1000) & ~0xfff; continue;
            harmless: /*printf("%08x harmless\n", addr);*/ addr += 4; continue;
        }
    }
    die("didn't find ldm /anywhere/"); 
}


void main_loop() {
    printf("+ %d\n", binary.actual_cpusubtype);
    fflush(stdout);

    while(1) {
        char arg[129]; arg[0] = 0;
        int mode = 0;
        if(scanf("%d ", &mode) != 1) die("?");
        if(!fgets(arg, sizeof(arg), stdin)) die("?");
        arg[strlen(arg) - 1] = 0;

        addr_t result;

        if(mode == 0) { 
            result = b_find_anywhere(&binary, arg, arg[0] == '+' ? 2 : 4, false);
        } else if(mode == 1) {
            result = b_sym(&binary, arg, true);
        } else if(mode == 2) {
            result = b_private_sym(&binary, arg, true);
        } else if(mode == 3) {
            b_dyldcache_load_macho(&binary, arg);
            result = 0;
        } else if(mode == 4) {
            addr_t first, second; int reg, num_before_r0, num_after_r0;
            uint32_t conds = (uint32_t) strtoll(arg, NULL, 16);
            find_kernel_ldm(&binary, conds, &first, &conds, &reg);
            find_second_ldm(&binary, conds, reg, &second, &num_before_r0, &num_after_r0);
            printf("+ %x %x %x %x\n", first, second, num_before_r0, num_after_r0);
            fflush(stdout);
            continue;
        } else die("mode?");

        printf("+ %x\n", result);
        fflush(stdout);
    }
}

int main(int argc, char **argv) {
    b_init(&binary);
    char **p = &argv[1];
    if(!p[0]) goto usage;
    while(p[0]) {
        if(p[0][0] == '-') switch(p[0][1]) {
        case 'C':
            b_load_running_dyldcache(&binary, (void *) 0x30000000);
            p++;
            break;
        case 'c':
            if(!p[1]) goto usage;
            b_load_dyldcache(&binary, p[1], false);
            p += 2;
            break;
        case 'k':
            if(!p[1]) goto usage;
            b_load_macho(&binary, p[1], false);
            p += 2;
            break;
        case 'd':
            if(!p[1]) goto usage;
            b_load_macho(&binary, p[1], false);
            p += 2;
            break;
        case 'K': {
            b_running_kernel_load_macho(&binary);  
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
            b_prange_load_macho(&binary, kern, false);
            p += 4;
            break;
        }
#endif
        default:
            goto usage;
        }
    }

    main_loop();

    return 0;

    usage:
    fprintf(stderr, "Usage: dmini (-c cache | -C | -d dyld | -k kc | -K"
#ifdef IMG3_SUPPORT
    " | -i kernel_img3 key iv"
#endif
    ")\n");
    return 1;
}

