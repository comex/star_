all: out.cff machdump 

stage1.txt: zero.py
	python zero.py
stage2.txt: zero.py
	python zero.py
out.cff: stage1.txt stage2.txt outcff.py
	python outcff.py || rm -f out.cff
clean:
	rm -f out.cff stage1.txt stage2.txt *.o *.bin
