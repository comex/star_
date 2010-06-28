#include <mach-o/loader.h>
#include <sys/mman.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    assert(argc > 3);
    unsigned int addr = (unsigned int) strtoll(argv[2], NULL, 16);
    unsigned int size = (unsigned int) strtoll(argv[3], NULL, 16);

    int fd = open(argv[1], O_RDONLY);
    assert(fd > 0);
    off_t filesize = lseek(fd, 0, SEEK_END);
    assert(filesize != 0);
    char *input = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(input != MAP_FAILED);
    uint32_t offset = sizeof(struct mach_header);
    int ncmds = ((struct mach_header *) input)->ncmds;
    while(ncmds--) {
        struct load_command *lc = (void *) (input + offset);
        if(lc->cmd == LC_SEGMENT) {
            struct segment_command *sc = (void *) lc;
            if(sc->vmaddr >= addr && addr + size < sc->vmaddr + sc->filesize) {
                write(1, input + sc->fileoff + (addr - sc->vmaddr), size);
                return 0;
            }
        }
        offset += lc->cmdsize;
    }
    fprintf(stderr, "Couldn't find any segments in %s\n", argv[1]);
    return 1;
}
