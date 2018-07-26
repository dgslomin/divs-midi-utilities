#ifndef TIME_SIGNATURE_EVENT_INCLUDED
#define TIME_SIGNATURE_EVENT_INCLUDED

class TimeSignatureEventType;
class TimeSignatureEventTimeCell;
class TimeSignatureEventTimeSignatureCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class TimeSignatureEventType: public EventType
{
public:
	static TimeSignatureEventType* GetInstance();

private:
	TimeSignatureEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class TimeSignatureEventTimeCell: public Cell
{
public:
	TimeSignatureEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class TimeSignatureEventTimeSignatureCell: public Cell
{
public:
	TimeSignatureEventTimeSignatureCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif
