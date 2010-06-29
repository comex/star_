include ../env.mk
all: machdump pmap
pmap: pmap.c ../config/config.cflags
	$(GCC) -o pmap pmap.c

clean:
	rm -f machdump pmap
