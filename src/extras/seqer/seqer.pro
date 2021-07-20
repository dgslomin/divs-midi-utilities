
TEMPLATE = app
TARGET = Seqer
INCLUDEPATH += ../../midifile
HEADERS += all-events-lane.h colors.h controller-lane.h inspector-sidebar.h label-lane.h lane.h note-lane.h numeric-value-lane.h sequence.h time-ruler.h window.h
SOURCES += all-events-lane.cpp colors.cpp controller-lane.cpp inspector-sidebar.cpp label-lane.cpp lane.cpp main.cpp note-lane.cpp numeric-value-lane.cpp sequence.cpp time-ruler.cpp window.cpp ../../midifile/midifile.c
RESOURCES += resources.qrc
QT += widgets

