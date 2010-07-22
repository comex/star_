all: 
	make -C misc essential
	make -C dsc
	make -C install
	make -C installui
	make -C sandbox
	make -C goo
	make -C cff
	make -C mail

clean:
	make -C install clean
	make -C installui clean
	make -C misc clean
	make -C cff clean
	make -C cry clean
	make -C dsc clean
	make -C goo clean
	make -C mail clean
	make -C sandbox clean

distclean:
	make clean
	rm -f config/config.cflags config/config.json config/config*cache
