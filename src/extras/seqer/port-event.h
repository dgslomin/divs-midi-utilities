#ifndef PORT_EVENT_INCLUDED
#define PORT_EVENT_INCLUDED

class PortEventType;
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
};

class PortEventTimeCell: public Cell
{
public:
	PortEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class PortEventTrackCell: public Cell
{
public:
	PortEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class PortEventNameCell: public Cell
{
public:
	PortEventNameCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif
