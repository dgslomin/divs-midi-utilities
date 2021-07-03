#ifndef SEQUENCE_INCLUDED
#define SEQUENCE_INCLUDED

class Sequence;

#include <list>
#include <QString>
#include "midifile.h"
#include "window.h"

class Sequence
{
public:
	std::list<Window*> windows;
	QString filename;
	MidiFile_t midi_file;
	bool is_modified;

	Sequence();
	~Sequence();
	void addWindow(Window* window);
	void removeWindow(Window* window);
	void refreshData();
	void refreshDisplay();
	bool save();
	bool saveAs(QString filename);
};

#endif
