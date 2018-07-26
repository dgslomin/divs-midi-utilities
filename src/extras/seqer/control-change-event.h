#ifndef CONTROL_CHANGE_EVENT_INCLUDED
#define CONTROL_CHANGE_EVENT_INCLUDED

class ControlChangeEventType;
class ControlChangeEventTimeCell;
class ControlChangeEventTrackCell;
class ControlChangeEventChannelCell;
class ControlChangeEventNumberCell;
class ControlChangeEventValueCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class ControlChangeEventType: public EventType
{
public:
	static ControlChangeEventType* GetInstance();

private:
	ControlChangeEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class ControlChangeEventTimeCell: public Cell
{
public:
	ControlChangeEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class ControlChangeEventTrackCell: public Cell
{
public:
	ControlChangeEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class ControlChangeEventChannelCell: public Cell
{
public:
	ControlChangeEventChannelCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class ControlChangeEventNumberCell: public Cell
{
public:
	ControlChangeEventNumberCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class ControlChangeEventValueCell: public Cell
{
public:
	ControlChangeEventValueCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif
