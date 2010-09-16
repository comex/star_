include ../env.mk
all: machdump pmap shelltester
%: %.c
	$(GCC_WITH_IOKIT) -o $@ $<
