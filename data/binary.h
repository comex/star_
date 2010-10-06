#pragma once

extern void *load_base;
static inline void *addrconv(addr_t addr) {
    return (void *) ((char *)load_base + (intptr_t)(addr & 0x0fffffff));
}

static inline bool is_valid_address(void *address) {
    char c;
    return !mincore(address, 1, &c);
}

#define r(sz) \
static inline uint##sz##_t read##sz(addr_t addr) { \
    return *(uint##sz##_t *)(addrconv(addr)); \
}

r(8)
r(16)
r(32)
r(64)

void macho_load_symbols();

void load_dyld_cache(const char *path, bool pre_loaded);
range_t dyld_nth_segment(int n);
addr_t dyld_find_anywhere(char *to_find, int align);
void dyld_choose_file(const char *filename);

void load_running_kernel();
void load_macho(const char *path);
range_t macho_segrange(const char *segname);
void *macho_offconv(uint32_t fileoff);

void check_range_has_addr(range_t range, addr_t addr);
addr_t sym(const char *name, bool to_execute);
