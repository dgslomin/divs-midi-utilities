#ifndef SEQUENCE_INCLUDED
#define SEQUENCE_INCLUDED

class Sequence;

#include <QObject>
#include <QString>
#include "midifile.h"
#include "window.h"

class Sequence: QObject
{
Q_OBJECT

public:
	int number_of_windows = 0;
	QString filename = "";
	MidiFile_t midi_file = NULL;
	bool is_modified = false;

	Sequence();
	~Sequence();
	void addWindow(Window* window);
	void removeWindow(Window* window);
	bool save();
	bool saveAs(QString filename);

signals:
	void updated();
};

#endif
