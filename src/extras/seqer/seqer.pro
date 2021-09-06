
TEMPLATE = app
TARGET = Seqer
QT += widgets
INCLUDEPATH += ../../midifile
HEADERS += all-events-lane.h colors.h continuous-value-lane.h controller-lane.h inspector-sidebar.h label-lane.h lane.h marker-lane.h menu.h note-lane.h separator.h sequence.h splitter.h tempo-lane.h time-ruler.h undo-stack.h velocity-lane.h window.h
SOURCES += all-events-lane.cpp colors.cpp continuous-value-lane.cpp controller-lane.cpp inspector-sidebar.cpp label-lane.cpp lane.cpp main.cpp marker-lane.cpp menu.cpp note-lane.cpp separator.cpp sequence.cpp splitter.cpp tempo-lane.cpp time-ruler.cpp undo-stack.cpp velocity-lane.cpp window.cpp ../../midifile/midifile.c
win32:CONFIG(debug, release|debug):CONFIG += console
win32:RC_ICONS += seqer.ico
macx:ICON = seqer.icns

