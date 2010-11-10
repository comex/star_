#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chain.h"

int _NSConcreteGlobalBlock;
int _NSConcreteStackBlock;

#if !USE_ASM_FUNCS
void *my_memcpy(void *dest, const void *src, size_t n) {
    char *a = dest;
    const char *b = src;
    while(n--) {
        *a++ = *b++;
    }
    return dest;
}

void *my_memset(void *dest, int c, size_t len) {
    char *a = dest;
    while(len--) {
        *a++ = (char) c;
    }
    return dest;
}
#endif

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

#if DEBUG

bool serial_important = false;

#if !HAVE_SERIAL // USB

static char *usb_base = (char *) 0xd3edc000; // -> 86100000
static char *gpio_base = (char *) 0xc9d9d000; // -> bfa00000
static char *ringbuf_start = (char *) 0x807d6000;
static char *ringbuf = NULL;
static size_t ringbuf_size = 0x3000;

void uart_set_rate(uint32_t rate) {
}

static void poke_wdt() {
    volatile uint32_t *wdt = (void *) (gpio_base + 0x4c);
    *wdt = (*wdt == 0x212) ? 0x213 : 0x212;
}

static void serial_putbuf(const char *c, size_t size) {
    // This is silly: the ring buffer is just because I don't understand how to use this properly
    if(!ringbuf || (ringbuf + size) > (ringbuf_start + ringbuf_size)) {
        ringbuf = ringbuf_start;
    }
    my_memcpy(ringbuf, c, size);
    for(uintptr_t p = (uintptr_t) ringbuf; p < ((uintptr_t) ringbuf) + size; p += 0x10) {
        asm volatile("mcr p15, 0, %0, c7, c14, 1" :: "r"(p));
    }
    poke_wdt();
    while(*((volatile uint32_t *) (usb_base + 3*0x20 + 0x900)) & 0x80000000);
    *((volatile uint32_t *) (usb_base + 3*0x20 + 0x914)) = ((uint32_t)ringbuf) - 0x40000000;
    *((volatile uint32_t *) (usb_base + 3*0x20 + 0x910)) = (((size + 63) / 64) << 19) | size;
    *((volatile uint32_t *) (usb_base + 3*0x20 + 0x900)) |= 0x84000000;
    ringbuf += 64;
}

static void serial_putc(char c) {
    serial_putbuf(&c, 1);
}

#else

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

static void serial_putbuf(const char *c, size_t size) {
    while(size--) serial_putc(*c++);
}

#endif

void serial_putstring(const char *string) {
    if(!DEBUG_VERBOSE && !serial_important) return;
    serial_putbuf(string, my_strlen(string));
}

void serial_puthexbuf(void *buf, uint32_t size) {
    if(!DEBUG_VERBOSE && !serial_important) return;
    uint8_t *p = buf;
    char digits[64];
    while(size > 0) {
        uint32_t lsize = size;
        if(lsize > 32) lsize = 32; // this had better be ok, 126 worked
        size -= lsize;
        char *digitsp = digits;
        while(lsize--) {
            uint8_t number = *p++;
            for(int i = 4; i >= 0; i -= 4) {
                uint8_t digit = ((number >> i) & 15);
                if(digit < 10) {
                    *digitsp++ = '0' + digit;
                } else {
                    *digitsp++ = 'a' + (digit - 10);
                }
            }
        }
        serial_putbuf(digits, digitsp - digits);
    }
}

void serial_puthex(uint32_t number) {
    if(!DEBUG_VERBOSE && !serial_important) return;
    char digits[16];
    char *digitsp = digits;
    bool going = false;
    for(int i = 28; i >= 0; i -= 4) {
        uint8_t digit = ((number >> i) & 15);
        if(digit) going = true;
        if(!going) continue;
        if(digit < 10) {
            *digitsp++ = '0' + digit;
        } else {
            *digitsp++ = 'a' + (digit - 10);
        }
    }
    serial_putbuf(digits, digitsp - digits);
    
}
#endif
