# install.c is in the download.  installui.m is bundled.
include ../env.mk
all: install.dylib wad.bin
install.dylib: install.o copier.o lsstuff.o 
	$(GCC_UNIVERSAL) -dynamiclib -o $@ $^ -framework Foundation -framework CoreFoundation libtar.a liblzma.a
%.o: %.[cm] common.h 
	$(GCC_UNIVERSAL) -Iheaders -std=gnu99 -c -o $@ $< -Werror
clean:
	rm -f install.dylib *.o
wad.bin: install.dylib freeze.tar.xz
	python wad.py
	
