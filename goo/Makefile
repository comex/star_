include ../env.mk
all: one.dylib .ran_zero 
.ran_zero: zero.py goo.py world1.py one.dylib ../installui/installui.dylib ../config/config.json goop.bin
	$(PYTHON_IF_NECESSARY) ./zero.py
	touch .ran_zero
one.dylib: one.py zero.py goo.py world1.py ../config/config.json goop.bin
	$(PYTHON_IF_NECESSARY) ./one.py
goop.bin: goop.S
	$(GCC) -mthumb -c -o goop.o goop.S
	../misc/machdump goop.o goop.bin
transe.bin: transe.py goo.py world2.py
	$(PYTHON_IF_NECESSARY) ./transe.py
clean:
	rm -f .ran_zero one.dylib *.pyc
