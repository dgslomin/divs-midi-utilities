#ifndef MARKER_EVENT_INCLUDED
#define MARKER_EVENT_INCLUDED

class MarkerEventType;
class MarkerEventTimeCell;
class MarkerEventNameCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

extern MarkerEventType* marker_event_type;

class MarkerEventType: public EventType
{
public:
	static MarkerEventType* GetInstance();

private:
	MarkerEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class MarkerEventTimeCell: public Cell
{
public:
	MarkerEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void SmallIncrease(SequenceEditor* sequence_editor, Row* row);
	void SmallDecrease(SequenceEditor* sequence_editor, Row* row);
	void LargeIncrease(SequenceEditor* sequence_editor, Row* row);
	void LargeDecrease(SequenceEditor* sequence_editor, Row* row);
	void Quantize(SequenceEditor* sequence_editor, Row* row);
};

class MarkerEventNameCell: public Cell
{
public:
	MarkerEventNameCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void EnterValue(SequenceEditor* sequence_editor, Row* row);
};

#endif
