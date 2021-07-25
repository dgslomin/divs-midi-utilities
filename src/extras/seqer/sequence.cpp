
#include <QtWidgets>
#include "midifile.h"
#include "sequence.h"
#include "window.h"
#include "undo-stack.h"

Sequence::Sequence(MidiFile_t midi_file)
{
	this->midi_file = midi_file;
	this->undo_stack = new UndoStack(this);
}

Sequence::~Sequence()
{
	delete this->undo_stack;
	MidiFile_free(this->midi_file);
}

void Sequence::addWindow(Window* window)
{
	this->number_of_windows++;
	connect(this, SIGNAL(updated()), window, SLOT(underlyingSequenceUpdated()));
}

void Sequence::removeWindow(Window* window)
{
	disconnect(this, SIGNAL(updated()), window, SLOT(underlyingSequenceUpdated()));
	this->number_of_windows--;
	if (this->number_of_windows == 0) delete this;
}

void Sequence::update(bool create_undo_command)
{
	if (create_undo_command)
	{
		this->is_modified = true;
		this->undo_stack->createUndoCommand();
	}

	emit updated();
}

bool Sequence::save()
{
	bool successful = Sequence::saveMidiFile(this->midi_file, this->filename);

	if (successful)
	{
		this->is_modified = false;
		this->update(false);
	}

	return successful;
}

bool Sequence::saveAs(QString filename)
{
	bool successful = Sequence::saveMidiFile(this->midi_file, filename);

	if (successful)
	{
		this->filename = filename;
		this->is_modified = false;
		this->update(false);
	}

	return successful;
}

MidiFile_t Sequence::loadMidiFile(QString filename)
{
	if (filename.isEmpty()) return NULL;
	MidiFile_t midi_file = MidiFile_load(filename.toUtf8().data());
	if (midi_file != NULL) Sequence::deserializeMidiFile(midi_file);
	return midi_file;
}

MidiFile_t Sequence::loadMidiFileFromBuffer(QByteArray buffer)
{
	MidiFile_t midi_file = MidiFile_loadFromBuffer((unsigned char *)(buffer.data()));
	if (midi_file != NULL) Sequence::deserializeMidiFile(midi_file);
	return midi_file;
}

bool Sequence::saveMidiFile(MidiFile_t midi_file, QString filename)
{
	if (filename.isEmpty()) return false;
	Sequence::serializeMidiFile(midi_file);
	bool successful = (MidiFile_save(midi_file, filename.toUtf8().data()) == 0);
	Sequence::deserializeMidiFile(midi_file);
	return successful;
}

QByteArray Sequence::saveMidiFileToBuffer(MidiFile_t midi_file)
{
	QByteArray buffer;
	Sequence::serializeMidiFile(midi_file);
	buffer.resize(MidiFile_getFileSize(midi_file));
	MidiFile_saveToBuffer(midi_file, (unsigned char *)(buffer.data()));
	Sequence::deserializeMidiFile(midi_file);
	return buffer;
}

void Sequence::serializeMidiFile(MidiFile_t midi_file)
{
	MidiFile_convertNoteEventsToStandardEvents(midi_file);
}

void Sequence::deserializeMidiFile(MidiFile_t midi_file)
{
	MidiFile_convertStandardEventsToNoteEvents(midi_file);
}

void Sequence::midiFileSelectAll(MidiFile_t midi_file)
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		MidiFileEvent_setSelected(midi_event, 1);
	}
}

void Sequence::midiFileSelectNone(MidiFile_t midi_file)
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		MidiFileEvent_setSelected(midi_event, 0);
	}
}

bool Sequence::midiFileHasSelection(MidiFile_t midi_file)
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isSelected(midi_event)) return true;
	}

	return false;
}

bool Sequence::midiFileHasMultipleSelectedTracks(MidiFile_t midi_file)
{
	int number_of_selected_tracks = 0; // excluding the conductor track

	for (MidiFileTrack_t track = MidiFileTrack_getNextTrack(MidiFile_getFirstTrack(midi_file)); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		for (MidiFileEvent_t midi_event = MidiFileTrack_getFirstEvent(track); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInTrack(midi_event))
		{
			if (MidiFileEvent_isSelected(midi_event))
			{
				number_of_selected_tracks++;
				if (number_of_selected_tracks > 1) return true;
				break;
			}
		}
	}

	return false;
}

bool Sequence::midiFileHasMultiplePopulatedTracks(MidiFile_t midi_file)
{
	int number_of_populated_tracks = 0; // excluding the conductor track

	for (MidiFileTrack_t track = MidiFileTrack_getNextTrack(MidiFile_getFirstTrack(midi_file)); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		if (MidiFileTrack_getFirstEvent(track) != NULL) number_of_populated_tracks++;
		if (number_of_populated_tracks > 1) return true;
	}

	return false;
}

