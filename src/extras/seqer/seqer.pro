
TEMPLATE = app
TARGET = Seqer
INCLUDEPATH += ../../midifile
HEADERS += colors.h controller-lane.h event-type-label-lane.h inspector-sidebar.h label-lane.h lane.h note-lane.h numeric-value-lane.h sequence.h time-ruler.h window.h
SOURCES += colors.cpp controller-lane.cpp event-type-label-lane.cpp inspector-sidebar.cpp label-lane.cpp lane.cpp main.cpp note-lane.cpp numeric-value-lane.cpp sequence.cpp time-ruler.cpp window.cpp ../../midifile/midifile.c
RESOURCES += resources.qrc
QT += widgets

