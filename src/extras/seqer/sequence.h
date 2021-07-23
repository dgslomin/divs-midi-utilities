#ifndef SEQUENCE_INCLUDED
#define SEQUENCE_INCLUDED

class Sequence;

#include <QtWidgets>
#include "midifile.h"
#include "undo-stack.h"
#include "window.h"

class Sequence: public QObject
{
	Q_OBJECT

public:
	int number_of_windows = 0;
	QString filename = "";
	bool is_modified = false;
	MidiFile_t midi_file;
	UndoStack* undo_stack;

	Sequence();
	~Sequence();
	void addWindow(Window* window);
	void removeWindow(Window* window);
	bool save();
	bool saveAs(QString filename);
	static MidiFile_t loadMidiFile(QString filename);
	static MidiFile_t loadMidiFileFromBuffer(QByteArray buffer);
	static bool saveMidiFile(MidiFile_t midi_file, QString filename);
	static QByteArray saveMidiFileToBuffer(MidiFile_t midi_file);
	static void serializeMidiFile(MidiFile_t midi_file);
	static void deserializeMidiFile(MidiFile_t midi_file);
	static void midiFileSelectAll(MidiFile_t midi_file);
	static void midiFileSelectNone(MidiFile_t midi_file);
	static bool midiFileHasSelection(MidiFile_t midi_file);
	static bool midiFileHasMultipleSelectedTracks(MidiFile_t midi_file);
	static bool midiFileHasMultiplePopulatedTracks(MidiFile_t midi_file);

signals:
	void updated(bool create_undo_command);
};

#endif
