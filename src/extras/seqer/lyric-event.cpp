
#include <wx/wx.h>
#include <midifile.h>
#include "lyric-event.h"
#include "sequence-editor.h"

LyricEventType* LyricEventType::GetInstance()
{
	static LyricEventType* instance = new LyricEventType();
	return instance;
}

LyricEventType::LyricEventType()
{
	this->name = wxString("Lyric");
	this->short_name = wxString("Lyric");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new LyricEventTimeCell();
	this->cells[2] = new LyricEventTrackCell();
	this->cells[3] = new Cell();
	this->cells[4] = new LyricEventLyricCell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool LyricEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isLyricEvent(event);
}

LyricEventTimeCell::LyricEventTimeCell()
{
	this->label = wxString("Time");
}

wxString LyricEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

LyricEventTrackCell::LyricEventTrackCell()
{
	this->label = wxString("Track");
}

wxString LyricEventTrackCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row->event)));
}

LyricEventLyricCell::LyricEventLyricCell()
{
	this->label = wxString("Lyric");
}

wxString LyricEventLyricCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString(MidiFileLyricEvent_getText(row->event));
}

