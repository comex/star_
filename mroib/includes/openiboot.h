#ifndef OPENIBOOT_H
#define OPENIBOOT_H

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed long long int64_t;
typedef signed int int32_t;
typedef signed short int16_t;
typedef signed char int8_t;
typedef long unsigned int size_t;
typedef signed int intptr_t;

typedef enum Boolean {
	FALSE = 0,
	TRUE = 1
} Boolean;

typedef enum OnOff {
	OFF = 0,
	ON = 1
} OnOff;

#ifndef NULL
#define NULL 0
#endif
#define uSecPerSec 1000000

/*
 *	Macros
 */

#define GET_REG(x) (*((volatile uint32_t*)(x)))
#define SET_REG(x, y) (*((volatile uint32_t*)(x)) = (y))
#define GET_REG32(x) GET_REG(x)
#define SET_REG32(x, y) SET_REG(x, y)
#define GET_REG16(x) (*((volatile uint16_t*)(x)))
#define SET_REG16(x, y) (*((volatile uint16_t*)(x)) = (y))
#define GET_REG8(x) (*((volatile uint8_t*)(x)))
#define SET_REG8(x, y) (*((volatile uint8_t*)(x)) = (y))
#define GET_BITS(x, start, length) ((((uint32_t)(x)) << (32 - ((start) + (length)))) >> (32 - (length)))

#endif
