#ifndef TEMPO_EVENT_INCLUDED
#define TEMPO_EVENT_INCLUDED

class TempoEventType;
class TempoEventTimeCell;
class TempoEventTempoCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class TempoEventType: public EventType
{
public:
	static TempoEventType* GetInstance();

private:
	TempoEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class TempoEventTimeCell: public Cell
{
public:
	TempoEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class TempoEventTempoCell: public Cell
{
public:
	TempoEventTempoCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif
