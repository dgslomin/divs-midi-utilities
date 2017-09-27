#ifndef PROGRAM_CHANGE_EVENT_INCLUDED
#define PROGRAM_CHANGE_EVENT_INCLUDED

class ProgramChangeEventRow;
class ProgramChangeEventTimeCell;
class ProgramChangeEventTrackCell;
class ProgramChangeEventChannelCell;
class ProgramChangeEventNumberCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class ProgramChangeEventRow: public Row
{
public:
	ProgramChangeEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class ProgramChangeEventTimeCell: public Cell
{
public:
	ProgramChangeEventTimeCell(Row* row);
	wxString GetValueText();
};

class ProgramChangeEventTrackCell: public Cell
{
public:
	ProgramChangeEventTrackCell(Row* row);
	wxString GetValueText();
};

class ProgramChangeEventChannelCell: public Cell
{
public:
	ProgramChangeEventChannelCell(Row* row);
	wxString GetValueText();
};

class ProgramChangeEventNumberCell: public Cell
{
public:
	ProgramChangeEventNumberCell(Row* row);
	wxString GetValueText();
};

#endif

