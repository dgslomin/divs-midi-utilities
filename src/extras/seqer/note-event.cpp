
#include <wx/wx.h>
#include <midifile.h>
#include "note-event.h"
#include "sequence-editor.h"

void SequenceEditor::InsertNote(int diatonic)
{
	if (this->overwrite_mode && (this->current_row_number < this->rows.size()) && MidiFileEvent_isNoteStartEvent(this->rows[this->current_row_number].event))
	{
		MidiFileEvent_t event = this->rows[this->current_row_number].event;
		MidiFileTrack_t track = MidiFileEvent_getTrack(event);
		int note_number = MidiFileNoteStartEvent_getNote(event);
		long start_tick = MidiFileEvent_getTick(event);
		int chromatic = GetChromaticFromDiatonicInKey(diatonic, MidiFileKeySignatureEvent_getNumber(MidiFile_getLatestKeySignatureEventForTick(this->sequence->midi_file, start_tick)));
		int new_note_number = this->insertion_note_number = MatchNoteOctave(SetNoteChromatic(this->insertion_note_number, chromatic), this->insertion_note_number);
		MidiFileNoteStartEvent_setNote(event, new_note_number);
		this->sequence->RefreshData();

		this->sequence->undo_command_processor->Submit(new UndoCommand(
			[=]{
				MidiFileNoteStartEvent_setNote(event, note_number);
				this->sequence->RefreshData();
			},
			NULL,
			[=]{
				MidiFileNoteStartEvent_setNote(event, new_note_number);
				this->sequence->RefreshData();
			},
			NULL
		));
	}
	else
	{
		MidiFileTrack_t track = MidiFile_getTrackByNumber(this->sequence->midi_file, this->insertion_track_number, 1);
		long start_step_number = this->GetStepNumberFromRowNumber(this->current_row_number);
		long start_tick = this->step_size->GetTickFromStep(start_step_number);
		long end_tick = this->step_size->GetTickFromStep(start_step_number + 1);
		int chromatic = GetChromaticFromDiatonicInKey(diatonic, MidiFileKeySignatureEvent_getNumber(MidiFile_getLatestKeySignatureEventForTick(this->sequence->midi_file, start_tick)));
		this->insertion_note_number = MatchNoteOctave(SetNoteChromatic(this->insertion_note_number, chromatic), this->insertion_note_number);
		MidiFileEvent_t event = MidiFileTrack_createNoteStartAndEndEvents(track, start_tick, end_tick, this->insertion_channel_number, this->insertion_note_number, this->insertion_velocity, this->insertion_end_velocity);
		MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
		this->sequence->RefreshData();
		this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

		this->sequence->undo_command_processor->Submit(new UndoCommand(
			[=]{
				MidiFileEvent_detach(event);
				MidiFileEvent_detach(end_event);
				this->sequence->RefreshData();
			},
			[=]{
				MidiFileEvent_delete(event);
				MidiFileEvent_delete(end_event);
			},
			[=]{
				MidiFileEvent_setTrack(event, track);
				MidiFileEvent_setTrack(end_event, track);
				this->sequence->RefreshData();
			},
			NULL
		));
	}
}

NoteEventType::NoteEventType()
{
	this->name = wxString("Note");
}

bool NoteEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isNoteStartEvent(event);
}

Row* NoteEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new NoteEventRow(sequence_editor, step_number, event);
}

NoteEventRow::NoteEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Note");
	this->cells[0] = new NoteEventTimeCell(this);
	this->cells[1] = new NoteEventTrackCell(this);
	this->cells[2] = new NoteEventChannelCell(this);
	this->cells[3] = new NoteEventNoteCell(this);
	this->cells[4] = new NoteEventVelocityCell(this);
	this->cells[5] = new NoteEventEndTimeCell(this);
	this->cells[6] = new NoteEventEndVelocityCell(this);
}

void NoteEventRow::Delete()
{
	SequenceEditor* sequence_editor = this->sequence_editor;
	MidiFileEvent_t event = this->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	MidiFileEvent_detach(event);
	MidiFileEvent_detach(end_event);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTrack(event, track);
			MidiFileEvent_setTrack(end_event, track);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_detach(event);
			MidiFileEvent_detach(end_event);
			sequence_editor->sequence->RefreshData();
		},
		[=]{
			MidiFileEvent_delete(event);
			MidiFileEvent_delete(end_event);
		}
	));
}

NoteEventTimeCell(Row* row): SequenceEditorCell(row)
{
	this->label = wxString("Time");
}

wxString NoteEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

void NoteEventTimeCell::SmallIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long new_tick = tick + sequence_editor->GetNumberOfTicksPerPixel(sequence_editor->GetStepNumberFromTick(tick));
	long end_tick = MidiFileEvent_getTick(end_event);
	long new_end_tick = end_tick + (new_tick - tick);
	MidiFileEvent_setTick(event, new_tick);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(event, tick);
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(event, new_tick);
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventTimeCell::SmallDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long new_tick = std::max<long>(tick - sequence_editor->GetNumberOfTicksPerPixel(sequence_editor->GetStepNumberFromTick(tick)), 0);
	long end_tick = MidiFileEvent_getTick(end_event);
	long new_end_tick = end_tick - (tick - new_tick);
	MidiFileEvent_setTick(event, new_tick);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(event, tick);
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(event, new_tick);
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventTimeCell::LargeIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long step_number = sequence_editor->GetStepNumberFromTick(tick);
	long step_tick = sequence_editor->step_size->GetTickFromStep(step_number);
	long new_step_tick = sequence_editor->step_size->GetTickFromStep(step_number + 1);
	long new_tick = new_step_tick + (tick - step_tick);
	long end_tick = MidiFileEvent_getTick(end_event);
	long new_end_tick = end_tick + (new_tick - tick);
	MidiFileEvent_setTick(event, new_tick);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(event, tick);
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(event, new_tick);
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventTimeCell::LargeDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long step_number = sequence_editor->GetStepNumberFromTick(tick);
	long step_tick = sequence_editor->step_size->GetTickFromStep(step_number);
	long new_step_tick = sequence_editor->step_size->GetTickFromStep(std::max<long>(step_number - 1, 0));
	long new_tick = new_step_tick + (tick - step_tick);
	long end_tick = MidiFileEvent_getTick(end_event);
	long new_end_tick = end_tick - (tick - new_tick);
	MidiFileEvent_setTick(event, new_tick);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(event, tick);
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(event, new_tick);
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventTimeCell::Quantize()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long new_tick = sequence_editor->step_size->GetTickFromStep(sequence_editor->GetStepNumberFromTick(tick));
	long end_tick = MidiFileEvent_getTick(end_event);
	long new_end_tick = end_tick - (tick - new_tick);
	MidiFileEvent_setTick(event, new_tick);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(event, tick);
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(event, new_tick);
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

NoteEventTrackCell(Row* row): SequenceEditorCell(row)
{
	this->label = wxString("Track");
}

wxString NoteEventTrackCell::GetValueText()
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(this->row->event)));
}

void NoteEventTrackCell::SmallIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	MidiFileTrack_t track = MidiFileEvent_getTrack(event);
	sequence_editor->insertion_track_number = MidiFileTrack_getNumber(track) + 1;
	MidiFileTrack_t new_track = MidiFile_getTrackByNumber(sequence_editor->sequence->midi_file, sequence_editor->insertion_track_number, 1);
	MidiFileEvent_setTrack(end_event, new_track);
	MidiFileEvent_setTrack(event, new_track);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTrack(end_event, track);
			MidiFileEvent_setTrack(event, track);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTrack(end_event, new_track);
			MidiFileEvent_setTrack(event, new_track);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventTrackCell::SmallDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	MidiFileTrack_t track = MidiFileEvent_getTrack(event);
	sequence_editor->insertion_track_number = std::max<int>(MidiFileTrack_getNumber(track) - 1, 0);
	MidiFileTrack_t new_track = MidiFile_getTrackByNumber(sequence_editor->sequence->midi_file, sequence_editor->insertion_track_number, 1);
	MidiFileEvent_setTrack(end_event, new_track);
	MidiFileEvent_setTrack(event, new_track);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTrack(end_event, track);
			MidiFileEvent_setTrack(event, track);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTrack(end_event, new_track);
			MidiFileEvent_setTrack(event, new_track);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

NoteEventChannelCell(Row* row): SequenceEditorCell(row)
{
	this->label = wxString("Channel");
}

wxString NoteEventChannelCell::GetValueText()
{
	return wxString::Format("%d", MidiFileNoteStartEvent_getChannel(this->row->event) + 1);
}

void NoteEventChannelCell::SmallIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int channel_number = MidiFileNoteStartEvent_getChannel(event);
	int new_channel_number = sequence_editor->insertion_channel_number = std::min<int>(channel_number + 1, 15);
	MidiFileNoteStartEvent_setChannel(event, new_channel_number);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setChannel(event, channel_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setChannel(event, new_channel_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventChannelCell::SmallDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int channel_number = MidiFileNoteStartEvent_getChannel(event);
	int new_channel_number = sequence_editor->insertion_channel_number = std::max<int>(channel_number - 1, 0);
	MidiFileNoteStartEvent_setChannel(event, new_channel_number);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setChannel(event, channel_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setChannel(event, new_channel_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

NoteEventNoteCell(Row* row): SequenceEditorCell(row)
{
	this->label = wxString("Note");
}

wxString NoteEventNoteCell::GetValueText()
{
	return GetNoteNameFromNumber(MidiFileNoteStartEvent_getNote(this->row->event));
}

void NoteEventNoteCell::SmallIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int note_number = MidiFileNoteStartEvent_getNote(event);
	int new_note_number = sequence_editor->insertion_note_number = std::min<int>(note_number + 1, 127);
	MidiFileNoteStartEvent_setNote(event, new_note_number);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setNote(event, note_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setNote(event, new_note_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventNoteCell::SmallDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int note_number = MidiFileNoteStartEvent_getNote(event);
	int new_note_number = sequence_editor->insertion_note_number = std::max<int>(note_number - 1, 0);
	MidiFileNoteStartEvent_setNote(event, new_note_number);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setNote(event, note_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setNote(event, new_note_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventNoteCell::LargeIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int note_number = MidiFileNoteStartEvent_getNote(event);
	int new_note_number = sequence_editor->insertion_note_number = std::min<int>(note_number + 12, 127);
	MidiFileNoteStartEvent_setNote(event, new_note_number);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setNote(event, note_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setNote(event, new_note_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventNoteCell::LargeDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int note_number = MidiFileNoteStartEvent_getNote(event);
	int new_note_number = sequence_editor->insertion_note_number = std::max<int>(note_number - 12, 0);
	MidiFileNoteStartEvent_setNote(event, new_note_number);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setNote(event, note_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setNote(event, new_note_number);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

NoteEventVelocityCell(Row* row): SequenceEditorCell(row)
{
	this->label = wxString("Velocity");
}

wxString NoteEventVelocityCell::GetValueText()
{
	return wxString::Format("%d", MidiFileNoteStartEvent_getVelocity(this->row->event));
}

void NoteEventVelocityCell::SmallIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int velocity = MidiFileNoteStartEvent_getVelocity(event);
	int new_velocity = sequence_editor->insertion_velocity = std::min<int>(velocity + 1, 127);
	MidiFileNoteStartEvent_setVelocity(event, new_velocity);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setVelocity(event, velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setVelocity(event, new_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventVelocityCell::SmallDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int velocity = MidiFileNoteStartEvent_getVelocity(event);
	int new_velocity = sequence_editor->insertion_velocity = std::max<int>(velocity - 1, 1);
	MidiFileNoteStartEvent_setVelocity(event, new_velocity);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setVelocity(event, velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setVelocity(event, new_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventVelocityCell::LargeIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int velocity = MidiFileNoteStartEvent_getVelocity(event);
	int new_velocity = sequence_editor->insertion_velocity = std::min<int>(velocity + 8, 127);
	MidiFileNoteStartEvent_setVelocity(event, new_velocity);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setVelocity(event, velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setVelocity(event, new_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventVelocityCell::LargeDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	int velocity = MidiFileNoteStartEvent_getVelocity(event);
	int new_velocity = sequence_editor->insertion_velocity = std::max<int>(velocity - 8, 1);
	MidiFileNoteStartEvent_setVelocity(event, new_velocity);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteStartEvent_setVelocity(event, velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteStartEvent_setVelocity(event, new_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

NoteEventEndTimeCell(Row* row): SequenceEditorCell(row)
{
	this->label = wxString("End time");
}

wxString NoteEventEndTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(this->row->event)));
}

void NoteEventEndTimeCell::SmallIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long end_tick = MidiFileEvent_getTick(end_event);
	long new_end_tick = end_tick + sequence_editor->GetNumberOfTicksPerPixel(sequence_editor->GetStepNumberFromTick(end_tick));
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventEndTimeCell::SmallDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long end_tick = MidiFileEvent_getTick(end_event);
	long new_end_tick = std::max<long>(end_tick - sequence_editor->GetNumberOfTicksPerPixel(sequence_editor->GetStepNumberFromTick(end_tick)), tick);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventEndTimeCell::LargeIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long end_tick = MidiFileEvent_getTick(end_event);
	long end_step_number = sequence_editor->GetStepNumberFromTick(end_tick);
	long end_step_tick = sequence_editor->step_size->GetTickFromStep(end_step_number);
	long new_end_step_tick = sequence_editor->step_size->GetTickFromStep(end_step_number + 1);
	long new_end_tick = new_end_step_tick + (end_tick - end_step_tick);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventEndTimeCell::LargeDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long end_tick = MidiFileEvent_getTick(end_event);
	long end_step_number = sequence_editor->GetStepNumberFromTick(end_tick);
	long end_step_tick = sequence_editor->step_size->GetTickFromStep(end_step_number);
	long new_end_step_tick = sequence_editor->step_size->GetTickFromStep(end_step_number - 1);
	long new_end_tick = std::max<long>(new_end_step_tick + (end_tick - end_step_tick), tick);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventEndTimeCell::Quantize()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	long tick = MidiFileEvent_getTick(event);
	long end_tick = MidiFileEvent_getTick(end_event);
	long new_end_tick = sequence_editor->step_size->GetTickFromStep(sequence_editor->GetStepNumberFromTick(end_tick) + 1);
	MidiFileEvent_setTick(end_event, new_end_tick);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_setTick(end_event, end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileEvent_setTick(end_event, new_end_tick);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

NoteEventEndVelocityCell(Row* row): SequenceEditorCell(row)
{
	this->label = wxString("End velocity");
}

wxString NoteEventEndVelocityCell::GetValueText()
{
	return wxString::Format("%d", MidiFileNoteEndEvent_getVelocity(MidiFileNoteStartEvent_getNoteEndEvent(this->row->event)));
}

void NoteEventEndVelocityCell::SmallIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	int end_velocity = MidiFileNoteEndEvent_getVelocity(end_event);
	int new_end_velocity = sequence_editor->insertion_end_velocity = std::min<int>(end_velocity + 1, 127);
	MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteEndEvent_setVelocity(end_event, end_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventEndVelocityCell::SmallDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	int end_velocity = MidiFileNoteEndEvent_getVelocity(end_event);
	int new_end_velocity = sequence_editor->insertion_end_velocity = std::max<int>(end_velocity - 1, 0);
	MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteEndEvent_setVelocity(end_event, end_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventEndVelocityCell::LargeIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	int end_velocity = MidiFileNoteEndEvent_getVelocity(end_event);
	int new_end_velocity = sequence_editor->insertion_end_velocity = std::min<int>(end_velocity + 8, 127);
	MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteEndEvent_setVelocity(end_event, end_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

void NoteEventEndVelocityCell::LargeDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	int end_velocity = MidiFileNoteEndEvent_getVelocity(end_event);
	int new_end_velocity = sequence_editor->insertion_end_velocity = std::max<int>(end_velocity - 8, 0);
	MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
	sequence_editor->sequence->RefreshData();

	sequence_editor->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileNoteEndEvent_setVelocity(end_event, end_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL,
		[=]{
			MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
			sequence_editor->sequence->RefreshData();
		},
		NULL
	));
}

