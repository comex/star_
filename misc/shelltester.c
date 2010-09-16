#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
int main(int argc, char **argv) {
    int fd = open(argv[1], O_RDONLY);
    assert(fd > 0);
    off_t end = lseek(fd, 0, SEEK_END);
    assert(MAP_FAILED != mmap((void *) 0x10000000, (size_t) end, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0));
    asm("mov sp, %0; pop {pc}" ::"r"(0x10000000));
    while(1);
}
