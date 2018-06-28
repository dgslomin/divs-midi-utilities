
cuppajoe-gpio-to-midi: cuppajoe-gpio-to-midi.o
	gcc -o cuppajoe-gpio-to-midi cuppajoe-gpio-to-midi.o -lasound -lwiringPi

cuppajoe-gpio-to-midi.o: cuppajoe-gpio-to-midi.c
	gcc -c cuppajoe-gpio-to-midi.c

clean:
	rm -f cuppajoe-gpio-to-midi.o

reallyclean: clean
	rm -f cuppajoe-gpio-to-midi

