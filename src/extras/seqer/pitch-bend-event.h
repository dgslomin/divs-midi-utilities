#ifndef PITCH_BEND_EVENT_INCLUDED
#define PITCH_BEND_EVENT_INCLUDED

class PitchBendEventType;
class PitchBendEventTimeCell;
class PitchBendEventTrackCell;
class PitchBendEventChannelCell;
class PitchBendEventValueCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class PitchBendEventType: public EventType
{
public:
	static PitchBendEventType* GetInstance();

private:
	PitchBendEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class PitchBendEventTimeCell: public Cell
{
public:
	PitchBendEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class PitchBendEventTrackCell: public Cell
{
public:
	PitchBendEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class PitchBendEventChannelCell: public Cell
{
public:
	PitchBendEventChannelCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class PitchBendEventValueCell: public Cell
{
public:
	PitchBendEventValueCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif
