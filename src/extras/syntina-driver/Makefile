
CC=gcc
CXX=g++
CFLAGS=-O -D__LINUX_ALSA__ -DRTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
LDFLAGS=-O
LIBS=-lasound -lpthread -lstdc++ -li2c -ljansson
MIDIUTILS_SRC=../../../src
NAU7802_SRC=3rdparty/NAU7802/src

syntina-driver: syntina-driver.o hardware.o midi.o utils.o midiutil-common.o midiutil-system.o midiutil-rtmidi.o RtMidi.o rtmidi_c.o NAU7802.o
	$(CC) $(LDFLAGS) -o syntina-driver syntina-driver.o hardware.o midi.o utils.o midiutil-common.o midiutil-system.o midiutil-rtmidi.o RtMidi.o rtmidi_c.o NAU7802.o $(LIBS)

syntina-driver.o: syntina-driver.c
	$(CC) $(CFLAGS) -c syntina-driver.c

hardware.o: hardware.c
	$(CC) $(CFLAGS) -I$(NAU7802_SRC) -c hardware.c

midi.o: midi.c
	$(CC) $(CFLAGS) -I$(MIDIUTILS_SRC)/midiutil -I$(MIDIUTILS_SRC)/3rdparty/rtmidi -c midi.c

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

midiutil-common.o: $(MIDIUTILS_SRC)/midiutil/midiutil-common.c
	$(CC) $(CFLAGS) -I$(MIDIUTILS_SRC)/midiutil -c $(MIDIUTILS_SRC)/midiutil/midiutil-common.c

midiutil-system.o: $(MIDIUTILS_SRC)/midiutil/midiutil-system.c
	$(CC) $(CFLAGS) -I$(MIDIUTILS_SRC)/midiutil -c $(MIDIUTILS_SRC)/midiutil/midiutil-system.c

midiutil-rtmidi.o: $(MIDIUTILS_SRC)/midiutil/midiutil-rtmidi.c
	$(CC) $(CFLAGS) -I$(MIDIUTILS_SRC)/3rdparty/rtmidi -I$(MIDIUTILS_SRC)/midiutil -c $(MIDIUTILS_SRC)/midiutil/midiutil-rtmidi.c

RtMidi.o: $(MIDIUTILS_SRC)/3rdparty/rtmidi/RtMidi.cpp
	$(CXX) $(CFLAGS) -I$(MIDIUTILS_SRC)/3rdparty/rtmidi -c $(MIDIUTILS_SRC)/3rdparty/rtmidi/RtMidi.cpp

rtmidi_c.o: $(MIDIUTILS_SRC)/3rdparty/rtmidi/rtmidi_c.cpp
	$(CXX) $(CFLAGS) -I$(MIDIUTILS_SRC)/3rdparty/rtmidi -c $(MIDIUTILS_SRC)/3rdparty/rtmidi/rtmidi_c.cpp

NAU7802.o: $(NAU7802_SRC)/NAU7802.c
	$(CC) $(CFLAGS) -I$(NAU7802_SRC) -c $(NAU7802_SRC)/NAU7802.c

clean:
	rm -f syntina-driver.o
	rm -f hardware.o
	rm -f midi.o
	rm -f utils.o
	rm -f midiutil-common.o
	rm -f midiutil-system.o
	rm -f midiutil-rtmidi.o
	rm -f RtMidi.o
	rm -f rtmidi_c.o
	rm -f NAU7802.o

reallyclean: clean
	rm -f syntina-driver

