cdef extern from "sys/mman.h":
    void *mmap(void *start, size_t length, int prot, int flags, int fd, long long offset)
    int munmap(void *start, size_t len)
    cdef int PROT_READ
    cdef int MAP_SHARED
    cdef void *MAP_FAILED
cdef extern from "stdlib.h":
    void *calloc(size_t count, size_t size)
import os

def search_for_things(filename, patterns):
    cdef int fd = os.open(filename, 0)
    cdef size_t size = os.lseek(fd, 0, 2)
    os.lseek(fd, 0, 0)
    cdef char *stuff = <char *> mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0)
    if stuff == MAP_FAILED:
        raise Exception('Could not mmap')

    cdef int num_patterns = len(patterns)
    cdef signed char *progress = <signed char *> calloc(1, num_patterns)
    cdef char **pattern_bufs = <char **> calloc(num_patterns, sizeof(char *))
    results = {}
    for i in range(num_patterns):
        pattern_bufs[i] = patterns[i]
        results[i] = None
    cdef int pos = 0
    cdef char c
    cdef int n
    cdef char *pattern
    while pos < size:
        c = stuff[pos]   
        for i in range(num_patterns):
            n = progress[i]
            if n == -1: continue
            pattern = pattern_bufs[i]
            if pattern[n] == c:
                n += 1
                if pattern[n] == 0:
                    results[i] = pos - n + 1
                    progress[i] = -1
                else:
                    progress[i] = n
            else:
                progress[i] = 0
                    
        pos += 1

    munmap(stuff, size)
    os.close(fd)
    return results
