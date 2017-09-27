
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "lyric-event.h"

LyricEventRow::LyricEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Lyric");
	this->cells[0] = new LyricEventTimeCell(this);
	this->cells[1] = new LyricEventTrackCell(this);
	this->cells[2] = new Cell(this);
	this->cells[3] = new LyricEventLyricCell(this);
	this->cells[4] = new Cell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
}

LyricEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString LyricEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

LyricEventTrackCell(Row* row): Cell(row)
{
	this->label = wxString("Track");
}

wxString LyricEventTrackCell::GetValueText()
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(this->row->event)));
}

LyricEventLyricCell(Row* row): Cell(row)
{
	this->label = wxString("Lyric");
}

wxString LyricEventLyricCell::GetValueText()
{
	return wxString(MidiFileLyricEvent_getText(this->row->event));
}

