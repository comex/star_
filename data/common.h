#pragma once

//#define PROFILING

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#ifdef PROFILING
#include <time.h>
#endif

const static char *_arg = NULL;
#define die(fmt, args...) do { \
    fprintf(stderr, "%s: ", __func__); \
    if(_arg) fprintf(stderr, "%s: ", _arg); \
    fprintf(stderr, fmt "\n", ##args); \
    abort(); \
} while(0)
#define edie(fmt, args...) die(fmt ": %s", ##args, strerror(errno))

struct binary;
typedef uint32_t addr_t;
typedef struct { const struct binary *binary; addr_t start; size_t size; } range_t;
typedef struct { void *start; size_t size; } prange_t;

void check_range_has_addr(range_t range, addr_t addr);

prange_t pdup(prange_t range);
void write_range(prange_t range, const char *fn, mode_t mode);
