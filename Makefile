all: 
	make machdump
	make -C dsc
	make -C kern
	python zero.py
	make -C cff

machdump: machdump.c
	gcc -o $@ $<

clean:
	rm -f machdump
	make -C cff clean
	make -C cry clean
	make -C dsc clean
	make -C kern clean

distclean:
	make clean
	rm -f config/config.cflags config/config.json config/config*cache
