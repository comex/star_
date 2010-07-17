# install.c is in the download.  installui.m is bundled.
include ../env.mk
all: installui.dylib install.dylib wad.bin
installui.dylib: installui.o dddata.o ../config/config.cflags common.h
	$(GCC_UNIVERSAL) -dynamiclib -Iheaders -std=gnu99 -o installui.dylib installui.o dddata.o -framework Foundation -framework UIKit -framework IOKit -framework CoreGraphics -lz -Werror
install.dylib: install.o copier.o lsstuff.o 
	$(GCC_UNIVERSAL) -dynamiclib -o $@ $^ -framework Foundation -framework CoreFoundation libtar.a liblzma.a
%.o: %.[cm] common.h
	$(GCC_UNIVERSAL) -Iheaders -std=gnu99 -c -o $@ $< -Werror
clean:
	rm -f installui.dylib *.o
wad.bin: install.dylib freeze.tar.xz
	python wad.py
	
