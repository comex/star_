include ../env.mk
all: one.dylib .ran_zero 
.ran_zero: zero.py one.dylib ../install/installui.dylib ../config/config.json
	$(PYTHON_IF_NECESSARY) ./zero.py
	touch .ran_zero
one.dylib: one.py zero.py ../config/config.json
	$(PYTHON_IF_NECESSARY) ./one.py
clean:
	rm -f .ran_zero one.dylib *.pyc
