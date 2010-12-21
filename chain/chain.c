#include "chain.h"

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
    uint32_t end; // 14 (-> PE_state+4 - v_baseAddr) 5f700000
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

static int find_orig_args() {
    uint32_t pt_phys;
    asm("mrc p15, 0, %0, c2, c0, 1" :"=r"(pt_phys)); // ttbr1
    orig_args = (void *) (pt_phys + 0x40000000);
    for(int i = 0; i < 0x10000; i++) {
        if((orig_args->virtbase == 0x80000000 || orig_args->virtbase == 0xc0000000) && \
           orig_args->physbase == 0x40000000 && \
           orig_args->v_rowBytes == orig_args->v_width * 4) {
            return 0;
        }
        orig_args = (void *) (((uint32_t *) orig_args) - 1);
    }

    IOLog("Could not find orig_args\n");
    return -1;
}

static int phase_1() {
    if(find_orig_args()) return -1;

    if(!(vic = map_from_iokit("vic"))) return -1;

    uart_set_rate(115200);

    serial_putstring("Hello\n");

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

    if(!args_final || !devicetree_final || !pagetable_final) {
        return -1;
    }


#if DEBUG && 0
    serial_important = true;
    serial_putstring("New DeviceTree:\n");
    serial_puthexbuf((void *) devicetree, 0xde78);
    serial_putstring("\n");
    serial_important = false;
#endif

    serial_putstring("calling PEHaltRestart\n");

    PE_halt_restart = phase_2;
    PEHaltRestart(kPERestartCPU);

    return 0;
}

static int phase_2(unsigned int type) {
    // no kanye 
    asm volatile("cpsid if");

    serial_putstring("phase_2\n");

#define put(x) do { serial_putstring(#x ": "); serial_puthex((uint32_t) x); serial_putstring("\n"); } while(0)
    put(type);
    put(orig_args);
    put(args_final);
    put(devicetree_final);
    put(pagetable_final);
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

    serial_putstring("did the mcr\n");

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
                // like iBoot, ignore everything but PC
                jump_addr = (uint32_t) ut->state.__pc;
            }
        }
        serial_putstring(" ok\n");
    }

    serial_putstring("total used: "); serial_puthex(placeholders_p - placeholders); serial_putstring("\n");

    serial_putstring("jump_addr: "); serial_puthex((uint32_t) jump_addr); serial_putstring("\n");

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

    static const char c[] = "serial=0 io=4095 debug=0x25 sdio.debug.init-delay=10000 sdio.log.level=65535 sdio.log.flags=1 kdp_match_name=serial";
    my_memcpy(args_final->cmdline, c, sizeof(c));

    // TODO fix this for armv6
#define place_thing(thing, addr, hookaddr) \
    do { \
        extern void thing##_start(), thing##_end(); \
        my_memcpy((void *) addr, (void *) thing##_start, ((uint32_t)thing##_end - (uint32_t)thing##_start)); \
        static uint32_t jump_to_fu_arm[] = {0xe51ff004, addr}; \
        static uint16_t jump_to_fu_thumb_al4[] = {0xf8df, 0xf000, addr & 0xffff, addr >> 16}; \
        static uint16_t jump_to_fu_thumb_notal4[] = {0xbf00, 0xf8df, 0xf000, addr & 0xffff, addr >> 16}; \
        switch(hookaddr & 3) { \
        case 3: \
            my_memcpy((void *) (hookaddr - 1), jump_to_fu_thumb_notal4, sizeof(jump_to_fu_thumb_notal4)); \
            break; \
        case 1: \
            my_memcpy((void *) (hookaddr - 1), jump_to_fu_thumb_al4, sizeof(jump_to_fu_thumb_al4)); \
            break; \
        case 2: \
        case 0: \
            my_memcpy((void *) hookaddr, jump_to_fu_arm, sizeof(jump_to_fu_arm)); \
            break; \
        } \
    } while(0)

    //*((uint16_t *) 0x807261d4) = 0x4770; // patch idie

#if PUTC
#ifdef TARGET_IPHONE3_1_4_1
// 80069acc - _sleh_abort
// 80064310 - prefetch abort in system mode
// 800643c8 - data abort in system mode
// 800152f1 - panic
// 80067c59 - Debugger
#define SCRATCH 0x807d5518
#define CONSLOG_PUTC 0x8001abb5
#define CONSDEBUG_PUTC 0x8001ab59
#define PUTCHAR 0x8015f259
#define CNPUTC 0x8006a93d

#elif defined(TARGET_IPAD1_1_4_2_1) 

#define SCRATCH 0x80851500
#define CONSLOG_PUTC 0x8001ab41
#define CONSDEBUG_PUTC 0x8001aae5
#define PUTCHAR 0x80160b01
#define CNPUTC 0x8006b951

#endif

    place_thing(putc, SCRATCH, CONSLOG_PUTC);
    place_thing(putc, SCRATCH, CONSDEBUG_PUTC);
    place_thing(putc, SCRATCH, PUTCHAR);
    place_thing(putc, SCRATCH, CNPUTC);
    
    //place_thing(fffuuu, SCRATCH + 0x10000, 0x80867645);
#endif // PUTC
    
    // "In addition, if the physical address of the code that enables or disables the MMU differs from its MVA, instruction prefetching can cause complications. Therefore, ARM strongly recommends that any code that enables or disables the MMU has identical virtual and physical addresses."

    serial_putstring("pagetable_final: "); serial_puthex((uint32_t) pagetable_final); serial_putstring("  old entry: "); serial_puthex(pagetable_final[0x400]);  serial_putstring("  at 80000000: "); serial_puthex(pagetable_final[0x800]); serial_putstring("\n");

    for(uint32_t i = 0x400; i < 0x420; i++) {
        pagetable_final[i] = (i << 20) | 0x40c0e;
    }



    serial_putstring("invalidating stuff\n");
    flush_cache(pagetable_final, 0x2000*sizeof(uint32_t));
    invalidate_tlb();
    
    my_memcpy((void *) 0x40000000, (void *) (((uint32_t) vita) & ~1), 0x100);

    uint32_t jump_phys = jump_addr + args_final->physbase - args_final->virtbase;
    uint32_t args_phys = ((uint32_t)args_final) + args_final->physbase - args_final->virtbase;

    serial_puthex(jump_phys);
    serial_putstring(" ");
    serial_puthex(args_phys);
    serial_putstring("\n");

#if PUTC
    *((uint32_t *) 0x8000011c) = 0; // for putc
#endif

    ((void (*)(uint32_t, uint32_t)) 0x40000000)(args_phys, jump_phys);

    serial_putstring("it returned?\n");
    return 0;
}

typedef uint32_t user_addr_t;

struct args {
    user_addr_t kern_hdr;
    size_t kern_size;
    user_addr_t devicetree;
    size_t devicetree_size;
};

int ok_go(void *p, struct args *uap, int32_t *retval) {
    kern_hdr = kalloc(uap->kern_size);
    copyin(uap->kern_hdr, kern_hdr, uap->kern_size);
    devicetree = kalloc(uap->devicetree_size);
    devicetree_size = uap->devicetree_size;
    copyin(uap->devicetree, devicetree, uap->devicetree_size);

    *retval = phase_1();
    if(*retval) {
        kfree(kern_hdr, uap->kern_size);
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


