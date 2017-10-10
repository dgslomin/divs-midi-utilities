#ifndef AFTERTOUCH_EVENT_INCLUDED
#define AFTERTOUCH_EVENT_INCLUDED

class AftertouchEventType;
class AftertouchEventRow;
class AftertouchEventTimeCell;
class AftertouchEventTrackCell;
class AftertouchEventChannelCell;
class AftertouchEventNoteCell;
class AftertouchEventAmountCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class AftertouchEventType: public EventType
{
public:
	AftertouchEventType();
	bool MatchesEvent(MidiFileEvent_t event);
	Row* GetRow(MidiFileEvent_t event);
};

class AftertouchEventRow: public Row
{
public:
	AftertouchEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class AftertouchEventTimeCell: public Cell
{
public:
	AftertouchEventTimeCell(Row* row);
	wxString GetValueText();
};

class AftertouchEventTrackCell: public Cell
{
public:
	AftertouchEventTrackCell(Row* row);
	wxString GetValueText();
};

class AftertouchEventChannelCell: public Cell
{
public:
	AftertouchEventChannelCell(Row* row);
	wxString GetValueText();
};

class AftertouchEventNoteCell: public Cell
{
public:
	AftertouchEventNoteCell(Row* row);
	wxString GetValueText();
};

class AftertouchEventAmountCell: public Cell
{
public:
	AftertouchEventAmountCell(Row* row);
	wxString GetValueText();
};

#endif

