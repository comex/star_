include ../env.mk
all: machdump kcode.bin bootstrap.bin
%: %.c
	gcc -o $@ $<

%.bin: %.S machdump Makefile
	perl gas-preprocessor.pl $(GCC)  -c -o $(@:.bin=.o) $< -nostdlib -nodefaultlibs -nostartfiles
	ld -o $(@:.bin=.e) $(@:.bin=.o) 
	./machdump $(@:.bin=.e) $@
	rm -f $(@:.bin=.o)

clean:
	rm -f *.e *.bin *.o machdump
