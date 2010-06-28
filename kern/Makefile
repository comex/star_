include ../env.mk
all: ../machdump machdumprange kcode.bin bootstrap.bin

%.bin: %.S ../machdump Makefile ../config/config.cflags
	perl gas-preprocessor.pl $(GCC) -c -o $(@:.bin=.o) $< -nostdlib -nodefaultlibs -nostartfiles
	ld -o $(@:.bin=.e) $(@:.bin=.o) 
	../machdump $(@:.bin=.e) $@
	rm -f $(@:.bin=.o)

#kcode.bin: kcode.S ../machdump Makefile ../config/config.cflags machdumprange
#	perl gas-preprocessor.pl $(GCC) -c -o $(@:.bin=.o) $< -nostdlib -nodefaultlibs -nostartfiles
#	ld -o $(@:.bin=.e) $(@:.bin=.o) 
#	../machdump $(@:.bin=.e) $@
#	./machdumprange $@ $(shell echo CONFIG_BCOPYTARGET | cpp -P $(CFLAGS) | tail -n 1) 60
#	rm -f $(@:.bin=.o)

clean:
	rm -f *.e *.bin *.o 
