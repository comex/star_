all: out.pdf
out.pdf: out.cff
	python mkpdf.py
out.cff: stage1.txt stage2.txt outcff.py
	python outcff.py || rm -f out.cff
stage1.txt: ../zero.py
	python ../zero.py
stage2.txt: ../zero.py
	python ../zero.py
clean:
	rm -f out.cff stage1.txt stage2.txt *.o *.bin
