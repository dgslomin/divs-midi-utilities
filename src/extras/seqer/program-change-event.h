#ifndef PROGRAM_CHANGE_EVENT_INCLUDED
#define PROGRAM_CHANGE_EVENT_INCLUDED

class ProgramChangeEventType;
class ProgramChangeEventTimeCell;
class ProgramChangeEventTrackCell;
class ProgramChangeEventChannelCell;
class ProgramChangeEventNumberCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class ProgramChangeEventType: public EventType
{
public:
	static ProgramChangeEventType* GetInstance();

private:
	ProgramChangeEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class ProgramChangeEventTimeCell: public Cell
{
public:
	ProgramChangeEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class ProgramChangeEventTrackCell: public Cell
{
public:
	ProgramChangeEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class ProgramChangeEventChannelCell: public Cell
{
public:
	ProgramChangeEventChannelCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class ProgramChangeEventNumberCell: public Cell
{
public:
	ProgramChangeEventNumberCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif

