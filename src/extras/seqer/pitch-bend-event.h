#ifndef PITCH_BEND_EVENT_INCLUDED
#define PITCH_BEND_EVENT_INCLUDED

class PitchBendEventRow;
class PitchBendEventTimeCell;
class PitchBendEventTrackCell;
class PitchBendEventChannelCell;
class PitchBendEventValueCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class PitchBendEventRow: public Row
{
public:
	PitchBendEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class PitchBendEventTimeCell: public Cell
{
public:
	PitchBendEventTimeCell(Row* row);
	wxString GetValueText();
};

class PitchBendEventTrackCell: public Cell
{
public:
	PitchBendEventTrackCell(Row* row);
	wxString GetValueText();
};

class PitchBendEventChannelCell: public Cell
{
public:
	PitchBendEventChannelCell(Row* row);
	wxString GetValueText();
};

class PitchBendEventValueCell: public Cell
{
public:
	PitchBendEventValueCell(Row* row);
	wxString GetValueText();
};

#endif
