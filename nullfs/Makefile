include ../env.mk
all: loader loader.txt nullfs.dylib
loader.txt: nullfs.dylib nm.py
	python nm.py 
loader: loader.c 
	$(GCC) -std=gnu99 -o loader loader.c
	ldid -S loader
nullfs.dylib: null_subr.o null_vfsops.o null_vnops.o 
	$(GCC_ARMV7) -dynamiclib -undefined dynamic_lookup -o $@ $^ -image_base 0xf0000000 -segaddr __TEXT 0xc03c0000
%.o: %.c
	$(GCC_ARMV7) -dynamiclib -c -o $@ $< -DKERNEL -DXNU_KERNEL_PRIVATE -DPRIVATE -DBSD_KERNEL_PRIVATE -DKERNEL_PRIVATE -D__APPLE_API_PRIVATE -Iinc -Ixnu/bsd -Ixnu/osfmk -I.

clean:
	rm -f *.o nullfs.dylib loader loader.txt
