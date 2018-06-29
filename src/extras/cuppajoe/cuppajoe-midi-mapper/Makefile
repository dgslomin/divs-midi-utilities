
cuppajoe-midi-mapper: cuppajoe-midi-mapper.o
	gcc -o cuppajoe-midi-mapper cuppajoe-midi-mapper.o -lm -lasound

cuppajoe-midi-mapper.o: cuppajoe-midi-mapper.c
	gcc -c cuppajoe-midi-mapper.c

clean:
	rm -f cuppajoe-midi-mapper.o

reallyclean: clean
	rm -f cuppajoe-midi-mapper

