# install.c is in the download.  installui.m is bundled.
include ../env.mk
all: installui.dylib install.dylib
installui.dylib: installui.m
	$(GCC) -dynamiclib -Iheaders -std=gnu99 -o installui.dylib installui.m -framework Foundation -framework UIKit -framework IOKit  -Werror
install.dylib: install.o copier.o
	$(GCC) -dynamiclib -o $@ $^ -framework CoreFoundation -L. -ltar -llzma
%.o: %.c
	$(GCC) -Iheaders -std=gnu99 -c -o $@ $< -Werror
clean:
	rm -f installui.dylib
