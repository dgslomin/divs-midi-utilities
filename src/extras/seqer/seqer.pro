
TEMPLATE = app
TARGET = Seqer
INCLUDEPATH += ../../midifile
HEADERS += colors.h inspector-sidebar.h lane.h note-lane.h sequence.h time-ruler.h window.h
SOURCES += colors.cpp inspector-sidebar.cpp lane.cpp main.cpp note-lane.cpp sequence.cpp time-ruler.cpp window.cpp ../../midifile/midifile.c
RESOURCES += resources.qrc
QT += widgets

