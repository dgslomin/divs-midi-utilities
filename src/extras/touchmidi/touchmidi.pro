
TEMPLATE = app
TARGET = touchmidi
QT += widgets
INCLUDEPATH += ../../midiutil ../../3rdparty/rtmidi
HEADERS += touchmidi.h
SOURCES += touchmidi.cpp ../../midiutil/midiutil-rtmidi.c ../../3rdparty/rtmidi/RtMidi.cpp ../../3rdparty/rtmidi/rtmidi_c.cpp
win32:debug { CONFIG += console }

