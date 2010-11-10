#include "chain.h"

#define CMD_ITERATE(hdr, cmd) for(struct load_command *cmd = (void *)((hdr) + 1), *end = (void *)((char *)(hdr) + (hdr)->sizeofcmds); cmd; cmd = (cmd->cmdsize > 0 && cmd->cmdsize < (uint32_t)((char *)end - (char *)cmd)) ? (void *)((char *)cmd + cmd->cmdsize) : NULL)


struct mach_header *kern_hdr;
size_t kern_size;
char *devicetree;
size_t devicetree_size;

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
    char cmdline[]; // 38
} __attribute__((packed));

struct memory_map_entry {
    void *address;
    uint32_t size;
} __attribute__((packed));

static const char *placeholders[] = {
    "MemoryMapReserved-0",
    "MemoryMapReserved-1",
    "MemoryMapReserved-2",
    "MemoryMapReserved-3",
    "MemoryMapReserved-4",
    "MemoryMapReserved-5",
    "MemoryMapReserved-6",
    "MemoryMapReserved-7",
    "MemoryMapReserved-8",
    "MemoryMapReserved-9",
    "MemoryMapReserved-10",
    "MemoryMapReserved-11",
    "MemoryMapReserved-12",
    "MemoryMapReserved-13",
    "MemoryMapReserved-14",
    "MemoryMapReserved-15"
};


__attribute__((always_inline)) static inline void invalidate_tlb() {
    asm volatile("mov r2, #0;"
                 "mcr p15, 0, r2, c8, c7, 0;" // invalidate entire unified TLB
                 "mcr p15, 0, r2, c8, c6, 0;" // invalidate entire data TLB
                 "mcr p15, 0, r2, c8, c5, 0;" // invalidate entire instruction TLB
                 "mcr p15, 0, r2, c7, c10, 4;" // DSB
                 "mcr p15, 0, r2, c7, c5, 4;" // ISB
                 ::: "r2");
                 
}

static void flush_cache(void *start, size_t size) {
    // flush/invalidate
    uint32_t start_ = (uint32_t) start;
    for(uint32_t addr = (start_ & ~0x3f); addr < (start_ + size); addr += 0x40) {
        asm volatile("mcr p15, 0, %0, c7, c14, 1" :: "r"(addr));
        asm volatile("mcr p15, 0, %0, c7, c5, 1" :: "r"(addr));
    }
}

__attribute__((noreturn))
static void vita(uint32_t args_phys, uint32_t jump_phys) {
    asm volatile("mcr p15, 0, r2, c7, c5, 0;" // redundantly kill icache
                 "mrc p15, 0, r2, c1, c0, 0;"
                 "bic r2, #0x1000;"
                 "bic r2, #0x7;"
                 "mcr p15, 0, r2, c1, c0, 0;"
                 // http://lists.infradead.org/pipermail/barebox/2010-January/000528.html
                 ::: "r2");

     
    invalidate_tlb();
    asm volatile("mcr p15, 0, r0, c7, c5, 6;"); // invalidate branch predictor

#if 0
    // n.b. this is a physical address
    while(0 == (*((volatile uint32_t *) 0x82500010) & 4));
    *((volatile char *) 0x82500020) = '!';
#endif

    // This appears to work (and avoid GCC complaining about noreturn functions returning), but it also generates a warning.  I don't know how to get rid of it.
    //((void (__attribute__((noreturn)) *)(uint32_t)) jump_phys)(args_phys);
    __attribute__((noreturn)) void (*ptr)(uint32_t) = (void *) jump_phys;
    ptr(args_phys);
}

static void load_it() {
    uart_set_rate(115200);

    char *dt;
    
    dt = devicetree;
    dt_super_iterate(&dt);

    dt = devicetree;
    char *memory_map_entry = dt_get_entry(&dt, "IODeviceTree:/chosen/memory-map");
    if(!memory_map_entry) {
        IOLog("wtf\n");
        return;
    }

    struct boot_args *args = (struct boot_args *) 0x809d6000; // XXX use ttbr1?
    
    const char **placeholders_p = placeholders;
    struct memory_map_entry s;
    s.address = args;
    s.size = sizeof(*args);
    if(!dt_entry_set_prop(memory_map_entry, *placeholders_p++, "BootArgs", &s, sizeof(s))) {
        IOLog("Could not put BootArgs in memory map\n");
        return;
    }
    
    s.address = (void *) args->dt_vaddr;
    s.size = devicetree_size;
    if(!dt_entry_set_prop(memory_map_entry, *placeholders_p++, "DeviceTree", &s, sizeof(s))) {
        IOLog("Could not put DeviceTree in memory map\n");
        return;
    }
    
    CMD_ITERATE(kern_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            // update the devicetree
            static char buf[32] = "Kernel-";
            my_memcpy(buf + 7, seg->segname, 16);
            struct memory_map_entry s;
            s.address = (void *) (seg->vmaddr + args->physbase - args->virtbase);
            s.size = seg->vmsize;
            dt_entry_set_prop(memory_map_entry, *placeholders_p++, buf, &s, sizeof(s));
        }
    }

#if DEBUG && 0
    serial_important = true;
    serial_putstring("New DeviceTree:\n");
    serial_puthexbuf((void *) devicetree, 0xde78);
    serial_putstring("\n");
    serial_important = false;
#endif

    asm volatile("cpsid if");
    // no kanye 
    args->v_display = 0; // verbose
    
    serial_putstring("Hi "); serial_puthex(args->dt_vaddr); serial_putstring("\n");

    my_memcpy((void *) args->dt_vaddr, devicetree, devicetree_size);
    devicetree = (void *) args->dt_vaddr;

    uint32_t jump_addr = 0;

    serial_putstring("magic: "); serial_puthex(kern_hdr->magic); serial_putstring("\n");

    CMD_ITERATE(kern_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            struct section *sections = (void *) (seg + 1);
            serial_putstring(seg->segname); serial_putstring("\n");

            if(seg->filesize > 0) {
                my_memcpy((void *) seg->vmaddr, ((char *) kern_hdr) + seg->fileoff, seg->filesize);
            }

            for(uint32_t i = 0; i < seg->nsects; i++) {
                struct section *sect = &sections[i];
                if((sect->flags & SECTION_TYPE) == S_ZEROFILL) {
                    my_memset((void *) sect->addr, 0, sect->size);
                }
            }
            
            flush_cache((void *) seg->vmaddr, seg->vmsize);
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

    serial_putstring("total used: "); serial_puthex(placeholders_p - placeholders); serial_putstring("\n");

    serial_putstring("jump_addr: "); serial_puthex((uint32_t) jump_addr); serial_putstring("\n");

#if HAVE_SERIAL
    static const char c[] = " io=65535 serial=15 debug=15 diag=15";
#else
    static const char c[] = " io=65535 debug=15 diag=15";
#endif
    my_memcpy(args->cmdline, c, sizeof(c));

    // "In addition, if the physical address of the code that enables or disables the MMU differs from its MVA, instruction prefetching can cause complications. Therefore, ARM strongly recommends that any code that enables or disables the MMU has identical virtual and physical addresses."
    uint32_t ttbr1;
    asm("mrc p15, 0, %0, c2, c0, 1" :"=r"(ttbr1) :);
    uint32_t *pt = (uint32_t *) ((ttbr1 & 0xffffc000) + args->virtbase - args->physbase);

    serial_putstring("pt: "); serial_puthex((uint32_t) pt); serial_putstring("  old entry: "); serial_puthex(pt[0x400]);  serial_putstring("  at 80000000: "); serial_puthex(pt[0x800]); serial_putstring("\n");
    
#if 1
    // debug a jump to 0
    *((uint32_t *) 0x80065090) = 0xe1a00000; // remove the kernel's zero...
    flush_cache((void *) 0x80065000, 0x1000);
    pt[0] = pt[0x1000] = (0x400 << 20) | 0x40c0e; // but wait...
    serial_putstring("setting "); serial_puthex((uint32_t) &pt[0]); serial_putstring("\n");
    // .long 0x4778; mov r10, lr; mov r11, sp; ldr pc, [pc, #-4]; .long 0x0badbadb
    static uint32_t omg[] = {0x00004778, 0xe1a0a00e, 0xe1a0b00d, 0xe51ff004, 0x0badbadb};
    my_memcpy((void *) 0x80000000, omg, sizeof(omg));
#endif

    for(uint32_t i = 0x400; i < 0x420; i++) {
        pt[i] = (i << 20) | 0x40c0e;
    }

#if 1
    extern void fffuuu_start(), fffuuu_end();
#   define fffuuu_addr 0x807d5518
    my_memcpy((void *) fffuuu_addr, (void *) fffuuu_start, ((uint32_t)fffuuu_end - (uint32_t)fffuuu_start));
    static uint32_t jump_to_fu_arm[] = {0xe51ff004, fffuuu_addr};
    static uint16_t jump_to_fu_thumb_al4[] = {0xf8df, 0xf000, fffuuu_addr & 0xffff, fffuuu_addr >> 16};
    static uint16_t jump_to_fu_thumb_notal4[] = {0xbf00, 0xf8df, 0xf000, fffuuu_addr & 0xffff, fffuuu_addr >> 16};
    // 80069acc - _sleh_abort
    // 80064310 - prefetch abort in system mode
    // 800643c8 - data abort in system mode
    my_memcpy((void *) 0x80064310, jump_to_fu_arm, sizeof(jump_to_fu_arm));
#endif

    serial_putstring("invalidating stuff\n");
    flush_cache(&pt[0], 0x2000*sizeof(uint32_t));
    invalidate_tlb();
    
    uint32_t sz = 0x100;
    serial_putstring("kernel_pmap: "); serial_puthex(*((uint32_t *) 0x8024e218)); serial_putstring("...\n");

    my_memcpy((void *) 0x40000000, (void *) (((uint32_t) vita) & ~1), 0x100);

    serial_putstring("-> vita ");

    uint32_t jump_phys = jump_addr + args->physbase - args->virtbase;
    uint32_t args_phys = ((uint32_t)args) + args->physbase - args->virtbase;

    serial_puthex(jump_phys);
    serial_putstring(" ");
    serial_puthex(args_phys);
    serial_putstring(" btw, what I'm jumping to looks like ");
    serial_puthex(*((uint32_t *) jump_addr));
    serial_putstring("\n");

    ((void (*)(uint32_t, uint32_t)) 0x40000000)(args_phys, jump_phys);

    serial_putstring("it returned?\n");
}

typedef uint32_t user_addr_t;

struct args {
    user_addr_t kern_hdr;
    size_t kern_size;
    user_addr_t devicetree;
    size_t devicetree_size;
};

extern void *kalloc(uint32_t size);
extern void kfree(void *data, uint32_t size);
extern int copyin(const user_addr_t user_addr, void *kernel_addr, uint32_t nbytes);

int ok_go(void *p, struct args *uap, int32_t *retval) {
    kern_hdr = kalloc(uap->kern_size);
    copyin(uap->kern_hdr, kern_hdr, uap->kern_size);
    devicetree = kalloc(uap->devicetree_size);
    devicetree_size = uap->devicetree_size;
    copyin(uap->devicetree, devicetree, uap->devicetree_size);

    load_it();

    kfree(kern_hdr, uap->kern_size);
    kfree(devicetree, uap->devicetree_size);
    *retval = -1;
    return 0;
}

struct proc;
typedef int32_t sy_call_t(struct proc *, void *, int *);
typedef void    sy_munge_t(const void *, void *);

struct sysent {     /* system call table */
    int16_t     sy_narg;    /* number of args */
    int8_t      sy_resv;    /* reserved  */
    int8_t      sy_flags;   /* flags */
    sy_call_t   *sy_call;   /* implementing function */
    sy_munge_t  *sy_arg_munge32; /* system call arguments munger for 32-bit process */
    sy_munge_t  *sy_arg_munge64; /* system call arguments munger for 64-bit process */
    int32_t     sy_return_type; /* system call return types */
    uint16_t    sy_arg_bytes;   /* Total size of arguments in bytes for
                     * 32-bit system calls
                     */
};
#define _SYSCALL_RET_INT_T      1   


extern struct sysent sysent[];

__attribute__((constructor))
static void init() {
    sysent[8] = (struct sysent){ 1, 0, 0, (void *) ok_go, NULL, NULL, _SYSCALL_RET_INT_T, sizeof(struct args) };
}


