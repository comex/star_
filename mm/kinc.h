#include <stdint.h>
#include <stdbool.h>
// This is stupid and generates wasteful code, but is necessary.  The BL instruction generated otherwise treats it as ARM and ignores the least-significant bit.
// A proper solution is apparently making the generated symbol have the right attribute, but I can't do that without... manually generating an ELF file?
#define LC __attribute__((long_call))

typedef uint32_t user_addr_t, vm_size_t, vm_address_t, boolean_t, size_t, vm_offset_t, vm_prot_t;
typedef void *vm_map_t;

extern vm_map_t kernel_map;
extern uint32_t *kernel_pmap;

LC void invalidate_icache(vm_offset_t addr, unsigned cnt, bool phys);

LC void *IOMalloc(size_t size);

LC int vm_allocate(vm_map_t map, vm_offset_t *addr, vm_size_t size, int flags);

LC int vm_deallocate(register vm_map_t map, vm_offset_t start, vm_size_t size);


LC int vm_protect(vm_map_t map, vm_offset_t start, vm_size_t size, boolean_t set_maximum, vm_prot_t new_protection);

LC int copyout(const void *kernel_addr, user_addr_t user_addr, vm_size_t nbytes);

LC void IOLog(const char *msg, ...) __attribute__((format (printf, 1, 2)));

#define prop(a, off, typ) *((typ *)(((char *) (a))+(off)))

#define NULL ((void *) 0)
