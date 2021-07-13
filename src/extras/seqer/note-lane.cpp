
#include <QBrush>
#include <QPainter>
#include <QRect>
#include <QSettings>
#include "lane.h"
#include "midifile.h"
#include "note-lane.h"
#include "window.h"

NoteLane::NoteLane(Window* window): Lane(window)
{
	QSettings settings;
	this->pixels_per_note = settings.value("note-lane/pixels-per-note", 6).toInt();
	this->scroll_y = settings.value("note-lane/scroll-y", 0).toInt();
}

void NoteLane::paintBackground(QPainter* painter)
{
	int width = this->width();
	int height = this->height();
	painter->fillRect(0, 0, width, height, this->background_color);
	bool black_note[] = { false, true, false, true, false, false, true, false, true, false, true, false };

	for (int note = 0; note < 128; note++)
	{
		painter->fillRect(0, this->getYFromNote(note), width, this->pixels_per_note, black_note[note % 12] ? this->black_note_background_color : this->white_note_background_color);
	}
}

void NoteLane::paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset)
{
	QRect bounds(0, 0, this->width(), this->height());

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			QRect rect = this->getRectFromEvent(midi_event, selected_events_x_offset, selected_events_y_offset);

			if (rect.intersects(bounds))
			{
				if (MidiFileEvent_isSelected(midi_event))
				{
					painter->setPen(this->selected_event_pen);
					painter->setBrush(this->selected_event_brush);
				}
				else
				{
					painter->setPen(this->unselected_event_pen);
					painter->setBrush(this->unselected_event_brush);
				}

				painter->drawRect(rect);
			}
		}
	}
}

MidiFileEvent_t NoteLane::getEventFromXY(int x, int y)
{
	for (MidiFileEvent_t midi_event = MidiFile_getLastEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			QRect rect = this->getRectFromEvent(midi_event, 0, 0);
			if (rect.contains(x, y, false)) return midi_event;
		}
	}

	return NULL;
}

QPoint NoteLane::getPointFromEvent(MidiFileEvent_t midi_event)
{
	return this->getRectFromEvent(midi_event, 0, 0).topLeft();
}

MidiFileEvent_t NoteLane::addEventAtXY(int x, int y)
{
	int start_tick = this->window->getTickFromX(x);
	int end_tick = MidiFile_getTickFromBeat(this->window->sequence->midi_file, MidiFile_getBeatFromTick(this->window->sequence->midi_file, start_tick) + 1);
	int note = this->getNoteFromY(y);
	return MidiFileTrack_createNoteStartAndEndEvents(MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 1), start_tick, end_tick, this->channel, note, this->velocity, 0);
}

void NoteLane::moveEventsByXY(int x_offset, int y_offset)
{
	// We make a copy because changing both start and end events while iterating destabilizes the iterator (even with visitEvents()).
	MidiFile_t new_midi_file = MidiFile_newFromTemplate(this->window->sequence->midi_file);

	for (MidiFileTrack_t track = MidiFile_getFirstTrack(this->window->sequence->midi_file); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		MidiFileTrack_t new_track = MidiFile_createTrack(new_midi_file);

		for (MidiFileEvent_t midi_event = MidiFileTrack_getFirstEvent(track); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInTrack(midi_event))
		{
			if (MidiFileEvent_isNoteStartEvent(midi_event))
			{
				MidiFileEvent_t new_midi_event = MidiFileTrack_copyEvent(new_track, midi_event);
				MidiFileEvent_t new_note_end_midi_event = MidiFileTrack_copyEvent(new_track, MidiFileNoteStartEvent_getNoteEndEvent(midi_event));

				if (MidiFileEvent_isSelected(new_midi_event))
				{
					int start_x = this->window->getXFromTick(MidiFileEvent_getTick(new_midi_event));
					int end_x = this->window->getXFromTick(MidiFileEvent_getTick(new_note_end_midi_event));
					int y = this->getYFromNote(MidiFileNoteStartEvent_getNote(new_midi_event));

					MidiFileEvent_setTick(new_midi_event, this->window->getTickFromX(start_x + x_offset));
					MidiFileEvent_setTick(new_note_end_midi_event, this->window->getTickFromX(end_x + x_offset));
					MidiFileNoteStartEvent_setNote(new_midi_event, this->getNoteFromY(y + y_offset));
				}
			}
			else if (MidiFileNoteEndEvent_getNoteStartEvent(midi_event) != NULL)
			{
				// These are handled as part of the note start logic
			}
			else
			{
				MidiFileTrack_copyEvent(new_track, midi_event);
			}
		}
	}

	MidiFile_free(this->window->sequence->midi_file);
	this->window->sequence->midi_file = new_midi_file;
}

void NoteLane::selectEventsInRect(int x, int y, int width, int height)
{
	QRect bounds(x, y, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			QRect rect = this->getRectFromEvent(midi_event, 0, 0);

			if (rect.intersects(bounds))
			{
				MidiFileEvent_setSelected(midi_event, 1);
			}
		}
	}
}

void NoteLane::scrollYBy(int y_offset)
{
	this->scroll_y += y_offset;
}

void NoteLane::zoomYBy(int y_offset)
{
	this->pixels_per_note = std::max(this->pixels_per_note - y_offset, 1);
}

QRect NoteLane::getRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset)
{
	int start_x = this->window->getXFromTick(MidiFileEvent_getTick(midi_event));
	int end_x = this->window->getXFromTick(MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(midi_event)));
	int y = this->getYFromNote(MidiFileNoteStartEvent_getNote(midi_event));

	if (MidiFileEvent_isSelected(midi_event))
	{
		start_x += selected_events_x_offset;
		end_x += selected_events_x_offset;
		y += selected_events_y_offset;
	}

	return QRect(start_x, y, end_x - start_x, this->pixels_per_note);
}

int NoteLane::getYFromNote(int note)
{
	return this->height() - ((note + 1) * this->pixels_per_note) + this->scroll_y;
}

int NoteLane::getNoteFromY(int y)
{
	return (this->height() - y - 1 - this->scroll_y) / this->pixels_per_note;
}
