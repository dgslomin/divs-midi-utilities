#ifndef AFTERTOUCH_EVENT_INCLUDED
#define AFTERTOUCH_EVENT_INCLUDED

class AftertouchEventType;
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
	static AftertouchEventType* GetInstance();

private:
	AftertouchEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class AftertouchEventTimeCell: public Cell
{
public:
	AftertouchEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class AftertouchEventTrackCell: public Cell
{
public:
	AftertouchEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class AftertouchEventChannelCell: public Cell
{
public:
	AftertouchEventChannelCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class AftertouchEventNoteCell: public Cell
{
public:
	AftertouchEventNoteCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class AftertouchEventAmountCell: public Cell
{
public:
	AftertouchEventAmountCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif

