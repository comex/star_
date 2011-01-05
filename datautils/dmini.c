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
    for(int i = 0; (range = b_nth_segment(binary, i)).binary; i++) {
        if(!(binary->dyld_mappings[i].sfm_init_prot & PROT_EXEC)) continue;
        char *p = rangeconv(range).start;
        for(addr_t addr = range.start; addr + 4 <= (range.start + range.size); my_valid_conds = valid_conds, addr = (addr + 0x1000) & ~0xfff) {
            harmless:;
            uint32_t val = *((uint32_t *) (p + (addr - range.start)));
            uint32_t cond = ((val & 0xf0000000) >> 28);
            
            bool harmless = false;

            if((addr & 0xfff) != 0xffc) {
                if(cond != 15 && 0 == (my_valid_conds & (3 << (2*cond)))) {
                    addr += 4; goto harmless;
                } else if(!(val & 0xc000000)) { // data processing
                    uint32_t rd = (val & 0xf000) >> 12;
                    if(rd != 0 && rd != 15) {
                        if(!(val & (1 << 20))) my_valid_conds = 0x1aaaaaaa; // AL known 1, others unknown
                        addr += 4; goto harmless;
                    } else if(rd == 0) {
                        uint32_t op = ((val & 0x1f00000) >> 20);
                        if(op == 17 || op == 19 || op == 21 || op == 23) {
                            my_valid_conds = 0x1aaaaaaa;
                            addr += 4; goto harmless;
                        }
                    }
                }
            }

            if(cond == 15 || (1u << (2*cond)) != (my_valid_conds & (3 << (2*cond)))) {
                continue;
            }

            //printf("ready to test %08x @ %08x\n", val, addr);

            bool ldmib;
            if((val & 0xf9f8000) == 0x8908000) {
                ldmib = false;
            } else if((val & 0xf9f8000) == 0x9908000) {
                ldmib = true;
            } else {
                continue;
            }
            printf("%08x -> %08x (%s)\n", addr, val, ldmib ? "ib" : "ia");
            uint32_t reglist = val & 0x7fff;
            int ones = count_ones(reglist);
            if(ldmib) ones--;
            if(ones < 3 || ones > 5) continue;
            // figure out 


            printf("%d\n", count_ones(reglist));
            if(count_ones(reglist) != (ldmib ? 1 : 2)) continue;
            //printf(":) %08x = %08x\n", addr, val);
            return addr; 
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
            result = find_kernel_ldm(&binary, atoi(arg));
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

