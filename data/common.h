#pragma once

#define PROFILING

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

#define die(fmt, args...) do { fprintf(stderr, fmt, ##args); abort(); } while(0)
#define edie(fmt, args...) die(fmt, ##args, strerror(errno))

extern unsigned char pf2_bin[], one_bin[];
extern unsigned int pf2_bin_len, one_bin_len;

typedef uint32_t addr_t;
typedef struct { addr_t start; size_t size; } range_t;
typedef struct { void *start; size_t size; } prange_t;

