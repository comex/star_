#pragma once
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <syslog.h>
#define ctassert(x, y) extern char XX_## y [(x) ? 1 : -1]
#ifndef _log
#define _log(args...) syslog(LOG_EMERG, args)
#endif

#define _assert(expr, arg...) ((expr) ?: (_assert_helper(#expr, arg + 0), (typeof(expr)) 0))
#define _assert_zero(expr, arg...) do { typeof(expr) _value = (expr); \
                                        if(_value) _assert_zero_helper(#expr, arg + 0, (unsigned int) _value); \
                                      } while(0)
#ifdef NO_ASSERT_MESSAGES
#define _assert_helper(...) abort()
#define _assert_zero_helper(...) abort()
#else

__attribute__((noreturn))
static void _assert_helper(const char name[], const char *arg) {
    _log("assertion failed: %s%s%s%s (errno=%s)\n", name, arg ? "[" : "", arg ? arg : "", arg ? "]" : "", strerror(errno));
    exit(1);
}

__attribute__((noreturn))
static void _assert_zero_helper(const char name[], const char *arg, unsigned int value) {
    _log("assertion failed: %s%s%s%s (value=0x%x, errno=%s)\n", name, arg ? "[" : "", arg ? arg : "", arg ? "]" : "", value, strerror(errno));
    exit(1);
}

#endif

#ifdef PROFILING
#define TIME(thing) do { uint64_t _ta = time_ms(); thing; uint64_t _tb = time_ms(); _logI("[%.4ld ms] %s", (long int) (_tb - _ta), #thing); } while(0)
#else
#define TIME(thing) thing
#endif

static inline uint64_t time_ms() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((uint64_t) tp.tv_sec) * 1000000 + (uint64_t) tp.tv_usec;
}

static void hex_dump(void *data, int size)
{
    // (not my code)
    /* dumps size bytes of *data to stdout. Looks like:
     * [0000] 75 6E 6B 6E 6F 77 6E 20
     *                  30 FF 00 00 00 00 39 00 unknown 0.....9.
     * (in a single line of course)
     */

    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }
            
        c = *p;
        if (isalnum(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) { 
            /* line completed */
            printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}

