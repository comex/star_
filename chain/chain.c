#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <mach-o/loader.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <config/config.h>

#define IOLog ((void (*)(char *fmt, ...)) CONFIG_IOLOG)
#define kalloc ((void *(*)(unsigned int)) CONFIG_KALLOC)

#define LT __attribute__((section("__LOCKTEXT,__locktext")))
#define LD __attribute__((section("__LOCKDATA,__lockdata")))

#define CMD_ITERATE(hdr, cmd) for(struct load_command *cmd = (void *)((hdr) + 1), *end = (void *)((char *)(hdr) + (hdr)->sizeofcmds); cmd; cmd = (cmd->cmdsize > 0 && cmd->cmdsize < ((char *)end - (char *)cmd)) ? (void *)((char *)cmd + cmd->cmdsize) : NULL)

#define DEBUG 1

// stuff.c
#if DEBUG
// call this before clobbering the kernel kthx
extern void uart_set_rate(uint32_t rate);
extern void serial_putstring(const char *string);
extern void serial_puthex(uint32_t number);
#else
#define uart_set_rate(x)
#define serial_putstring(x)
#define serial_puthex(x)
#endif
extern int my_strcmp(const char *a, const char *b);
// {bcopy, bzero}.s
extern void *my_memcpy(void *dest, const void *src, size_t n);
extern void *my_memset(void *b, int c, size_t len);

LD struct mach_header *kern_hdr;
LD size_t kern_size;
LD char *devicetree;
LD size_t devicetree_size;

struct boot_args {
    uint16_t something; // 0
    uint16_t epoch; // must be 2 (6 on iPhone1,2 etc...?)
    uint32_t virtbase; // 4
    uint32_t physbase; // 8
    uint32_t size; // c
    uint32_t pt_addr; // 10: | 0x18 (eh, but we're 0x4000 off) -> ttbr1
    uint32_t end; // 14 (-> PE_state+4 - v_baseAddr) 5f700000
    uint32_t v_display; // 18 (-> PE_state+0x18 - v_display) 1
    uint32_t v_rowBytes;  // 1c (-> PE_state+8 - v_rowBytes) 2560
    uint32_t v_width; // 20 (-> PE_state+0xc - v_width) 640
    uint32_t v_height; // 24 (-> PE_state+0x10 - v_height) 960
    uint32_t v_depth; // 28 (-> PE_state+0x14 - v_depth) 65568?
    uint32_t unk2c; // 2c
    uint32_t dt_vaddr; // 30 (-> PE_state+0x6c)
    uint32_t dt_size; // 34
} __attribute__((packed));

LT static char *dt_recurse() {
    char *entry = devicetree; // in case we return it
    uint32_t n_properties = *((uint32_t *) devicetree); devicetree += 4;
    uint32_t n_children = *((uint32_t *) devicetree); devicetree += 4;
    serial_putstring("n_properties="); serial_puthex(n_properties); serial_putstring("\n");
    while(n_properties--) {
        char *name = devicetree;
        devicetree += 32;
        uint32_t length = *((uint32_t *) devicetree); devicetree += 4;
        char *value = devicetree;
        if(!my_strcmp(name, "name") && !my_strcmp(value, "memory-map")) {
            return entry;
        }
        devicetree += (length + 3) & ~3;
    }
    while(n_children--) {
        void *result = dt_recurse();
        if(result) return result;
    }
    // we didn't find it...
    return NULL;
}

struct dt_mm_entry {
    struct {
        uint32_t base;
        uint32_t size;
    } *p;
    char *name;
};
LD static struct dt_mm_entry dt_mm_entries[32];
LD static int n_dt_mm_entries;

__attribute__((always_inline)) static inline void invalidate_tlb() {
    asm volatile("mov r2, #0;"
                 "mcr p15, 0, r2, c8, c7, 0;" // invalidate entire unified TLB
                 "mcr p15, 0, r2, c8, c6, 0;" // invalidate entire data TLB
                 "mcr p15, 0, r2, c8, c5, 0;"
                 "dmb; dsb; isb" ::: "r2"); // invalidate entire instruction TLB
                 
}

LT static void flush_cache(void *start, size_t size) {
    // flush/invalidate
    uint32_t start_ = (uint32_t) start;
    for(uint32_t addr = (start_ & ~0x3f); addr < (start_ + size); addr += 0x40) {
        asm volatile("mcr p15, 0, %0, c7, c14, 1" :: "r"(addr));
        asm volatile("mcr p15, 0, %0, c7, c5, 1" :: "r"(addr));
    }
}

__attribute__((noreturn))
LT static void vita(uint32_t args_phys, uint32_t jump_phys) {
#if DEBUG
    // just test that we can actually access args_phys
    (void) *((volatile uint32_t *) args_phys);
    while(0 == (*((volatile uint32_t *) 0xc0000010) & 4));
    *((volatile char *) 0xc0000020) = '!';
#endif

    asm volatile("mcr p15, 0, r2, c7, c5, 0;" // redundantly kill icache
                 "mrc p15, 0, r2, c1, c0, 0;"
                 "bic r2, #0x1000;"
                 "bic r2, #0x7;"
                 "mcr p15, 0, r2, c1, c0, 0;"
                 // http://lists.infradead.org/pipermail/barebox/2010-January/000528.html
                 ::: "r2");

     
    invalidate_tlb();
    asm volatile("mov r2, #0;"
                 "mcr p15, 0, r2, c7, c5, 6;" // invalidate branch predictor
                 "mcr p15, 0, r2, c7, c10, 4;" // CP15DSB
                 "mcr p15, 0, r2, c7, c5, 4;" // CP15ISB
                 ::: "r2"); 

#if DEBUG
    // n.b. this is a physical address
    while(0 == (*((volatile uint32_t *) 0x82500010) & 4));
    *((volatile char *) 0x82500020) = '?';
#endif

    asm volatile("mov r0, %0;"
                 "bx %1"
                 :: "r"(args_phys), "r"(jump_phys));
    while(1);
}

LT static void load_it() {
    uart_set_rate(115200);
    serial_putstring("Hi\n");
    // no kanye 
    asm volatile("cpsid if");

    struct boot_args *args = (struct boot_args *) 0x809d6000; // XXX use ttbr1?
    args->v_display = 0; // verbose
    
    serial_putstring("Hi "); serial_puthex(args->dt_vaddr); serial_putstring("\n");

    my_memcpy((void *) args->dt_vaddr, devicetree, devicetree_size);
    devicetree = (void *) args->dt_vaddr;
    // search the devicetree for memory-map
    char *entry = dt_recurse();

    if(!entry) {
        serial_putstring("Didn't find memory-map!\n");
        return;
    }

    n_dt_mm_entries = 0;

    uint32_t n_properties = *((uint32_t *) entry); entry += 4;
    entry += 4; // n_children

    while(n_properties--) {
        char *name = entry;
        entry += 32;
        uint32_t length = *((uint32_t *) entry); entry += 4;
        if(my_strcmp(name, "AAPL,phandle") && my_strcmp(name, "name")) {
            serial_putstring(name); serial_putstring("\n");
            dt_mm_entries[n_dt_mm_entries].name = name;
            dt_mm_entries[n_dt_mm_entries].p = (void *) entry;
            n_dt_mm_entries++;
        }
        entry += (length + 3) & ~3;
    }
   
    serial_putstring("# placeholder entries: "); serial_puthex(n_dt_mm_entries); serial_putstring("\n");

    int i_dt_mm_entries = 0;

    uint32_t jump_addr = 0;

    serial_putstring("magic: "); serial_puthex(kern_hdr->magic); serial_putstring("\n");

    CMD_ITERATE(kern_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            struct section *sections = (void *) (seg + 1);
            struct dt_mm_entry *an_entry = &dt_mm_entries[i_dt_mm_entries++];
            serial_putstring(seg->segname); serial_putstring("\n");
            an_entry->p->base = seg->vmaddr;
            an_entry->p->size = seg->vmsize;
            my_memcpy(an_entry->name, seg->segname, 16);

            if(seg->filesize > 0) {
                my_memcpy((void *) seg->vmaddr, ((char *) kern_hdr) + seg->fileoff, seg->filesize);
                flush_cache((void *) seg->vmaddr, seg->filesize);
            }

            for(int i = 0; i < seg->nsects; i++) {
                struct section *sect = &sections[i];

                if((sect->flags & SECTION_TYPE) == S_ZEROFILL) {
                    my_memset((void *) seg->vmaddr, 0, seg->vmsize);
                }
            }
        } else if(cmd->cmd == LC_UNIXTHREAD) {
            struct {
                uint32_t cmd;
                uint32_t cmdsize;
                uint32_t flavor;
                uint32_t count;
                arm_thread_state_t state;
            } *ut = (void *) cmd;
            serial_putstring("got UNIXTHREAD flavor="); serial_puthex(ut->flavor); serial_putstring("\n");
            if(ut->flavor == ARM_THREAD_STATE) {
                jump_addr = (uint32_t) ut->state.__pc;
            }
        }
        serial_putstring(" ok\n");
    }

    serial_putstring("total used: "); serial_puthex(i_dt_mm_entries); serial_putstring("\n");

    serial_putstring("jump_addr: "); serial_puthex((uint32_t) jump_addr); serial_putstring("\n");

    // "In addition, if the physical address of the code that enables or disables the MMU differs from its MVA, instruction prefetching can cause complications. Therefore, ARM strongly recommends that any code that enables or disables the MMU has identical virtual and physical addresses."
    uint32_t ttbr1;
    asm("mrc p15, 0, %0, c2, c0, 1" :"=r"(ttbr1) :);
    uint32_t *pt = (uint32_t *) ((ttbr1 & 0xffffc000) + args->virtbase - args->physbase);

    serial_putstring("pt: "); serial_puthex((uint32_t) pt); serial_putstring("  old entry: "); serial_puthex(pt[0x400]);  serial_putstring("  at 80000000: "); serial_puthex(pt[0x800]); serial_putstring("\n");

    for(uint32_t i = 0x400; i < 0x420; i++) {
        pt[i] = (i << 20) | 0x40c0e;
        // pt[0x400/*00000*/] = 0x40040c0e;
    }

    serial_putstring("invalidating stuff\n");

    flush_cache(&pt[0x400], 0x20*sizeof(uint32_t));
    invalidate_tlb();
    
    uint32_t sz = 0x100;
    serial_putstring("memcpy sz:"); serial_puthex(sz); serial_putstring("\n");

    my_memcpy((void *) 0x40000000, (void *) (((uint32_t) vita) & ~1), sz);

    serial_putstring("-> vita ");

    uint32_t jump_phys = jump_addr + args->physbase - args->virtbase;
    uint32_t args_phys = ((uint32_t)args) + args->physbase - args->virtbase;

    serial_puthex(jump_phys);
    serial_putstring(" ");
    serial_puthex(args_phys);
    serial_putstring(" btw, what I'm jumping to looks like ");
    serial_puthex(*((uint32_t *) jump_addr));
    serial_putstring("\n");

    ((void (*)(uint32_t, uint32_t)) 0x40000001)(args_phys, jump_phys);

    serial_putstring("it returned?\n");
}

LT int ok_go(void *p, void *uap, int32_t *retval) {
    IOLog("Whoa, I'm here!...\n");
    *((uint32_t *) CONFIG_SYSENT_PATCH) = CONFIG_SYSENT_PATCH_ORIG;
    load_it();
    *retval = -1;
    return 0;
}

