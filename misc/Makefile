include ../env.mk
all: machdump pmap
essential: machdump
machdump: machdump.c
	$(GCC_NATIVE) -o machdump machdump.c
pmap: pmap.c ../config/config.cflags
	$(GCC) -o pmap pmap.c
pmap2: pmap2.c ../config/config.cflags
	$(GCC) -o pmap2 pmap2.c -I../cry/headers -std=gnu99 -framework IOKit -framework CoreFoundation -framework IOSurface

clean:
	rm -f machdump pmap
