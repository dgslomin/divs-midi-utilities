#ifndef TEMPO_EVENT_INCLUDED
#define TEMPO_EVENT_INCLUDED

class TempoEventType;
class TempoEventRow;
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
	Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class TempoEventRow: public Row
{
public:
	TempoEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class TempoEventTimeCell: public Cell
{
public:
	TempoEventTimeCell(Row* row);
	wxString GetValueText();
};

class TempoEventTempoCell: public Cell
{
public:
	TempoEventTempoCell(Row* row);
	wxString GetValueText();
};

#endif
