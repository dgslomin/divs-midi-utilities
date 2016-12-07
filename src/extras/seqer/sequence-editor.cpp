
#include <math.h>
#include <algorithm>
#include <functional>
#include <vector>
#include <wx/wx.h>
#include <wx/cmdproc.h>
#include <midifile.h>
#include "seqer.h"
#include "sequence-editor.h"
#include "event-list.h"
#include "piano-roll.h"
#include "music-math.h"
#include "color.h"

#ifdef __WXMSW__
#define CANVAS_BORDER wxBORDER_THEME
#else
#define CANVAS_BORDER wxBORDER_DEFAULT
#endif

SequenceEditor::SequenceEditor(Window* window, SequenceEditor* existing_sequence_editor): wxScrolledCanvas(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | CANVAS_BORDER)
{
	this->window = window;
	this->sequence = (existing_sequence_editor == NULL) ? new Sequence() : existing_sequence_editor->sequence;
	this->sequence->AddSequenceEditor(this);
	this->event_list = new EventList(this);
	this->piano_roll = new PianoRoll(this);
	this->step_size = new StepsPerMeasureSize(this);
	this->current_row_number = 0;
	this->last_row_number = 0;
	this->current_column_number = 1;
	this->insertion_track_number = 1;
	this->insertion_channel_number = 0;
	this->insertion_note_number = 60;
	this->insertion_velocity = 64;
	this->insertion_end_velocity = 0;
#ifndef __WXOSX__
	this->SetDoubleBuffered(true);
#endif
	this->DisableKeyboardScrolling();
	this->SetBackgroundColour(*wxWHITE);
	this->RefreshData();
}

SequenceEditor::~SequenceEditor()
{
	this->sequence->RemoveSequenceEditor(this);
	delete this->event_list;
	delete this->piano_roll;
	delete this->step_size;
}

bool SequenceEditor::IsModified()
{
	return this->sequence->is_modified;
}

wxString SequenceEditor::GetFilename()
{
	return this->sequence->filename;
}

void SequenceEditor::New()
{
	this->sequence->RemoveSequenceEditor(this);
	this->sequence = new Sequence();
	this->sequence->AddSequenceEditor(this);
	this->current_row_number = 0;
	this->last_row_number = 0;
	this->current_column_number = 0;
	this->RefreshData();
}

bool SequenceEditor::Load(wxString filename)
{
	MidiFile_t new_midi_file = MidiFile_load((char *)(filename.ToStdString().c_str()));
	if (new_midi_file == NULL) return false;
	this->sequence->RemoveSequenceEditor(this);
	this->sequence = new Sequence();
	this->sequence->AddSequenceEditor(this);
	this->sequence->filename = filename;
	MidiFile_free(this->sequence->midi_file);
	this->sequence->midi_file = new_midi_file;
	this->current_row_number = 0;
	this->last_row_number = 0;
	this->current_column_number = 0;
	this->SetStepSize(new StepsPerMeasureSize(this));
	this->RefreshData();
	return true;
}

bool SequenceEditor::Save()
{
	if (this->sequence->filename == wxEmptyString) return false;

	if (MidiFile_save(this->sequence->midi_file, (char *)(this->sequence->filename.ToStdString().c_str())) == 0)
	{
		this->sequence->is_modified = false;
		return true;
	}
	else
	{
		return false;
	}
}

bool SequenceEditor::SaveAs(wxString filename)
{
	if (MidiFile_save(this->sequence->midi_file, (char *)(filename.ToStdString().c_str())) == 0)
	{
		this->sequence->filename = filename;
		this->sequence->is_modified = false;
		return true;
	}
	else
	{
		return false;
	}
}

void SequenceEditor::SetStepSize(StepSize* step_size)
{
	RowLocator row_locator = this->GetLocatorFromRowNumber(this->current_row_number);
	delete this->step_size;
	this->step_size = step_size;
	this->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberFromLocator(row_locator));
	this->RefreshDisplay();
}

void SequenceEditor::ZoomIn()
{
	this->SetStepSize(this->step_size->ZoomIn());
}

void SequenceEditor::ZoomOut()
{
	this->SetStepSize(this->step_size->ZoomOut());
}

void SequenceEditor::SetFilters(std::vector<int> filtered_event_types, std::vector<int> filtered_tracks, std::vector<int> filtered_channels)
{
	RowLocator row_locator = this->GetLocatorFromRowNumber(this->current_row_number);
	this->filtered_event_types = filtered_event_types;
	this->filtered_tracks = filtered_tracks;
	this->filtered_channels = filtered_channels;
	this->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberFromLocator(row_locator));
	this->RefreshDisplay();
}

void SequenceEditor::RowUp()
{
	this->SetCurrentRowNumber(std::max<long>(this->current_row_number - 1, 0));
}

void SequenceEditor::RowDown()
{
	this->SetCurrentRowNumber(current_row_number + 1);
}

void SequenceEditor::PageUp()
{
	this->SetCurrentRowNumber(std::max<long>(this->current_row_number - this->GetNumberOfVisibleRows(), 0));
}

void SequenceEditor::PageDown()
{
	this->SetCurrentRowNumber(current_row_number + this->GetNumberOfVisibleRows());
}

void SequenceEditor::GoToFirstRow()
{
	this->SetCurrentRowNumber(0);
}

void SequenceEditor::GoToLastRow()
{
	this->SetCurrentRowNumber(std::max<long>(this->rows.size() - 1, this->last_row_number));
}

void SequenceEditor::ColumnLeft()
{
	this->current_column_number = std::max<long>(this->current_column_number - 1, 1);
	this->RefreshDisplay();
}

void SequenceEditor::ColumnRight()
{
	this->current_column_number = std::min<long>(this->current_column_number + 1, 7);
	this->RefreshDisplay();
}

void SequenceEditor::GoToColumn(int column_number)
{
	this->current_column_number = std::min<long>(std::max<long>(column_number, 1), 7);
	this->RefreshDisplay();
}

void SequenceEditor::GoToNextMarker()
{
	for (long row_number = this->current_row_number + 1; row_number < this->rows.size(); row_number++)
	{
		if (MidiFileEvent_isMarkerEvent(this->rows[row_number].event))
		{
			this->SetCurrentRowNumber(row_number);
			return;
		}
	}

	long current_tick = this->GetTickFromRowNumber(this->current_row_number);

	for (MidiFileEvent_t event = MidiFileTrack_getFirstEvent(MidiFile_getFirstTrack(this->sequence->midi_file)); event != NULL; event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isMarkerEvent(event))
		{
			long tick = MidiFileEvent_getTick(event);

			if (tick > current_tick)
			{
				this->SetCurrentRowNumber(this->GetRowNumberFromTick(tick));
				return;
			}
		}
	}
}

void SequenceEditor::GoToPreviousMarker()
{
	for (long row_number = std::min<long>(this->current_row_number, this->rows.size()) - 1; row_number >= 0; row_number--)
	{
		if (MidiFileEvent_isMarkerEvent(this->rows[row_number].event))
		{
			this->SetCurrentRowNumber(row_number);
			return;
		}
	}

	long current_tick = this->GetTickFromRowNumber(this->current_row_number);

	for (MidiFileEvent_t event = MidiFileTrack_getLastEvent(MidiFile_getFirstTrack(this->sequence->midi_file)); event != NULL; event = MidiFileEvent_getPreviousEventInTrack(event))
	{
		if (MidiFileEvent_isMarkerEvent(event))
		{
 			long tick = MidiFileEvent_getTick(event);

			if (tick < current_tick)
			{
				this->SetCurrentRowNumber(this->GetRowNumberFromTick(tick));
				return;
			}
		}
	}
}

void SequenceEditor::GoToMarker(wxString marker_name)
{
	for (long row_number = 0; row_number < this->rows.size(); row_number++)
	{
		MidiFileEvent_t event = rows[row_number].event;

		if (MidiFileEvent_isMarkerEvent(event) && (marker_name.Cmp(MidiFileMarkerEvent_getText(event)) == 0))
		{
			this->SetCurrentRowNumber(row_number);
			return;
		}
	}

	long tick = MidiFile_getTickFromMarker(this->sequence->midi_file, (char *)(marker_name.ToStdString().c_str()));
	if (tick >= 0) this->SetCurrentRowNumber(this->GetRowNumberFromTick(tick));
}

void SequenceEditor::InsertNote(int diatonic)
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

void SequenceEditor::InsertMarker()
{
	MidiFileTrack_t track = MidiFile_getTrackByNumber(this->sequence->midi_file, 0, 1);
	MidiFileEvent_t event = MidiFileTrack_createMarkerEvent(track, this->step_size->GetTickFromStep(this->GetStepNumberFromRowNumber(this->current_row_number)), (char *)(""));
	this->sequence->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

	this->sequence->undo_command_processor->Submit(new UndoCommand(
		[=]{
			MidiFileEvent_detach(event);
			this->sequence->RefreshData();
		},
		[=]{
			MidiFileEvent_delete(event);
		},
		[=]{
			MidiFileEvent_setTrack(event, track);
			this->sequence->RefreshData();
		},
		NULL
	));
}

void SequenceEditor::DeleteRow()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_t event = this->rows[this->current_row_number].event;
	MidiFileTrack_t track = MidiFileEvent_getTrack(event);

	switch (this->GetEventType(event))
	{
		case EVENT_TYPE_NOTE:
		{
			MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
			MidiFileEvent_detach(event);
			MidiFileEvent_detach(end_event);
			this->sequence->RefreshData();

			this->sequence->undo_command_processor->Submit(new UndoCommand(
				[=]{
					MidiFileEvent_setTrack(event, track);
					MidiFileEvent_setTrack(end_event, track);
					this->sequence->RefreshData();
				},
				NULL,
				[=]{
					MidiFileEvent_detach(event);
					MidiFileEvent_detach(end_event);
					this->sequence->RefreshData();
				},
				[=]{
					MidiFileEvent_delete(event);
					MidiFileEvent_delete(end_event);
				}
			));

			break;
		}
		case EVENT_TYPE_MARKER:
		{
			MidiFileEvent_detach(event);
			this->sequence->RefreshData();

			this->sequence->undo_command_processor->Submit(new UndoCommand(
				[=]{
					MidiFileEvent_setTrack(event, track);
					this->sequence->RefreshData();
				},
				NULL,
				[=]{
					MidiFileEvent_detach(event);
					this->sequence->RefreshData();
				},
				[=]{
					MidiFileEvent_delete(event);
				}
			));

			break;
		}
		default:
		{
			break;
		}
	}
}

void SequenceEditor::EnterValue()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_t event = this->rows[this->current_row_number].event;

	switch (this->GetEventType(event))
	{
		case EVENT_TYPE_MARKER:
		{
			switch (this->current_column_number)
			{
				case 4:
				{
					wxTextEntryDialog* dialog = new wxTextEntryDialog(this, "Name", "Edit Marker", MidiFileMarkerEvent_getText(event));

					if (dialog->ShowModal() == wxID_OK)
					{
						wxString name = wxString(MidiFileMarkerEvent_getText(event));
						wxString new_name = dialog->GetValue();
						MidiFileMarkerEvent_setText(event, (char *)(new_name.ToStdString().c_str()));
						this->sequence->RefreshData();

						this->sequence->undo_command_processor->Submit(new UndoCommand(
							[=]{
								MidiFileMarkerEvent_setText(event, (char *)(name.ToStdString().c_str()));
								this->sequence->RefreshData();
							},
							NULL,
							[=]{
								MidiFileMarkerEvent_setText(event, (char *)(new_name.ToStdString().c_str()));
								this->sequence->RefreshData();
							},
							NULL
						));
					}

					dialog->Destroy();
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

void SequenceEditor::SmallIncrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_t event = this->rows[this->current_row_number].event;

	switch (this->GetEventType(event))
	{
		case EVENT_TYPE_NOTE:
		{
			MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long new_tick = tick + this->GetNumberOfTicksPerPixel(this->GetStepNumberFromTick(tick));
					long end_tick = MidiFileEvent_getTick(end_event);
					long new_end_tick = end_tick + (new_tick - tick);
					MidiFileEvent_setTick(event, new_tick);
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 2:
				{
					MidiFileTrack_t track = MidiFileEvent_getTrack(event);
					this->insertion_track_number = MidiFileTrack_getNumber(track) + 1;
					MidiFileTrack_t new_track = MidiFile_getTrackByNumber(this->sequence->midi_file, this->insertion_track_number, 1);
					MidiFileEvent_setTrack(end_event, new_track);
					MidiFileEvent_setTrack(event, new_track);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTrack(end_event, track);
							MidiFileEvent_setTrack(event, track);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTrack(end_event, new_track);
							MidiFileEvent_setTrack(event, new_track);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 3:
				{
					int channel_number = MidiFileNoteStartEvent_getChannel(event);
					int new_channel_number = this->insertion_channel_number = std::min<int>(channel_number + 1, 15);
					MidiFileNoteStartEvent_setChannel(event, new_channel_number);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteStartEvent_setChannel(event, channel_number);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteStartEvent_setChannel(event, new_channel_number);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 4:
				{
					int note_number = MidiFileNoteStartEvent_getNote(event);
					int new_note_number = this->insertion_note_number = std::min<int>(note_number + 1, 127);
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

					break;
				}
				case 5:
				{
					int velocity = MidiFileNoteStartEvent_getVelocity(event);
					int new_velocity = this->insertion_velocity = std::min<int>(velocity + 1, 127);
					MidiFileNoteStartEvent_setVelocity(event, new_velocity);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteStartEvent_setVelocity(event, velocity);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteStartEvent_setVelocity(event, new_velocity);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 6:
				{
					long tick = MidiFileEvent_getTick(event);
					long end_tick = MidiFileEvent_getTick(end_event);
					long new_end_tick = end_tick + this->GetNumberOfTicksPerPixel(this->GetStepNumberFromTick(end_tick));
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 7:
				{
					int end_velocity = MidiFileNoteEndEvent_getVelocity(end_event);
					int new_end_velocity = this->insertion_end_velocity = std::min<int>(end_velocity + 1, 127);
					MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteEndEvent_setVelocity(end_event, end_velocity);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_MARKER:
		{
			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long new_tick = tick + this->GetNumberOfTicksPerPixel(this->GetStepNumberFromTick(tick));
					MidiFileEvent_setTick(event, new_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

void SequenceEditor::SmallDecrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_t event = this->rows[this->current_row_number].event;

	switch (this->GetEventType(event))
	{
		case EVENT_TYPE_NOTE:
		{
			MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long new_tick = std::max<long>(tick - this->GetNumberOfTicksPerPixel(this->GetStepNumberFromTick(tick)), 0);
					long end_tick = MidiFileEvent_getTick(end_event);
					long new_end_tick = end_tick - (tick - new_tick);
					MidiFileEvent_setTick(event, new_tick);
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 2:
				{
					MidiFileTrack_t track = MidiFileEvent_getTrack(event);
					this->insertion_track_number = std::max<int>(MidiFileTrack_getNumber(track) - 1, 0);
					MidiFileTrack_t new_track = MidiFile_getTrackByNumber(this->sequence->midi_file, this->insertion_track_number, 1);
					MidiFileEvent_setTrack(end_event, new_track);
					MidiFileEvent_setTrack(event, new_track);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTrack(end_event, track);
							MidiFileEvent_setTrack(event, track);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTrack(end_event, new_track);
							MidiFileEvent_setTrack(event, new_track);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 3:
				{
					int channel_number = MidiFileNoteStartEvent_getChannel(event);
					int new_channel_number = this->insertion_channel_number = std::max<int>(channel_number - 1, 0);
					MidiFileNoteStartEvent_setChannel(event, new_channel_number);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteStartEvent_setChannel(event, channel_number);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteStartEvent_setChannel(event, new_channel_number);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 4:
				{
					int note_number = MidiFileNoteStartEvent_getNote(event);
					int new_note_number = this->insertion_note_number = std::max<int>(note_number - 1, 0);
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

					break;
				}
				case 5:
				{
					int velocity = MidiFileNoteStartEvent_getVelocity(event);
					int new_velocity = this->insertion_velocity = std::max<int>(velocity - 1, 1);
					MidiFileNoteStartEvent_setVelocity(event, new_velocity);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteStartEvent_setVelocity(event, velocity);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteStartEvent_setVelocity(event, new_velocity);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 6:
				{
					long tick = MidiFileEvent_getTick(event);
					long end_tick = MidiFileEvent_getTick(end_event);
					long new_end_tick = std::max<long>(end_tick - this->GetNumberOfTicksPerPixel(this->GetStepNumberFromTick(end_tick)), tick);
					MidiFileEvent_setTick(end_event, new_end_tick);

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 7:
				{
					int end_velocity = MidiFileNoteEndEvent_getVelocity(end_event);
					int new_end_velocity = this->insertion_end_velocity = std::max<int>(end_velocity - 1, 0);
					MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteEndEvent_setVelocity(end_event, end_velocity);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_MARKER:
		{
			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long new_tick = std::max<long>(tick - this->GetNumberOfTicksPerPixel(this->GetStepNumberFromTick(tick)), 0);
					MidiFileEvent_setTick(event, new_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

void SequenceEditor::LargeIncrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_t event = this->rows[this->current_row_number].event;

	switch (this->GetEventType(event))
	{
		case EVENT_TYPE_NOTE:
		{
			MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long step_number = this->GetStepNumberFromTick(tick);
					long step_tick = this->step_size->GetTickFromStep(step_number);
					long new_step_tick = this->step_size->GetTickFromStep(step_number + 1);
					long new_tick = new_step_tick + (tick - step_tick);
					long end_tick = MidiFileEvent_getTick(end_event);
					long new_end_tick = end_tick + (new_tick - tick);
					MidiFileEvent_setTick(event, new_tick);
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 2:
				{
					MidiFileTrack_t track = MidiFileEvent_getTrack(event);
					this->insertion_track_number = MidiFileTrack_getNumber(track) + 1;
					MidiFileTrack_t new_track = MidiFile_getTrackByNumber(this->sequence->midi_file, this->insertion_track_number, 1);
					MidiFileEvent_setTrack(end_event, new_track);
					MidiFileEvent_setTrack(event, new_track);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTrack(end_event, track);
							MidiFileEvent_setTrack(event, track);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTrack(end_event, new_track);
							MidiFileEvent_setTrack(event, new_track);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 3:
				{
					int channel_number = MidiFileNoteStartEvent_getChannel(event);
					int new_channel_number = this->insertion_channel_number = std::min<int>(channel_number + 1, 15);
					MidiFileNoteStartEvent_setChannel(event, new_channel_number);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteStartEvent_setChannel(event, channel_number);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteStartEvent_setChannel(event, new_channel_number);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 4:
				{
					int note_number = MidiFileNoteStartEvent_getNote(event);
					int new_note_number = this->insertion_note_number = std::min<int>(note_number + 12, 127);
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

					break;
				}
				case 5:
				{
					int velocity = MidiFileNoteStartEvent_getVelocity(event);
					int new_velocity = this->insertion_velocity = std::min<int>(velocity + 8, 127);
					MidiFileNoteStartEvent_setVelocity(event, new_velocity);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteStartEvent_setVelocity(event, velocity);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteStartEvent_setVelocity(event, new_velocity);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 6:
				{
					long tick = MidiFileEvent_getTick(event);
					long end_tick = MidiFileEvent_getTick(end_event);
					long end_step_number = this->GetStepNumberFromTick(end_tick);
					long end_step_tick = this->step_size->GetTickFromStep(end_step_number);
					long new_end_step_tick = this->step_size->GetTickFromStep(end_step_number + 1);
					long new_end_tick = new_end_step_tick + (end_tick - end_step_tick);
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 7:
				{
					int end_velocity = MidiFileNoteEndEvent_getVelocity(end_event);
					int new_end_velocity = this->insertion_end_velocity = std::min<int>(end_velocity + 8, 127);
					MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteEndEvent_setVelocity(end_event, end_velocity);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_MARKER:
		{
			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long step_number = this->GetStepNumberFromTick(tick);
					long step_tick = this->step_size->GetTickFromStep(step_number);
					long new_step_tick = this->step_size->GetTickFromStep(step_number + 1);
					long new_tick = new_step_tick + (tick - step_tick);
					MidiFileEvent_setTick(event, new_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

void SequenceEditor::LargeDecrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_t event = this->rows[this->current_row_number].event;

	switch (this->GetEventType(event))
	{
		case EVENT_TYPE_NOTE:
		{
			MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long step_number = this->GetStepNumberFromTick(tick);
					long step_tick = this->step_size->GetTickFromStep(step_number);
					long new_step_tick = this->step_size->GetTickFromStep(std::max<long>(step_number - 1, 0));
					long new_tick = new_step_tick + (tick - step_tick);
					long end_tick = MidiFileEvent_getTick(end_event);
					long new_end_tick = end_tick - (tick - new_tick);
					MidiFileEvent_setTick(event, new_tick);
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 2:
				{
					MidiFileTrack_t track = MidiFileEvent_getTrack(event);
					this->insertion_track_number = std::max<int>(MidiFileTrack_getNumber(track) - 1, 0);
					MidiFileTrack_t new_track = MidiFile_getTrackByNumber(this->sequence->midi_file, this->insertion_track_number, 1);
					MidiFileEvent_setTrack(end_event, new_track);
					MidiFileEvent_setTrack(event, new_track);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTrack(end_event, track);
							MidiFileEvent_setTrack(event, track);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTrack(end_event, new_track);
							MidiFileEvent_setTrack(event, new_track);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 3:
				{
					int channel_number = MidiFileNoteStartEvent_getChannel(event);
					int new_channel_number = this->insertion_channel_number = std::max<int>(channel_number - 1, 0);
					MidiFileNoteStartEvent_setChannel(event, new_channel_number);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteStartEvent_setChannel(event, channel_number);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteStartEvent_setChannel(event, new_channel_number);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 4:
				{
					int note_number = MidiFileNoteStartEvent_getNote(event);
					int new_note_number = this->insertion_note_number = std::max<int>(note_number - 12, 0);
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

					break;
				}
				case 5:
				{
					int velocity = MidiFileNoteStartEvent_getVelocity(event);
					int new_velocity = this->insertion_velocity = std::max<int>(velocity - 8, 1);
					MidiFileNoteStartEvent_setVelocity(event, new_velocity);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteStartEvent_setVelocity(event, velocity);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteStartEvent_setVelocity(event, new_velocity);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 6:
				{
					long tick = MidiFileEvent_getTick(event);
					long end_tick = MidiFileEvent_getTick(end_event);
					long end_step_number = this->GetStepNumberFromTick(end_tick);
					long end_step_tick = this->step_size->GetTickFromStep(end_step_number);
					long new_end_step_tick = this->step_size->GetTickFromStep(end_step_number - 1);
					long new_end_tick = std::max<long>(new_end_step_tick + (end_tick - end_step_tick), tick);
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 7:
				{
					int end_velocity = MidiFileNoteEndEvent_getVelocity(end_event);
					int new_end_velocity = this->insertion_end_velocity = std::max<int>(end_velocity - 8, 0);
					MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileNoteEndEvent_setVelocity(end_event, end_velocity);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileNoteEndEvent_setVelocity(end_event, new_end_velocity);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_MARKER:
		{
			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long step_number = this->GetStepNumberFromTick(tick);
					long step_tick = this->step_size->GetTickFromStep(step_number);
					long new_step_tick = this->step_size->GetTickFromStep(std::max<long>(step_number - 1, 0));
					long new_tick = new_step_tick + (tick - step_tick);
					MidiFileEvent_setTick(event, new_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

void SequenceEditor::Quantize()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_t event = this->rows[this->current_row_number].event;

	switch (this->GetEventType(event))
	{
		case EVENT_TYPE_NOTE:
		{
			MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long new_tick = this->step_size->GetTickFromStep(this->GetStepNumberFromTick(tick));
					long end_tick = MidiFileEvent_getTick(end_event);
					long new_end_tick = end_tick - (tick - new_tick);
					MidiFileEvent_setTick(event, new_tick);
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				case 6:
				{
					long tick = MidiFileEvent_getTick(event);
					long end_tick = MidiFileEvent_getTick(end_event);
					long new_end_tick = this->step_size->GetTickFromStep(this->GetStepNumberFromTick(end_tick) + 1);
					MidiFileEvent_setTick(end_event, new_end_tick);
					this->sequence->RefreshData();

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(end_event, end_tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(end_event, new_end_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_MARKER:
		{
			switch (this->current_column_number)
			{
				case 1:
				{
					long tick = MidiFileEvent_getTick(event);
					long new_tick = this->step_size->GetTickFromStep(this->GetStepNumberFromTick(tick));
					MidiFileEvent_setTick(event, new_tick);
					this->sequence->RefreshData();
					this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));

					this->sequence->undo_command_processor->Submit(new UndoCommand(
						[=]{
							MidiFileEvent_setTick(event, tick);
							this->sequence->RefreshData();
						},
						NULL,
						[=]{
							MidiFileEvent_setTick(event, new_tick);
							this->sequence->RefreshData();
						},
						NULL
					));

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

void SequenceEditor::RefreshData()
{
	this->rows.clear();
	this->steps.clear();

	if (this->sequence->midi_file != NULL)
	{
		long last_step_number = -1;

		for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->sequence->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
			if (!this->Filter(event)) continue;

			long step_number = this->GetStepNumberFromTick(MidiFileEvent_getTick(event));

			while (last_step_number < step_number - 1)
			{
				last_step_number++;
				this->rows.push_back(Row(last_step_number, NULL));
				this->steps.push_back(Step(this->rows.size() - 1));
			}

			this->rows.push_back(Row(step_number, event));

			if (step_number == last_step_number)
			{
				this->steps[this->steps.size() - 1].last_row_number++;
			}
			else
			{
				this->steps.push_back(Step(this->rows.size() - 1));
			}

			last_step_number = step_number;
		}
	}

	this->event_list->RefreshData();
	this->piano_roll->RefreshData();
	this->UpdateScrollbar();
	this->sequence->RefreshDisplay();
}

void SequenceEditor::RefreshDisplay()
{
	this->window->SetStatusText(this->event_list->GetColumnLabel(this->current_row_number, this->current_column_number), 0);
	this->Refresh();
}

void SequenceEditor::OnDraw(wxDC& dc)
{
	this->piano_roll->OnDraw(dc);
	this->event_list->OnDraw(dc);
}

void SequenceEditor::UpdateScrollbar()
{
	this->SetScrollbars(0, this->event_list->row_height, 0, std::max<long>(this->rows.size(), this->last_row_number + 1), 0, this->GetScrollPos(wxVERTICAL));

	if (this->current_row_number < this->event_list->GetFirstVisibleRowNumber())
	{
		this->Scroll(wxDefaultCoord, this->current_row_number);
	}
	else if (this->current_row_number > event_list->GetLastVisibleRowNumber() - 2)
	{
		this->Scroll(wxDefaultCoord, this->current_row_number - this->GetNumberOfVisibleRows() + 1);
	}
}

long SequenceEditor::GetVisibleWidth()
{
	return this->GetClientSize().GetWidth();
}

long SequenceEditor::GetVisibleHeight()
{
	return this->GetClientSize().GetHeight();
}

long SequenceEditor::GetNumberOfVisibleRows()
{
	return this->GetVisibleHeight() / this->event_list->row_height;
}

long SequenceEditor::GetFirstVisibleY()
{
	return this->event_list->GetYFromRowNumber(this->event_list->GetFirstVisibleRowNumber());
}

long SequenceEditor::GetLastVisibleY()
{
	return this->GetFirstVisibleY() + this->GetVisibleHeight();
}

long SequenceEditor::GetFirstRowNumberFromStepNumber(long step_number)
{
	if (this->steps.size() == 0)
	{
		return step_number;
	}
	else if (step_number < this->steps.size())
	{
		return this->steps[step_number].first_row_number;
	}
	else
	{
		return this->steps[this->steps.size() - 1].last_row_number + step_number - this->steps.size() + 1;
	}
}

long SequenceEditor::GetLastRowNumberFromStepNumber(long step_number)
{
	if (this->steps.size() == 0)
	{
		return step_number;
	}
	else if (step_number < this->steps.size())
	{
		return this->steps[step_number].last_row_number;
	}
	else
	{
		return this->steps[this->steps.size() - 1].last_row_number + step_number - this->steps.size() + 1;
	}
}

long SequenceEditor::GetStepNumberFromRowNumber(long row_number)
{
	if (this->rows.size() == 0)
	{
		return row_number;
	}
	else if (row_number < this->rows.size())
	{
		return this->rows[row_number].step_number;
	}
	else
	{
		return this->rows[this->rows.size() - 1].step_number + row_number - this->rows.size() + 1;
	}
}

long SequenceEditor::GetStepNumberFromTick(long tick)
{
	return (long)(this->GetFractionalStepNumberFromTick(tick));
}

double SequenceEditor::GetFractionalStepNumberFromTick(long tick)
{
	return this->step_size->GetStepFromTick(tick);
}

long SequenceEditor::GetTickFromRowNumber(long row_number)
{
	if (row_number < this->rows.size())
	{
		MidiFileEvent_t event = this->rows[row_number].event;
		if (event != NULL) return MidiFileEvent_getTick(event);
	}

	return this->step_size->GetTickFromStep(this->GetStepNumberFromRowNumber(row_number));
}

long SequenceEditor::GetRowNumberFromTick(long tick)
{
	long step_number = this->GetStepNumberFromTick(tick);
	long first_row_number = this->GetFirstRowNumberFromStepNumber(step_number);
	long last_row_number = this->GetLastRowNumberFromStepNumber(step_number);

	if (first_row_number < this->rows.size())
	{
		for (long row_number = first_row_number; row_number <= last_row_number; row_number++)
		{
			if (MidiFileEvent_getTick(this->rows[row_number].event) >= tick) return row_number;
		}
	}

	return first_row_number;
}

long SequenceEditor::GetNumberOfTicksPerPixel(long step_number)
{
	long step_tick = this->step_size->GetTickFromStep(step_number);
	long next_step_tick = this->step_size->GetTickFromStep(step_number + 1);
	long number_of_rows = this->GetLastRowNumberFromStepNumber(step_number) - this->GetFirstRowNumberFromStepNumber(step_number) + 1;
	return std::max<long>((next_step_tick - step_tick) / (number_of_rows * this->event_list->row_height), 1);
}

long SequenceEditor::GetRowNumberForEvent(MidiFileEvent_t event)
{
	for (long i = 0; i < this->rows.size(); i++)
	{
		if (rows[i].event == event)
		{
			return i;
		}
	}

	return -1;
}

MidiFileEvent_t SequenceEditor::GetLatestTimeSignatureEventForRowNumber(long row_number)
{
	return MidiFile_getLatestTimeSignatureEventForTick(this->sequence->midi_file, this->step_size->GetTickFromStep(this->GetStepNumberFromRowNumber(row_number)));
}

bool SequenceEditor::Filter(MidiFileEvent_t event)
{
	EventType_t event_type = this->GetEventType(event);
	if (event_type == EVENT_TYPE_INVALID) return false;

	if (this->filtered_event_types.size() > 0)
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_event_types.size(); i++)
		{
			if (event_type == this->filtered_event_types[i])
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	if (this->filtered_tracks.size() > 0)
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_tracks.size(); i++)
		{
			if (this->filtered_tracks[i] == MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	if ((this->filtered_channels.size() > 0) && MidiFileEvent_isVoiceEvent(event))
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_channels.size(); i++)
		{
			if (this->filtered_channels[i] == MidiFileVoiceEvent_getChannel(event))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	return true;
}

void SequenceEditor::SetCurrentRowNumber(long current_row_number)
{
	if (current_row_number >= 0)
	{
		this->current_row_number = current_row_number;
		if (current_row_number > this->last_row_number) this->last_row_number = current_row_number;
	}

	this->UpdateScrollbar();
	this->RefreshDisplay();
}

RowLocator SequenceEditor::GetLocatorFromRowNumber(long row_number)
{
	RowLocator row_locator = RowLocator();
	row_locator.event = (row_number < this->rows.size()) ? this->rows[row_number].event : NULL;
	row_locator.tick = this->GetTickFromRowNumber(row_number);
	return row_locator;
}

long SequenceEditor::GetRowNumberFromLocator(RowLocator row_locator)
{
	if (row_locator.event == NULL)
	{
		return this->GetFirstRowNumberFromStepNumber(this->GetStepNumberFromTick(row_locator.tick));
	}
	else
	{
		long row_number = this->GetRowNumberForEvent(row_locator.event);

		if (row_number < 0)
		{
			return this->GetFirstRowNumberFromStepNumber(this->GetStepNumberFromTick(MidiFileEvent_getTick(row_locator.event)));
		}
		else
		{
			return row_number;
		}
	}
}

wxString SequenceEditor::GetEventTypeName(EventType_t event_type)
{
	switch (event_type)
	{
		case EVENT_TYPE_NOTE:
		{
			return wxString("Note");
		}
		case EVENT_TYPE_CONTROL_CHANGE:
		{
			return wxString("Control change");
		}
		case EVENT_TYPE_PROGRAM_CHANGE:
		{
			return wxString("Program change");
		}
		case EVENT_TYPE_AFTERTOUCH:
		{
			return wxString("Aftertouch");
		}
		case EVENT_TYPE_PITCH_BEND:
		{
			return wxString("Pitch bend");
		}
		case EVENT_TYPE_SYSTEM_EXCLUSIVE:
		{
			return wxString("System exclusive");
		}
		case EVENT_TYPE_TEXT:
		{
			return wxString("Text");
		}
		case EVENT_TYPE_LYRIC:
		{
			return wxString("Lyric");
		}
		case EVENT_TYPE_MARKER:
		{
			return wxString("Marker");
		}
		case EVENT_TYPE_PORT:
		{
			return wxString("Port");
		}
		case EVENT_TYPE_TEMPO:
		{
			return wxString("Tempo");
		}
		case EVENT_TYPE_TIME_SIGNATURE:
		{
			return wxString("Time signature");
		}
		case EVENT_TYPE_KEY_SIGNATURE:
		{
			return wxString("Key signature");
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

EventType_t SequenceEditor::GetEventType(MidiFileEvent_t event)
{
	if (MidiFileEvent_isNoteStartEvent(event)) return EVENT_TYPE_NOTE;
	if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE) return EVENT_TYPE_CONTROL_CHANGE;
	if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE) return EVENT_TYPE_PROGRAM_CHANGE;
	if ((MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE) || (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE)) return EVENT_TYPE_AFTERTOUCH;
	if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PITCH_WHEEL) return EVENT_TYPE_PITCH_BEND;
	if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_SYSEX) return EVENT_TYPE_SYSTEM_EXCLUSIVE;
	if (MidiFileEvent_isTextEvent(event)) return EVENT_TYPE_TEXT;
	if (MidiFileEvent_isLyricEvent(event)) return EVENT_TYPE_LYRIC;
	if (MidiFileEvent_isMarkerEvent(event)) return EVENT_TYPE_MARKER;
	if (MidiFileEvent_isPortEvent(event)) return EVENT_TYPE_PORT;
	if (MidiFileEvent_isTempoEvent(event)) return EVENT_TYPE_TEMPO;
	if (MidiFileEvent_isTimeSignatureEvent(event)) return EVENT_TYPE_TIME_SIGNATURE;
	if (MidiFileEvent_isKeySignatureEvent(event)) return EVENT_TYPE_KEY_SIGNATURE;
	return EVENT_TYPE_INVALID;
}

Sequence::Sequence()
{
	this->undo_command_processor = new wxCommandProcessor();
	this->filename = wxEmptyString;
	this->midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	this->is_modified = false;
}

Sequence::~Sequence()
{
	MidiFile_free(this->midi_file);
	delete this->undo_command_processor;
}

void Sequence::AddSequenceEditor(SequenceEditor* sequence_editor)
{
	this->sequence_editors.push_back(sequence_editor);
}

void Sequence::RemoveSequenceEditor(SequenceEditor* sequence_editor)
{
	this->sequence_editors.remove(sequence_editor);
	if (this->sequence_editors.empty()) delete this;
}

void Sequence::RefreshData()
{
	this->is_modified = true;
	for (std::list<SequenceEditor*>::iterator sequence_editor_iterator = this->sequence_editors.begin(); sequence_editor_iterator != this->sequence_editors.end(); sequence_editor_iterator++) (*sequence_editor_iterator)->RefreshData();
}

void Sequence::RefreshDisplay()
{
	for (std::list<SequenceEditor*>::iterator sequence_editor_iterator = this->sequence_editors.begin(); sequence_editor_iterator != this->sequence_editors.end(); sequence_editor_iterator++) (*sequence_editor_iterator)->RefreshDisplay();
}

Row::Row(long step_number, MidiFileEvent_t event)
{
	this->step_number = step_number;
	this->event = event;
}

Step::Step(long row_number)
{
	this->first_row_number = row_number;
	this->last_row_number = row_number;
}

UndoCommand::UndoCommand(std::function<void ()> undo_callback, std::function<void ()> cleanup_when_undone_callback, std::function<void ()> redo_callback, std::function<void ()> cleanup_when_done_callback): wxCommand(true)
{
	this->undo_callback = undo_callback;
	this->cleanup_when_undone_callback = cleanup_when_undone_callback;
	this->redo_callback = redo_callback;
	this->cleanup_when_done_callback = cleanup_when_done_callback;
	this->has_been_undone = false;
}

UndoCommand::~UndoCommand()
{
	if (this->has_been_undone)
	{
		if (this->cleanup_when_undone_callback != NULL) this->cleanup_when_undone_callback();
	}
	else
	{
		if (this->cleanup_when_done_callback != NULL) this->cleanup_when_done_callback();
	}
}

bool UndoCommand::Do()
{
	if (this->has_been_undone)
	{
		if (this->redo_callback != NULL) this->redo_callback();
		this->has_been_undone = false;
	}

	return true;
}

bool UndoCommand::Undo()
{
	if (this->undo_callback != NULL) this->undo_callback();
	this->has_been_undone = true;
	return true;
}

