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
	QString filename = "";
	MidiFile_t midi_file = NULL;
	bool is_modified = false;

	Sequence();
	~Sequence();
	void addWindow(Window* window);
	void removeWindow(Window* window);
	void updateWindows();
	bool save();
	bool saveAs(QString filename);
};

#endif
