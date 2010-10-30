#include <stdint.h>
#include <stdbool.h>
#include <config/config.h>
#define LT __attribute__((section("__LOCKTEXT,__locktext")))
#define LD __attribute__((section("__LOCKDATA,__lockdata")))

#define fancy_set_rate ((void (*)(uint32_t, uint32_t)) CONFIG_FANCY_SET_RATE)

LT void uart_set_rate(uint32_t rate) {
    fancy_set_rate(0/*ignored!*/, rate);
}

LT static void uart_putc(char c) {
    while(0 == (*((volatile uint32_t *) 0xc0000010) & 4));
    *((volatile char *) 0xc0000020) = c;
}

LT static void serial_putc(char c) {
    uart_putc(c);
    if(c == '\n') {
        // dunno why the kernel does this, but...
        uart_putc('\r');
    }
}

LT void serial_putstring(const char *string) {
    char c; while(c = *string++) serial_putc(c);
}

LT void serial_puthex(uint32_t number) {
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

LT int my_strcmp(const char *a, const char *b) {
    while(1) {
        char c = *a++, d = *b++;
        if(c != d) return c - d;
        if(!c) return 0;
    }
}
