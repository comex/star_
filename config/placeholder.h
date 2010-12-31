#include "../config/config_asm.h"

// preplace32 is lazy
#define preplace32(range, a, b) do { uintptr_t _ = preplace32_a(range, a); if(_) preplace32_b(range, _, a, b); } while(0)

static void check_no_placeholders(prange_t pr) {
    for(uintptr_t addr = (uintptr_t)pr.start; addr + sizeof(uint32_t) <= (uintptr_t)pr.start + pr.size; addr++) {
        uint32_t val = *(uint32_t *)addr;
        if(val > CONFIG_MIN && val < CONFIG_MAX) {
            die("got %08x", val);
        }
    }
}

static uintptr_t preplace32_a(prange_t range, uint32_t a) {
    for(uintptr_t addr = (uintptr_t)range.start; addr + sizeof(uint32_t) <= (uintptr_t)range.start + range.size; addr++) {
        if(*(uint32_t *)addr == a) {
            return addr;
        }
    }
    //fprintf(stderr, "preplace32: warning: didn't find %08x anywhere\n", a);
    return 0;
}

static void preplace32_b(prange_t range, uintptr_t start, uint32_t a, uint32_t b) {
    for(uintptr_t addr = start; addr + sizeof(uint32_t) <= (uintptr_t)range.start + range.size; addr++) {
        if(*(uint32_t *)addr == a) {
            *(uint32_t *)addr = b;
        }
    }
}

