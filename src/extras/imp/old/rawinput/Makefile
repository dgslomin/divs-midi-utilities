
all: rawinput.exe rawinput2.exe

rawinput.exe: rawinput.obj
	cl /nologo rawinput.obj user32.lib

rawinput.obj: rawinput.c
	cl /nologo /D_WIN32_WINNT=0x0501 /c rawinput.c

rawinput2.exe: rawinput2.obj
	cl /nologo rawinput2.obj user32.lib gdi32.lib

rawinput2.obj: rawinput2.c
	cl /nologo /D_WIN32_WINNT=0x0501 /c rawinput2.c

clean:
	@if exist rawinput.obj del rawinput.obj
	@if exist rawinput2.obj del rawinput2.obj

reallyclean: clean
	@if exist rawinput.exe del rawinput.exe
	@if exist rawinput2.exe del rawinput2.exe

