#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

__attribute__((noinline, noreturn)) void go(void *stack, uint32_t pc) {
    asm("mov sp, %0; mov pc, %1" :: "r"(stack), "r"(pc));
    while(1);
}

int main(int argc, char **argv) {
    int fd = open(argv[1], O_RDONLY);
    assert(fd > 0);
    off_t end = lseek(fd, 0, SEEK_END);
    assert(end > 4);
    lseek(fd, 0, SEEK_SET);
    size_t stack_size = (size_t) (end - 4);
    uint32_t pc;
    assert(read(fd, &pc, sizeof(pc)) == sizeof(pc));
    assert(MAP_FAILED != mmap((void *) 0x10000000, 0x01000000 + stack_size, PROT_WRITE | PROT_READ, MAP_ANON | MAP_FIXED | MAP_SHARED, 0, 0));
    void *stack = (void *) 0x11000000;
    assert((size_t) read(fd, stack, stack_size) == stack_size);
    go(stack, pc);
}
