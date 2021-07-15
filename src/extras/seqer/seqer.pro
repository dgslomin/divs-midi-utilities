
TEMPLATE = app
TARGET = Seqer
INCLUDEPATH += ../../midifile
HEADERS += colors.h event-type-label-lane.h inspector-sidebar.h label-lane.h lane.h note-lane.h sequence.h time-ruler.h window.h
SOURCES += colors.cpp event-type-label-lane.cpp inspector-sidebar.cpp label-lane.cpp lane.cpp main.cpp note-lane.cpp sequence.cpp time-ruler.cpp window.cpp ../../midifile/midifile.c
RESOURCES += resources.qrc
QT += widgets

