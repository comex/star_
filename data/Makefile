ARM_EABI_PATH := /Users/comex/arm-none-eabi/bin
all: data
one.c: one.bin
	xxd -i one.bin > one.c
exploiter.c: exploiter.bin
	xxd -i exploiter.bin > exploiter.c
data: data.c exploiter.c one.c
	gcc -O3 -std=gnu99 -g -o $@ data.c exploiter.c one.c -Winline -Wimplicit -Werror
clean:
	rm -rf exploiter.bin exploiter.c data *.dSYM
