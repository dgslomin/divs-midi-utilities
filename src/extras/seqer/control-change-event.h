#ifndef CONTROL_CHANGE_EVENT_INCLUDED
#define CONTROL_CHANGE_EVENT_INCLUDED

class ControlChangeEventType;
class ControlChangeEventRow;
class ControlChangeEventTimeCell;
class ControlChangeEventTrackCell;
class ControlChangeEventChannelCell;
class ControlChangeEventNumberCell;
class ControlChangeEventValueCell;

#include <wx/wx.h>
#include <midifile.h>
#include "event-type.h"
#include "sequence-editor.h"

class ControlChangeEventType: public EventType
{
public:
	ControlChangeEventType();
	bool MatchesEvent(MidiFileEvent_t event);
	Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class ControlChangeEventRow: public Row
{
public:
	ControlChangeEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class ControlChangeEventTimeCell: public Cell
{
public:
	ControlChangeEventTimeCell(Row* row);
	wxString GetValueText();
};

class ControlChangeEventTrackCell: public Cell
{
public:
	ControlChangeEventTrackCell(Row* row);
	wxString GetValueText();
};

class ControlChangeEventChannelCell: public Cell
{
public:
	ControlChangeEventChannelCell(Row* row);
	wxString GetValueText();
};

class ControlChangeEventNumberCell: public Cell
{
public:
	ControlChangeEventNumberCell(Row* row);
	wxString GetValueText();
};

class ControlChangeEventValueCell: public Cell
{
public:
	ControlChangeEventValueCell(Row* row);
	wxString GetValueText();
};

#endif
