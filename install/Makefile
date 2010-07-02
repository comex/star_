# install.c is in the download.  installui.m is bundled.
include ../env.mk
installui.dylib: installui.m
	$(GCC) -dynamiclib -Iheaders -std=gnu99 -o installui.dylib installui.m -framework Foundation -framework UIKit -framework IOKit 
