all: stage1.txt one.dylib
# also stage2.txt
.ran_zero: zero.py ../config/config.json
	./zero.py
	touch .ran_zero
one.dylib: one.py zero.py ../config/config.json
	./one.py
clean:
	rm -f stage1.txt stage2.txt one.dylib *.pyc
