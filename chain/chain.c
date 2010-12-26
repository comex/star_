#include "chain.h"

#define put(x) do { serial_putstring(#x ": "); serial_puthex((uint32_t) x); serial_putstring("\n"); } while(0)

#define CMD_ITERATE(hdr, cmd) for(struct load_command *cmd = (void *)((hdr) + 1), *end = (void *)((char *)(hdr) + (hdr)->sizeofcmds); cmd; cmd = (cmd->cmdsize > 0 && cmd->cmdsize < (uint32_t)((char *)end - (char *)cmd)) ? (void *)((char *)cmd + cmd->cmdsize) : NULL)

extern int (*PE_halt_restart)(unsigned int type);
extern int PEHaltRestart(unsigned int type);
#define kPEHaltCPU 0
#define kPERestartCPU 1

static struct mach_header *kern_hdr;
static size_t kern_size;
static char *devicetree;
static size_t devicetree_size;

static volatile char *vic;
static struct boot_args *args_final;
static char *devicetree_final;
static uint32_t *pagetable_final;
    
static struct boot_args *orig_args;

static struct boot_args *const args_storage = (void *) 0x8000f000;
static char *const pagetable_storage = (void *) 0x80010000;

struct boot_args {
    uint16_t something; // 0 - 1
    uint16_t epoch; // 2 - must be 2 (6 on iPhone1,2 etc...?)
    uint32_t virtbase; // 4
    uint32_t physbase; // 8
    uint32_t size; // c
    uint32_t pt_paddr; // 10: | 0x18 (eh, but we're 0x4000 off) -> ttbr1
    uint32_t v_baseAddr; // 14 (-> PE_state+4 - v_baseAddr) 5f700000
    uint32_t v_display; // 18 (-> PE_state+0x18 - v_display) 1
    uint32_t v_rowBytes;  // 1c (-> PE_state+8 - v_rowBytes) 2560
    uint32_t v_width; // 20 (-> PE_state+0xc - v_width) 640
    uint32_t v_height; // 24 (-> PE_state+0x10 - v_height) 960
    uint32_t v_depth; // 28 (-> PE_state+0x14 - v_depth) 65568?
    uint32_t unk2c; // 2c
    char     *dt_vaddr; // 30 (-> PE_state+0x6c)
    uint32_t dt_size; // 34
    char     cmdline[]; // 38
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
static const char **placeholders_p;

__attribute__((always_inline)) static inline void invalidate_tlb() {
    asm volatile("mov r2, #0;"
                 "mcr p15, 0, r2, c8, c7, 0;" // invalidate entire unified TLB
                 "mcr p15, 0, r2, c8, c6, 0;" // invalidate entire data TLB
                 "mcr p15, 0, r2, c8, c5, 0;" // invalidate entire instruction TLB
                 "mcr p15, 0, r2, c7, c10, 4;" // DSB
                 "mcr p15, 0, r2, c7, c5, 4;" // ISB
                 "mcr p15, 0, r2, c7, c5, 6;" // branch predictor
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
    //serial_putstring("vita\n");
    asm volatile("mcr p15, 0, r2, c7, c5, 0;" // redundantly kill icache
                 "mrc p15, 0, r2, c1, c0, 0;"
                 "bic r2, #0x1000;"
                 "bic r2, #0x7;"
                 "mcr p15, 0, r2, c1, c0, 0;"
                 // http://lists.infradead.org/pipermail/barebox/2010-January/000528.html
                 ::: "r2");

     
    invalidate_tlb();

    // This appears to work (and avoid GCC complaining about noreturn functions returning), but it also generates a warning.  I don't know how to get rid of it.
    //((void (__attribute__((noreturn)) *)(uint32_t)) jump_phys)(args_phys);
    __attribute__((noreturn)) void (*ptr)(uint32_t) = (void *) jump_phys;
    ptr(args_phys);
}

static int phase_2(unsigned int type);

static void *allocate_memory_area(char *memory_map_entry, const char *name, size_t size) {
    uint32_t max = 0, *max_p = &max;

    char *e = memory_map_entry;
    dt_prop_iterate(&e, ^bool(char *name, char *value, size_t l) {
        if(strcmp(name, "name") && strcmp(name, "AAPL,phandle")) {
            struct memory_map_entry *entry = (void *) value;
            uint32_t new_max = (uint32_t) entry->address + entry->size;
            if(new_max > *max_p) *max_p = new_max;
        }
        return true;
    });
    
    max = (max + 0x3fff) & ~0x3fff;

    struct memory_map_entry new;
    new.address = (void *) max;
    new.size = size;

    if(!dt_entry_set_prop(memory_map_entry, *placeholders_p++, name, &new, sizeof(new))) {
        IOLog("Could not put %s in memory map\n", name);
        return NULL;
    }

    return (void *) (max + orig_args->virtbase - orig_args->physbase);
}

static struct boot_args *find_orig_args() {
    uint32_t pt_phys;
    asm("mrc p15, 0, %0, c2, c0, 1" :"=r"(pt_phys)); // ttbr1
    struct boot_args *oa = (void *) (pt_phys + 0x40000000);
    for(int i = 0; i < 0x10000; i++) {
        if((oa->virtbase == 0x80000000 || oa->virtbase == 0xc0000000) && \
           oa->physbase == 0x40000000 && \
           oa->v_rowBytes == oa->v_width * 4) {
            return oa;
        }
        oa = (void *) (((uint32_t *) oa) - 1);
    }

    IOLog("Could not find orig_args\n");
    return NULL;
}

static int phase_1() {
    if(!(orig_args = find_orig_args())) return -1;

    if(!(vic = map_from_iokit("vic"))) return -1;

    uart_set_rate(115200);

    serial_putstring("Hello... I am at ");
    serial_puthex((uint32_t) phase_1);
    serial_putstring(" aka ");
    serial_puthex(virt_to_phys(phase_1));
    serial_putstring("\n");

    trace;

    char *dt;
    
    dt = devicetree;
    dt_super_iterate(&dt);
    
    // XXX compatibility of the frequency thing?
    
    uint32_t frequency = 0;
    if(!dt_entry_set_prop(devicetree, "clock-frequency", NULL, &frequency, sizeof(frequency))) {
        IOLog("couldn't set /clock-frequency to 0\n");
        return -1;
    }

    trace;
    
    dt = devicetree;
    char *cpus = dt_get_entry(&dt, "IODeviceTree:/cpus/cpu0");
    if(!cpus) {
        IOLog("couldn't get /cpus/cpu0\n");
        return -1;
    }

    frequency = 200000000;
    if(!dt_entry_set_prop(cpus, "memory-frequency", NULL, &frequency, sizeof(frequency))) {
        IOLog("couldn't set /cpus/cpu0/memory-frequency\n");
        return -1;
    }
    
    frequency = 800000000;
    if(!dt_entry_set_prop(cpus, "clock-frequency", NULL, &frequency, sizeof(frequency))) {
        IOLog("couldn't set /cpus/cpu0/clock-frequency\n");
        return -1;
    }

    trace;
    
#if PUTC
    trace;

    dt = devicetree;
    char *usb_complex_entry = dt_get_entry(&dt, "IODeviceTree:/arm-io/usb-complex");
    if(!usb_complex_entry || !dt_entry_set_prop(usb_complex_entry, "compatible", NULL, "", 1)) { 
        IOLog("wtf usb-complex\n");
        return -1;
    }

    dt = devicetree;
    char *usb_device_entry = dt_get_entry(&dt, "IODeviceTree:/arm-io/usb-complex/usb-device");
    if(!usb_device_entry || !dt_entry_set_prop(usb_device_entry, "compatible", NULL, "", 1)) { 
        IOLog("wtf usb-device\n");
        return -1;
    }
#endif
    
    trace;

    dt = devicetree;
    char *chosen_entry = dt_get_entry(&dt, "IODeviceTree:/chosen");
    if(!chosen_entry || !dt_entry_set_prop(chosen_entry, "debug-enabled", NULL, "\x01\x00\x00\x00", 4)) {
        IOLog("wtf chosen\n");
        return -1;
    }
    
    dt = devicetree;
    char *memory_map_entry = dt_get_entry(&dt, "IODeviceTree:/chosen/memory-map");
    if(!memory_map_entry) {
        IOLog("wtf memory-map\n");
        return -1;
    }
    
    placeholders_p = &placeholders[0];
    
    trace;
    
    CMD_ITERATE(kern_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            // update the devicetree
            static char buf[32] = "Kernel-";
            my_memcpy(buf + 7, seg->segname, 16);
            struct memory_map_entry s;
            s.address = (void *) (seg->vmaddr + orig_args->physbase - orig_args->virtbase);
            s.size = seg->vmsize;
            dt_entry_set_prop(memory_map_entry, *placeholders_p++, buf, &s, sizeof(s));
        }
    }
    
    trace;

    args_final = allocate_memory_area(memory_map_entry, "BootArgs", 0x1000);
    devicetree_final = allocate_memory_area(memory_map_entry, "DeviceTree", devicetree_size);
    pagetable_final = allocate_memory_area(memory_map_entry, "PageTable", 0x10000); // just in case

    trace;

    put(args_final);
    put(devicetree_final);
    put(pagetable_final);

    if(!args_final || !devicetree_final || !pagetable_final) {
        return -1;
    }


#if 0
    serial_important = true;
    serial_putstring("New DeviceTree:\n");
    serial_puthexbuf((void *) devicetree, 0xde78);
    serial_putstring("\n");
    serial_important = false;
#endif

#if !DEBUG
    serial_putstring("calling PEHaltRestart\n");

    PE_halt_restart = phase_2;
    PEHaltRestart(kPERestartCPU);
#else
    serial_putstring("going directly to phase_2\n");
    phase_2(0x1234);
#endif

    return 0;
}

#if DEBUG || PUTC
static void place_thing(const void *start, const void *end, uint32_t addr, uint32_t hookaddr) {
    uint32_t s = (uint32_t) start;
    uint32_t e = (uint32_t) end;
    bool thing_thumb = false;
    if(s & 1) {
        s &= ~1;
        start = (void *) s;
        thing_thumb = true;
    }
    if(addr & 1) {
        addr &= ~1;
        thing_thumb = true;
    }
    my_memcpy((void *) addr, start, e - s);
    if(thing_thumb) {
        addr |= 1;
    }
    uint32_t jump_to_fu_arm[] = {0xe51ff004, addr};
    uint16_t jump_to_fu_thumb_al4[] = {0xf8df, 0xf000, addr & 0xffff, addr >> 16};
    uint16_t jump_to_fu_thumb_notal4[] = {0xbf00, 0xf8df, 0xf000, addr & 0xffff, addr >> 16};
    switch(hookaddr & 3) {
    case 1:
        my_memcpy((void *) (hookaddr - 1), jump_to_fu_thumb_al4, sizeof(jump_to_fu_thumb_al4));
        break;
    case 3:
        my_memcpy((void *) (hookaddr - 1), jump_to_fu_thumb_notal4, sizeof(jump_to_fu_thumb_notal4));
        break;
    case 0:
    case 2:
        my_memcpy((void *) hookaddr, jump_to_fu_arm, sizeof(jump_to_fu_arm));
        break;
    }
}

static void place_annoyance(uint32_t addr, uint32_t hookaddr, uint32_t hooksize) {
    if(hookaddr & 1) {
        if(hooksize > 16 || hooksize < (((hookaddr & 3) == 3) ? 10 : 8)) {
            serial_putstring("place_annoyance: hooksize is wrong\n");
            return;
        }

        extern void annoyance_start(), annoyance_end();
        extern char annoyance_space[];
        extern void *annoyance_iolog, *annoyance_come_from, *annoyance_return_to;
        my_memset(annoyance_space, 0, 16);
        my_memcpy(annoyance_space, (void *) (hookaddr - 1), hooksize);
        annoyance_iolog = IOLog;
        annoyance_come_from = (void *) hookaddr;
        annoyance_return_to = (void *) (hookaddr + hooksize);

        place_thing(annoyance_start, annoyance_end, addr | 1, hookaddr);
        
        serial_puthex(addr);
        serial_putstring(": ");
        serial_puthex(*((uint32_t *) addr));
        serial_putstring("\n");
    } else {
        serial_important = true;
        serial_putstring("place_annoyance: not thumb\n");
        serial_important = false;
    }
}

static void place_bxsp(uint32_t addr) {
    if(addr & 1) {
        *((uint16_t *) (addr - 1)) = 0x4768;
    } else {
        *((uint32_t *) addr) = 0xe12fff1d;
    }
}
#endif // DEBUG || PUTC

static int phase_2(unsigned int type) {
    // no kanye 
    asm volatile("cpsid if");

    serial_putstring("phase_2\n");

    put(type);
    put(orig_args);
    put(args_storage);
    put(pagetable_storage);
    
    // Anyway, at this point we can use the *_final pointers.

    // stuff from openiboot
    for(int i = 0; i <= 3; i++) {
        volatile uint32_t *thisvic = (volatile void *) (vic + 0x10000 * i);
        thisvic[0x14/4] = 0xffffffff;
        // openiboot does this but I don't think iBoot does - is it correct here?
        thisvic[0x24/4] = 0xffff;
        for(int j = 0; j < 0x20; j++) {
            thisvic[0x100/4 + j] = i*0x20 + j;
        }
    }

    serial_putstring("copying 1\n");
    my_memcpy(args_storage, orig_args, sizeof(struct boot_args));

    uint32_t ttbr1;
    asm("mrc p15, 0, %0, c2, c0, 1" :"=r"(ttbr1));
    uint32_t *orig_pt = (void *) ((ttbr1 & ~0x3fff) + orig_args->virtbase - orig_args->physbase);
    put(orig_pt);
    my_memcpy(pagetable_storage, orig_pt, 0x10000);
    flush_cache(pagetable_storage, 0x10000);
    
    uint32_t pagetable_storage_paddr = ((uint32_t) pagetable_storage) - orig_args->virtbase + orig_args->physbase;
    put(pagetable_storage_paddr);
    asm volatile("mcr p15, 0, %0, c2, c0, 1" ::"r"(pagetable_storage_paddr | 0x18)); // ttbr1
    invalidate_tlb();

    serial_putstring("copying 2\n");
    my_memcpy(args_final, args_storage, sizeof(struct boot_args));
    my_memcpy(pagetable_final, pagetable_storage, 0x10000);
    flush_cache(pagetable_final, 0x10000);

    serial_putstring("okay\n");

    args_final->pt_paddr = ((uint32_t) pagetable_final) - args_final->virtbase + args_final->physbase;

    serial_putstring("about to do the mcr, pt_paddr = "); serial_puthex(args_final->pt_paddr); serial_putstring("\n");

    asm volatile("mcr p15, 0, %0, c2, c0, 1" ::"r"(args_final->pt_paddr | 0x18)); // ttbr1
    
    invalidate_tlb();

    for(uint32_t i = 0x400; i < 0x5e0; i++) {
        pagetable_final[i] = (i << 20) | 0x40c0e;
    }

    for(uint32_t i = 0x5e0; i < 0x700; i++) {
        pagetable_final[i] = (i << 20) | 0x40c02; // device
    }

    invalidate_tlb();
    flush_cache(pagetable_final, 0x10000);
    invalidate_tlb();

    serial_putstring("did the mcr\n");

    kern_hdr = (void *) virt_to_phys(kern_hdr);
    serial_putstring("new "); put(kern_hdr);

    args_final->dt_vaddr = devicetree_final;
    args_final->v_display = 0; // verbose
    
    my_memcpy(devicetree_final, devicetree, devicetree_size);
    devicetree = devicetree_final;

    serial_putstring("copying segments\n");
    
    uint32_t jump_addr = 0;
    CMD_ITERATE(kern_hdr, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *seg = (void *) cmd;
            struct section *sections = (void *) (seg + 1);
            serial_putstring(seg->segname);

            if(seg->filesize > 0) {
                my_memcpy((void *) seg->vmaddr, ((char *) kern_hdr) + seg->fileoff, seg->filesize);
            }
            
            if(seg->vmsize > seg->filesize) {
                my_memset((char *) seg->vmaddr + seg->filesize, 0, seg->vmsize - seg->filesize);
            }
            serial_putstring(" flush");
            
            flush_cache((void *) seg->vmaddr, seg->vmsize);

            serial_putstring(" ok\n");
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
                // like iBoot, ignore everything but PC
                jump_addr = (uint32_t) ut->state.__pc;
            }
        }
    }

    serial_putstring("total used: "); serial_puthex(placeholders_p - placeholders); serial_putstring("\n");

    put(jump_addr);

/*
#if PUTC || HAVE_SERIAL
    static const char c[] = " io=4095 serial=15 diag=15 sdio.debug.init-delay=10000 sdio.log.level=65535 sdio.log.flags=1"
#   if PUTC
        " debug=0xa"
#   endif
    ;
#else
    static const char c[] = " io=4095 diag=15";
#endif
    */

    static const char c[] = "-v io=4095 sdio.log.level=65535 sdio.log.flags=1 kdp_match_name=serial "
#if 0
    "sdio.debug.init-delay=10000 "
#endif
#if HAVE_SERIAL || PUTC
    "serial=1 "
#endif
#if HAVE_SERIAL && 0
    "debug=0x225 "
#else
    "debug=0x400 "
#endif
    ;

    my_memcpy(args_final->cmdline, c, sizeof(c));

#if DEBUG
    // Note that for now it is necessary to ensure that the affected instruction range does not use PC, and ends on an instruction boundary.  Annoying, but...
    place_annoyance(SCRATCH + 0x1000, 0x80790f45, 8); 
    place_annoyance(SCRATCH + 0x1100, 0x80791001, 8); 
    place_annoyance(SCRATCH + 0x1200, 0x80791047, 10); 
    place_annoyance(SCRATCH + 0x1300, 0x807910a1, 8); 
    place_annoyance(SCRATCH + 0x1400, 0x807911cd, 10);
    place_annoyance(SCRATCH + 0x1500, 0x80791255, 10);
    place_annoyance(SCRATCH + 0x1600, 0x80791267, 10);
    place_annoyance(SCRATCH + 0x1700, 0x8079128d, 8);
    place_annoyance(SCRATCH + 0x1800, 0x807912c5, 8);
    place_annoyance(SCRATCH + 0x1900, 0x80791319, 8);
    place_annoyance(SCRATCH + 0x1a00, 0x807913bf, 10);
    place_annoyance(SCRATCH + 0x1b00, 0x807913d9, 8);
    place_annoyance(SCRATCH + 0x1c00, 0x80791401, 8);
    place_annoyance(SCRATCH + 0x1d00, 0x80791429, 8);
    //place_annoyance(SCRATCH + 0x1e00, 0x80791447, 12);
    //place_annoyance(SCRATCH + 0x1f00, 0x80791453, 12);
    //place_bxsp(0x80791453);
    //place_annoyance(SCRATCH + 0x2000, 0x807914bf, 10);
    //place_annoyance(SCRATCH + 0x2100, 0x807914d1, 8);
    //place_annoyance(SCRATCH + 0x2200, 0x807914f9, 12);
    place_bxsp(0x8079152b);
#endif

#if PUTC
    extern void putc_start(), putc_end();
    place_thing(putc_start, putc_end, SCRATCH, CONSLOG_PUTC);
    place_thing(putc_start, putc_end, SCRATCH, CONSDEBUG_PUTC);
    place_thing(putc_start, putc_end, SCRATCH, PUTCHAR);
    place_thing(putc_start, putc_end, SCRATCH, CNPUTC);
    
    //place_thing(fffuuu, SCRATCH + 0x10000, 0x80867645);
#endif // PUTC
    
    // "In addition, if the physical address of the code that enables or disables the MMU differs from its MVA, instruction prefetching can cause complications. Therefore, ARM strongly recommends that any code that enables or disables the MMU has identical virtual and physical addresses."

    my_memset((void *) args_final->v_baseAddr, 0xff, args_final->v_height * args_final->v_rowBytes);
    
    my_memcpy((void *) 0x40000000, (void *) (((uint32_t) vita) & ~1), 0x100);

    uint32_t jump_phys = jump_addr + args_final->physbase - args_final->virtbase;
    uint32_t args_phys = ((uint32_t)args_final) + args_final->physbase - args_final->virtbase;

    serial_putstring("boot args: "); serial_putstring(args_final->cmdline); serial_putstring("\n");
    serial_putstring("taking the plunge\n");

#if PUTC
    *((uint32_t *) 0x8000011c) = 0; // don't move this line up
#endif

    ((void (*)(uint32_t, uint32_t)) 0x40000000)(args_phys, jump_phys);

    serial_putstring("it returned?\n");
    return 0;
}

typedef uint32_t user_addr_t;

struct args {
    user_addr_t kern;
    size_t kern_size;
    user_addr_t devicetree;
    size_t devicetree_size;
};

int ok_go(void *p, struct args *uap, int32_t *retval) {
    kern_hdr = IOMallocContiguous(uap->kern_size, 1, NULL);
    copyin(uap->kern, kern_hdr, uap->kern_size);
    devicetree = kalloc(uap->devicetree_size);
    devicetree_size = uap->devicetree_size;
    copyin(uap->devicetree, devicetree, uap->devicetree_size);

    flush_cache(kern_hdr, uap->kern_size);
    flush_cache(devicetree, uap->devicetree_size);

    *retval = phase_1();
    if(*retval) {
        IOFreeContiguous(kern_hdr, uap->kern_size);
        kfree(devicetree, uap->devicetree_size);
    }
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


