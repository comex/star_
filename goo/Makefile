all: stage1.txt one.dylib
# also stage2.txt
stage1.txt: zero.py ../config/config.json
	python zero.py
one.dylib: one.py zero.py ../config/config.json
	python one.py
clean:
	rm -f stage1.txt stage2.txt one.dylib *.pyc
