ROOT := $(realpath $(shell dirname $(lastword $(MAKEFILE_LIST))))
BIN := /Developer/Platforms/iPhoneOS.platform/Developer/usr/bin
GCC := $(BIN)/gcc-4.2 -arch armv6 -Os --sysroot /var/sdk `cat $(ROOT)/config.cflags` -Wimplicit -isysroot /var/sdk -F/var/sdk/System/Library/Frameworks -F/var/sdk/System/Library/PrivateFrameworks
GCC_UNIVERSAL := $(BIN)/gcc-4.2 -arch armv7 -arch armv6 -Os -Ixz `cat $(ROOT)/config.cflags` -Wimplicit -isysroot /var/sdk


