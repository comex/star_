#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>

typedef mach_port_t io_registry_entry_t, io_service_t;

extern
const mach_port_t kIOMasterPortDefault;

CFMutableDictionaryRef
IOServiceMatching(
    const char *    name );

io_service_t
IOServiceGetMatchingService(
    mach_port_t masterPort,
    CFDictionaryRef matching );

kern_return_t
IORegistryEntrySetCFProperty(
    io_registry_entry_t entry,
        CFStringRef     propertyName,
    CFTypeRef       property );

kern_return_t
IORegistryEntrySetCFProperties(
    io_registry_entry_t entry,
    CFTypeRef       properties );

static void load(const char *fn, void **base, size_t *size) {
    int fd = open(fn, O_RDONLY);
    assert(fd != -1);
    off_t len = lseek(fd, 0, SEEK_END);
    void *addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(addr != MAP_FAILED);
    *base = addr;
    *size = (size_t) len;
}

int main() {
    io_service_t service = IOServiceGetMatchingService( kIOMasterPortDefault, IOServiceMatching("IOWatchDogTimer"));
    assert(service);
    //assert(!IORegistryEntrySetCFProperty(service, CFSTR("IOWatchDogEnabled"), kCFBooleanFalse));
    long zero = 0;
    CFNumberRef number = CFNumberCreate(NULL, kCFNumberLongType, &zero);

    assert(!IORegistryEntrySetCFProperties(service, number));

    void *kern_hdr;
    size_t kern_size;
    void *devicetree;
    size_t devicetree_size;


    load("kern", &kern_hdr, &kern_size);
    load("devicetree", &devicetree, &devicetree_size);

    syscall(8, kern_hdr, kern_size, devicetree, devicetree_size);
    return 0;
}
