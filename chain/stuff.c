#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chain.h"

#if 1
void uart_set_rate(uint32_t rate) {
}

static void serial_putc(char c) {
    ((void (*)(const char *, ...)) CONFIG_IOLOG)("%c", c);
}
#else
#define fancy_set_rate ((void (*)(uint32_t, uint32_t)) CONFIG_FANCY_SET_RATE)

void uart_set_rate(uint32_t rate) {
    fancy_set_rate(0/*ignored!*/, rate);
}

static void uart_putc(char c) {
    while(0 == (*((volatile uint32_t *) 0xc0000010) & 4));
    *((volatile char *) 0xc0000020) = c;
}

static void serial_putc(char c) {
    uart_putc(c);
    if(c == '\n') {
        // dunno why the kernel does this, but...
        uart_putc('\r');
    }
}
#endif

void serial_putstring(const char *string) {
    char c; while(c = *string++) serial_putc(c);
}

void serial_puthexbuf(void *buf, uint32_t size) {
    uint8_t *p = buf;
    while(size--) {
        uint8_t number = *p++;
        for(int i = 4; i >= 0; i -= 4) {
            uint8_t digit = ((number >> i) & 15);
            if(digit < 10) {
                serial_putc('0' + digit);
            } else {
                serial_putc('a' + (digit - 10));
            }
        }
    }
}

void serial_puthex(uint32_t number) {
    bool going = false;
    for(int i = 28; i >= 0; i -= 4) {
        uint8_t digit = ((number >> i) & 15);
        if(digit) going = true;
        if(!going) continue;
        if(digit < 10) {
            serial_putc('0' + digit);
        } else {
            serial_putc('a' + (digit - 10));
        }
    }
}

int my_strcmp(const char *a, const char *b) {
    while(1) {
        char c = *a++, d = *b++;
        if(c != d) return c - d;
        if(!c) return 0;
    }
}

int my_memcmp(const char *a, const char *b, size_t size) {
    while(size--) {
        char c = *a++, d = *b++;
        if(c != d) return c - d;
    }
    return 0;
}

size_t my_strlen(const char *a) {
    size_t i = 0;
    while(*a++) i++;
    return i;
}
