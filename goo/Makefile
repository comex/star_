include ../env.mk
all: one.dylib .ran stage1.txt 
# also stage2.txt
.ran_zero: zero.py ../config/config.json
	$(PYTHON_IF_NECESSARY) ./zero.py
	touch .ran_zero
one.dylib: one.py zero.py ../config/config.json
	$(PYTHON_IF_NECESSARY) ./one.py
clean:
	rm -f stage1.txt stage2.txt one.dylib *.pyc
