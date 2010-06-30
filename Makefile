all: 
	make -C misc
	make -C dsc
	make -C goo
	make -C cff

clean:
	make -C misc clean
	make -C cff clean
	make -C cry clean
	make -C dsc clean
	make -C goo clean

distclean:
	make clean
	rm -f config/config.cflags config/config.json config/config*cache
