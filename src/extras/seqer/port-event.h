#ifndef PORT_EVENT_INCLUDED
#define PORT_EVENT_INCLUDED

class PortEventType;
class PortEventRow;
class PortEventTimeCell;
class PortEventTrackCell;
class PortEventNameCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class PortEventType: public EventType
{
public:
	static PortEventType* GetInstance();

private:
	PortEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
	Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class PortEventRow: public Row
{
public:
	PortEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class PortEventTimeCell: public Cell
{
public:
	PortEventTimeCell(Row* row);
	wxString GetValueText();
};

class PortEventTrackCell: public Cell
{
public:
	PortEventTrackCell(Row* row);
	wxString GetValueText();
};

class PortEventNameCell: public Cell
{
public:
	PortEventNameCell(Row* row);
	wxString GetValueText();
};

#endif
