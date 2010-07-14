include ../env.mk
all: machdump pmap
machdump: machdump.c
	$(GCC_NATIVE) -o machdump machdump.c
pmap: pmap.c ../config/config.cflags
	$(GCC) -o pmap pmap.c

clean:
	rm -f machdump pmap
