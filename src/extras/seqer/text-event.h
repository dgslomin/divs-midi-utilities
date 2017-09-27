#ifndef TEXT_EVENT_INCLUDED
#define TEXT_EVENT_INCLUDED

class TextEventRow;
class TextEventTimeCell;
class TextEventTrackCell;
class TextEventTextCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class TextEventRow: public Row
{
public:
	TextEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class TextEventTimeCell: public Cell
{
public:
	TextEventTimeCell(Row* row);
	wxString GetValueText();
};

class TextEventTrackCell: public Cell
{
public:
	TextEventTrackCell(Row* row);
	wxString GetValueText();
};

class TextEventTextCell: public Cell
{
public:
	TextEventTextCell(Row* row);
	wxString GetValueText();
};

#endif
