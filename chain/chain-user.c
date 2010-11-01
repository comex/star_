#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

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
    void *kern_hdr;
    size_t kern_size;
    void *devicetree;
    size_t devicetree_size;


    load("kern", &kern_hdr, &kern_size);
    load("devicetree", &devicetree, &devicetree_size);

    syscall(8, kern_hdr, kern_size, devicetree, devicetree_size);
    return 0;
}
