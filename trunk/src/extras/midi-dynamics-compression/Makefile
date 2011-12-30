
all: t1.exe t2.exe t3.exe t4.exe

t1.exe: t1.obj
	cl /nologo /Fet1.exe t1.obj winmm.lib

t2.exe: t2.obj midifile.obj
	cl /nologo /Fet2.exe t2.obj midifile.obj

t3.exe: t3.obj midifile.obj
	cl /nologo /Fet3.exe t3.obj midifile.obj

t4.exe: t4.obj midifile.obj
	cl /nologo /Fet4.exe t4.obj midifile.obj

t1.obj: t1.c
	cl /nologo /c t1.c

t2.obj: t2.c ..\..\midifile\midifile.h
	cl /nologo /I..\..\midifile /c t2.c

t3.obj: t3.c ..\..\midifile\midifile.h
	cl /nologo /I..\..\midifile /c t3.c

t4.obj: t4.c ..\..\midifile\midifile.h
	cl /nologo /I..\..\midifile /c t4.c

midifile.obj: ..\..\midifile\midifile.c ..\..\midifile\midifile.h
	cl /nologo /I..\..\midifile /c ..\..\midifile\midifile.c

clean:
	@if exist t1.obj del t1.obj
	@if exist t2.obj del t2.obj
	@if exist t3.obj del t3.obj
	@if exist t4.obj del t4.obj
	@if exist midifile.obj del midifile.obj

reallyclean: clean
	@if exist t1.exe del t1.exe
	@if exist t2.exe del t2.exe
	@if exist t3.exe del t3.exe
	@if exist t4.exe del t4.exe

