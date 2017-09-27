#ifndef TEMPO_EVENT_INCLUDED
#define TEMPO_EVENT_INCLUDED

class TempoEventRow;
class TempoEventTimeCell;
class TempoEventTempoCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

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
