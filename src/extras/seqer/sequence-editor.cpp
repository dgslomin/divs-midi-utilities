
#include <math.h>
#include <algorithm>
#include <vector>
#include <wx/wx.h>
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

SequenceEditor::SequenceEditor(Window* window): wxScrolledCanvas(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | CANVAS_BORDER)
{
	this->window = window;
	this->sequence = new Sequence(this);
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
#ifndef __WXOSX__
	this->SetDoubleBuffered(true);
#endif
	this->DisableKeyboardScrolling();
	this->SetBackgroundColour(*wxWHITE);
	this->RefreshData();
}

SequenceEditor::~SequenceEditor()
{
	delete this->sequence;
	delete this->event_list;
	delete this->piano_roll;
	delete this->step_size;
}

void SequenceEditor::New()
{
	MidiFile_free(this->sequence->midi_file);
	this->sequence->midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	this->current_row_number = 0;
	this->last_row_number = 0;
	this->current_column_number = 0;
	this->RefreshData();
}

bool SequenceEditor::Load(wxString filename)
{
	MidiFile_t new_midi_file = MidiFile_load((char *)(filename.ToStdString().c_str()));
	if (new_midi_file == NULL) return false;
	MidiFile_free(this->sequence->midi_file);
	this->sequence->midi_file = new_midi_file;
	this->current_row_number = 0;
	this->last_row_number = 0;
	this->current_column_number = 0;
	this->SetStepSize(new StepsPerMeasureSize(this));
	this->RefreshData();
	return true;
}

void SequenceEditor::SetStepSize(StepSize* step_size)
{
	RowLocator row_locator = this->GetLocatorFromRowNumber(this->current_row_number);
	delete this->step_size;
	this->step_size = step_size;
	this->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberFromLocator(row_locator));
	this->Refresh();
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
	this->Refresh();
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
	this->Refresh();
}

void SequenceEditor::ColumnRight()
{
	this->current_column_number = std::min<long>(this->current_column_number + 1, 7);
	this->Refresh();
}

void SequenceEditor::GoToColumn(int column_number)
{
	this->current_column_number = std::min<long>(std::max<long>(column_number, 1), 7);
	this->Refresh();
}

void SequenceEditor::GoToNextMarker()
{
	// TODO
}

void SequenceEditor::GoToPreviousMarker()
{
	// TODO
}

void SequenceEditor::GoToMarker(wxString marker_name)
{
	// TODO
}

void SequenceEditor::InsertNote(int diatonic)
{
	MidiFileTrack_t track = MidiFile_getTrackByNumber(this->sequence->midi_file, this->insertion_track_number, 1);
	long start_step_number = this->GetStepNumberFromRowNumber(this->current_row_number);
	long start_tick = this->step_size->GetTickFromStep(start_step_number);
	long end_tick = this->step_size->GetTickFromStep(start_step_number + 1);
	int chromatic = GetChromaticFromDiatonicInKey(diatonic, MidiFileKeySignatureEvent_getNumber(MidiFile_getLatestKeySignatureEventForTick(this->sequence->midi_file, start_tick)));
	this->insertion_note_number = MatchNoteOctave(SetNoteChromatic(this->insertion_note_number, chromatic), this->insertion_note_number);
	MidiFileEvent_t event = MidiFileTrack_createNoteStartAndEndEvents(track, start_tick, end_tick, this->insertion_channel_number, this->insertion_note_number, this->insertion_velocity, 0);
	this->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));
}

void SequenceEditor::DeleteRow()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_delete(this->rows[this->current_row_number].event);
	this->RefreshData();
}

void SequenceEditor::SmallIncrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	MidiFileEvent_t event = this->rows[this->current_row_number].event;

	switch (this->GetEventType(event))
	{
		case EVENT_TYPE_NOTE:
		{
			switch (current_column_number)
			{
				case 3:
				{
					MidiFileNoteStartEvent_setChannel(event, std::min<int>(MidiFileNoteStartEvent_getChannel(event) + 1, 15));
					this->RefreshData();
					break;
				}
				case 4:
				{
					MidiFileNoteStartEvent_setNote(event, std::min<int>(MidiFileNoteStartEvent_getNote(event) + 1, 127));
					this->RefreshData();
					break;
				}
				case 5:
				{
					MidiFileNoteStartEvent_setVelocity(event, std::min<int>(MidiFileNoteStartEvent_getVelocity(event) + 1, 127));
					this->RefreshData();
					break;
				}
				case 7:
				{
					MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
					MidiFileNoteEndEvent_setVelocity(end_event, std::min<int>(MidiFileNoteEndEvent_getVelocity(end_event) + 1, 127));
					this->RefreshData();
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_CONTROL_CHANGE:
		{
			break;
		}
		case EVENT_TYPE_PROGRAM_CHANGE:
		{
			break;
		}
		case EVENT_TYPE_AFTERTOUCH:
		{
			break;
		}
		case EVENT_TYPE_PITCH_BEND:
		{
			break;
		}
		case EVENT_TYPE_SYSTEM_EXCLUSIVE:
		{
			break;
		}
		case EVENT_TYPE_TEXT:
		{
			break;
		}
		case EVENT_TYPE_LYRIC:
		{
			break;
		}
		case EVENT_TYPE_MARKER:
		{
			break;
		}
		case EVENT_TYPE_PORT:
		{
			break;
		}
		case EVENT_TYPE_TEMPO:
		{
			break;
		}
		case EVENT_TYPE_TIME_SIGNATURE:
		{
			break;
		}
		case EVENT_TYPE_KEY_SIGNATURE:
		{
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
			switch (current_column_number)
			{
				case 3:
				{
					MidiFileNoteStartEvent_setChannel(event, std::max<int>(MidiFileNoteStartEvent_getChannel(event) - 1, 0));
					this->RefreshData();
					break;
				}
				case 4:
				{
					MidiFileNoteStartEvent_setNote(event, std::max<int>(MidiFileNoteStartEvent_getNote(event) - 1, 0));
					this->RefreshData();
					break;
				}
				case 5:
				{
					MidiFileNoteStartEvent_setVelocity(event, std::max<int>(MidiFileNoteStartEvent_getVelocity(event) - 1, 1));
					this->RefreshData();
					break;
				}
				case 7:
				{
					MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
					MidiFileNoteEndEvent_setVelocity(end_event, std::max<int>(MidiFileNoteEndEvent_getVelocity(end_event) - 1, 0));
					this->RefreshData();
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_CONTROL_CHANGE:
		{
			break;
		}
		case EVENT_TYPE_PROGRAM_CHANGE:
		{
			break;
		}
		case EVENT_TYPE_AFTERTOUCH:
		{
			break;
		}
		case EVENT_TYPE_PITCH_BEND:
		{
			break;
		}
		case EVENT_TYPE_SYSTEM_EXCLUSIVE:
		{
			break;
		}
		case EVENT_TYPE_TEXT:
		{
			break;
		}
		case EVENT_TYPE_LYRIC:
		{
			break;
		}
		case EVENT_TYPE_MARKER:
		{
			break;
		}
		case EVENT_TYPE_PORT:
		{
			break;
		}
		case EVENT_TYPE_TEMPO:
		{
			break;
		}
		case EVENT_TYPE_TIME_SIGNATURE:
		{
			break;
		}
		case EVENT_TYPE_KEY_SIGNATURE:
		{
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
			switch (current_column_number)
			{
				case 3:
				{
					MidiFileNoteStartEvent_setChannel(event, std::min<int>(MidiFileNoteStartEvent_getChannel(event) + 1, 15));
					this->RefreshData();
					break;
				}
				case 4:
				{
					MidiFileNoteStartEvent_setNote(event, std::min<int>(MidiFileNoteStartEvent_getNote(event) + 12, 127));
					this->RefreshData();
					break;
				}
				case 5:
				{
					MidiFileNoteStartEvent_setVelocity(event, std::min<int>(MidiFileNoteStartEvent_getVelocity(event) + 8, 127));
					this->RefreshData();
					break;
				}
				case 7:
				{
					MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
					MidiFileNoteEndEvent_setVelocity(end_event, std::min<int>(MidiFileNoteEndEvent_getVelocity(end_event) + 8, 127));
					this->RefreshData();
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_CONTROL_CHANGE:
		{
			break;
		}
		case EVENT_TYPE_PROGRAM_CHANGE:
		{
			break;
		}
		case EVENT_TYPE_AFTERTOUCH:
		{
			break;
		}
		case EVENT_TYPE_PITCH_BEND:
		{
			break;
		}
		case EVENT_TYPE_SYSTEM_EXCLUSIVE:
		{
			break;
		}
		case EVENT_TYPE_TEXT:
		{
			break;
		}
		case EVENT_TYPE_LYRIC:
		{
			break;
		}
		case EVENT_TYPE_MARKER:
		{
			break;
		}
		case EVENT_TYPE_PORT:
		{
			break;
		}
		case EVENT_TYPE_TEMPO:
		{
			break;
		}
		case EVENT_TYPE_TIME_SIGNATURE:
		{
			break;
		}
		case EVENT_TYPE_KEY_SIGNATURE:
		{
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
			switch (current_column_number)
			{
				case 3:
				{
					MidiFileNoteStartEvent_setChannel(event, std::max<int>(MidiFileNoteStartEvent_getChannel(event) - 1, 0));
					this->RefreshData();
					break;
				}
				case 4:
				{
					MidiFileNoteStartEvent_setNote(event, std::max<int>(MidiFileNoteStartEvent_getNote(event) - 12, 0));
					this->RefreshData();
					break;
				}
				case 5:
				{
					MidiFileNoteStartEvent_setVelocity(event, std::max<int>(MidiFileNoteStartEvent_getVelocity(event) - 8, 1));
					this->RefreshData();
					break;
				}
				case 7:
				{
					MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
					MidiFileNoteEndEvent_setVelocity(end_event, std::max<int>(MidiFileNoteEndEvent_getVelocity(end_event) - 8, 0));
					this->RefreshData();
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case EVENT_TYPE_CONTROL_CHANGE:
		{
			break;
		}
		case EVENT_TYPE_PROGRAM_CHANGE:
		{
			break;
		}
		case EVENT_TYPE_AFTERTOUCH:
		{
			break;
		}
		case EVENT_TYPE_PITCH_BEND:
		{
			break;
		}
		case EVENT_TYPE_SYSTEM_EXCLUSIVE:
		{
			break;
		}
		case EVENT_TYPE_TEXT:
		{
			break;
		}
		case EVENT_TYPE_LYRIC:
		{
			break;
		}
		case EVENT_TYPE_MARKER:
		{
			break;
		}
		case EVENT_TYPE_PORT:
		{
			break;
		}
		case EVENT_TYPE_TEMPO:
		{
			break;
		}
		case EVENT_TYPE_TIME_SIGNATURE:
		{
			break;
		}
		case EVENT_TYPE_KEY_SIGNATURE:
		{
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
	this->Refresh();
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

Sequence::Sequence(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
}

Sequence::~Sequence()
{
	MidiFile_free(this->midi_file);
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

