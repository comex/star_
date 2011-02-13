#include <data/common.h>
#include <data/binary.h>
#include <assert.h>

int main(int argc, char **argv) {
    struct binary binary;
    b_init(&binary);
    mode_t mode;
    prange_t kernel = load_file(argv[1], true, &mode);
    b_prange_load_macho(&binary, kernel, argv[1]);

    int patchfd = open(argv[2], O_RDONLY);
    if(patchfd == -1) {
        edie("could not open patchfd");
    }
    
    while(1) {
        uint32_t name_len;
        ssize_t result = read(patchfd, &name_len, sizeof(name_len));
        if(result == 0) break;
        assert(result == sizeof(name_len));
        assert(name_len < 128);
        char *name = malloc(name_len + 1);
        assert(read(patchfd, name, name_len) == (ssize_t) name_len);
        name[name_len] = 0;
        
        addr_t addr;
        assert(read(patchfd, &addr, sizeof(addr)) == sizeof(addr));

        uint32_t size;
        assert(read(patchfd, &size, sizeof(size)) == sizeof(size));
        assert(size < 0x1000000);
        
        void *stuff = malloc(size);
        assert(read(patchfd, stuff, size) == (ssize_t) size);
        
        if(addr == 0) goto skip;
        if(name[0] == '+') goto skip;

        if(argv[4] && !strcmp(argv[4], "-i")) {
            retry:
            printf("%s [y/n] ", name);
            fflush(stdout);
            char buf[3];
            if(!fgets(buf, sizeof(buf), stdin)) abort();
            if(!strcmp(buf, "n\n")) {
                goto skip;
            } else if(strcmp(buf, "y\n")) {
                goto retry;
            }
        } else {
            printf("%s\n", name);
        }

        memcpy((char *) kernel.start + range_to_off_range((range_t) {&binary, addr, size}).start, stuff, size);

        skip:

        free(name);
        free(stuff);
    }

    store_file(kernel, argv[3], mode);
    return 0;
}
