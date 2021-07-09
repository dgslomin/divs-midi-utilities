
#include "midifile.h"
#include "sequence.h"
#include "window.h"

Sequence::Sequence()
{
	this->filename = "";
	this->midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	this->is_modified = false;
}

Sequence::~Sequence()
{
	MidiFile_free(this->midi_file);
}

void Sequence::addWindow(Window* window)
{
	this->windows.push_back(window);
}

void Sequence::removeWindow(Window* window)
{
	this->windows.remove(window);
	if (this->windows.empty()) delete this;
}

void Sequence::updateWindows()
{
	for (std::list<Window*>::iterator window_iterator = this->windows.begin(); window_iterator != this->windows.end(); window_iterator++) (*window_iterator)->update();
}

bool Sequence::save()
{
	if (this->filename.isEmpty()) return false;

	if (MidiFile_save(this->midi_file, this->filename.toUtf8().data()) == 0)
	{
		this->is_modified = false;
		this->updateWindows();
		return true;
	}
	else
	{
		return false;
	}
}

bool Sequence::saveAs(QString filename)
{
	if (MidiFile_save(this->midi_file, filename.toUtf8().data()) == 0)
	{
		this->filename = filename;
		this->is_modified = false;
		this->updateWindows();
		return true;
	}
	else
	{
		return false;
	}
}

