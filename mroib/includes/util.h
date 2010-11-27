#ifndef UTIL_H
#define UTIL_H

#include "openiboot.h"
#include <config/config.h>

#ifdef DEBUG
#define DebugPrintf bufferPrintf
#else
#define DebugPrintf(...)
#endif

void *kern_os_malloc(size_t);
void kern_os_free(void *);
void *kern_os_realloc(void *, size_t);
#define malloc kern_os_malloc
#define free kern_os_free
#define realloc kern_os_realloc
extern void *IOMallocAligned(uint32_t, uint32_t);

static void *memset(void *b, int c, size_t len) {
    unsigned char *p = b;
    unsigned char q = c;
    while(len--) *p++ = q;
    return b;
}

static inline void *calloc(size_t count, size_t size) {
    void *result = malloc(count * size);
    memset(result, 0, count * size);
    return result;
}

static inline size_t strlen(const char* str) {
    size_t result = 0;
    while(*str++) result++;
    return result;
}

static inline void *memcpy(void *dest, const void *src, uint32_t size) {
    char *p = dest;
    const char *q = src;
    while(size--) *p++ = *q++;
    return dest;
}

void CleanAndInvalidateCPUDataCache(void *buffer, int bufferLen);

#define EnterCriticalSection()
#define LeaveCriticalSection()

#endif
