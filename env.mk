ROOT := $(realpath $(shell dirname $(lastword $(MAKEFILE_LIST))))
BIN := /Developer/Platforms/iPhoneOS.platform/Developer/usr/bin
CFLAGS := `cat $(ROOT)/config/config.cflags` 
GCC_BASE := $(BIN)/gcc-4.2 -Os --sysroot /var/sdk $(CFLAGS) -Wimplicit -isysroot /var/sdk -F/var/sdk/System/Library/Frameworks -F/var/sdk/System/Library/PrivateFrameworks
GCC := $(GCC_BASE) -arch armv6
GCC_ARMV7 := $(GCC_BASE) -arch armv7
GCC_UNIVERSAL := $(GCC_BASE) -arch armv6 -arch armv7


