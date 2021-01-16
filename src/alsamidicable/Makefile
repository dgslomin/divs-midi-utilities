
../../bin/alsamidicable: alsamidicable.o
	gcc -o ../../bin/alsamidicable alsamidicable.o -lasound

alsamidicable.o: alsamidicable.c
	gcc -c alsamidicable.c

clean:
	rm -f alsamidicable.o

reallyclean: clean
	rm -f ../../bin/alsamidicable

