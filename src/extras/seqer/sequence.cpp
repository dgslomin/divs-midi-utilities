
#include "midifile.h"
#include "sequence.h"
#include "window.h"

Sequence::Sequence()
{
	this->midi_file = MidiFile_newFromTemplate(NULL);
}

Sequence::~Sequence()
{
	MidiFile_free(this->midi_file);
}

void Sequence::addWindow(Window* window)
{
	this->number_of_windows++;
	connect(this, SIGNAL(updated()), window, SIGNAL(sequenceUpdated()));
}

void Sequence::removeWindow(Window* window)
{
	disconnect(this, SIGNAL(updated()), window, SIGNAL(sequenceUpdated()));
	this->number_of_windows--;
	if (this->number_of_windows == 0) delete this;
}

bool Sequence::save()
{
	if (this->filename.isEmpty()) return false;

	if (MidiFile_save(this->midi_file, this->filename.toUtf8().data()) == 0)
	{
		this->is_modified = false;
		emit updated();
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
		emit updated();
		return true;
	}
	else
	{
		return false;
	}
}

