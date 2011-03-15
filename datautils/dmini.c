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

