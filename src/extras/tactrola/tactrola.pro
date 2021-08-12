
TEMPLATE = app
TARGET = tactrola
QT += widgets
INCLUDEPATH += ../../midiutil ../../3rdparty/rtmidi
HEADERS += tactrola.h
SOURCES += tactrola.cpp ../../midiutil/midiutil-common.c ../../midiutil/midiutil-rtmidi.c ../../3rdparty/rtmidi/RtMidi.cpp ../../3rdparty/rtmidi/rtmidi_c.cpp
win32:CONFIG(debug, release|debug):CONFIG += console
win32:DEFINES += __WINDOWS_MM__ RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES RTMIDI_DO_NOT_WARN_ABOUT_NO_DEVICES_FOUND
win32:LIBS += winmm.lib
macx:DEFINES += __MACOSX_CORE__
macx:LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
linux:DEFINES += __LINUX_ALSA__ RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
linux:LIBS += -lasound

