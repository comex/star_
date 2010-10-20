#pragma once

struct dyld_cache_header;
struct shared_file_mapping_np;
struct mach_header;
struct binary {
    int actual_cpusubtype;
    void *load_base;

    int dyld_fd;
    struct dyld_cache_header *dyld_hdr;
    uint32_t dyld_mapping_count;
    struct shared_file_mapping_np *dyld_mappings;

    struct mach_header *mach_hdr;

    struct nlist *symtab;
    uint32_t nsyms;
    char *strtab;
    uint32_t strsize;
};


static inline void *addrconv(const struct binary *binary, addr_t addr) {
    return (void *) ((char *)binary->load_base + (intptr_t)(addr & 0x0fffffff));
}

static inline prange_t rangeconv(range_t range) {
    return (prange_t) {addrconv(range.binary, range.start), range.size};
}

static inline bool is_valid_address(void *address) {
    char c;
    return !mincore(address, 1, &c);
}

static inline bool is_valid_range(prange_t range) {
    char c;
    return !mincore(range.start, range.size, &c);
}

#define r(sz) \
static inline uint##sz##_t read##sz(const struct binary *binary, addr_t addr) { \
    return *(uint##sz##_t *)(addrconv(binary, addr)); \
}

r(8)
r(16)
r(32)
r(64)

void b_init(struct binary *binary);

void b_load_dyldcache(struct binary *binary, const char *path, bool pre_loaded);
range_t b_dyldcache_nth_segment(const struct binary *binary, int n);
void b_dyldcache_load_macho(struct binary *binary, const char *filename);

void b_running_kernel_load_macho(struct binary *binary);
void b_macho_load_symbols(struct binary *binary);
void b_load_macho(struct binary *binary, const char *path);
__attribute__((pure))
range_t b_macho_segrange(const struct binary *binary, const char *segname);
void *b_macho_offconv(const struct binary *binary, uint32_t fileoff);

addr_t b_sym(const struct binary *binary, const char *name, bool to_execute);
