
#include <QPainter>
#include <QPoint>
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
	MidiFileTrack_t current_track = MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 0);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE)
		{
			QRect rect = this->getRectFromEvent(midi_event, selected_events_x_offset, selected_events_y_offset);

			if (rect.intersects(bounds))
			{
				bool is_background = (MidiFileEvent_getTrack(midi_event) != current_track);
				bool is_selected = MidiFileEvent_isSelected(midi_event);
				painter->setPen(is_background ? (is_selected ? this->selected_background_event_pen : this->unselected_background_event_pen) : (is_selected ? this->selected_event_pen : this->unselected_event_pen));
				painter->setBrush(is_background ? (is_selected ? this->selected_background_event_brush : this->unselected_background_event_brush) : (is_selected ? this->selected_event_brush : this->unselected_event_brush));
				painter->drawRect(rect);
			}
		}
	}
}

MidiFileEvent_t NoteLane::getEventFromXY(int x, int y)
{
	for (MidiFileEvent_t midi_event = MidiFile_getLastEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE)
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
	int duration_ticks = MidiFile_getTickFromBeat(this->window->sequence->midi_file, MidiFile_getBeatFromTick(this->window->sequence->midi_file, start_tick) + 1) - start_tick;
	int note = this->getNoteFromY(y);
	return MidiFileTrack_createNoteEvent(MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 1), start_tick, duration_ticks, this->channel, note, this->velocity, 0);
}

void NoteLane::moveEventsByXY(int x_offset, int y_offset)
{
	for (MidiFileEvent_t midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file))
	{
		if ((MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE) && MidiFileEvent_isSelected(midi_event))
		{
			if (x_offset != 0) MidiFileEvent_setTick(midi_event, this->window->getTickFromX(this->window->getXFromTick(MidiFileEvent_getTick(midi_event)) + x_offset));
			if (y_offset != 0) MidiFileNoteEvent_setNote(midi_event, this->getNoteFromY(this->getYFromNote(MidiFileNoteEvent_getNote(midi_event)) + y_offset));
		}
	}
}

void NoteLane::selectEventsInRect(int x, int y, int width, int height)
{
	QRect bounds(x, y, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE)
		{
			QRect rect = this->getRectFromEvent(midi_event, 0, 0);
			if (rect.intersects(bounds)) MidiFileEvent_setSelected(midi_event, 1);
		}
	}
}

void NoteLane::scrollYBy(int offset)
{
	this->scroll_y += offset;
	this->cursor_y += offset;
	this->update();
}

void NoteLane::zoomYBy(float factor)
{
	int cursor_note = this->getNoteFromY(this->cursor_y);
	this->pixels_per_note *= factor;
	this->cursor_y = this->getYFromNote(cursor_note);
	this->update();
}

QRect NoteLane::getRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset)
{
	int x = this->window->getXFromTick(MidiFileEvent_getTick(midi_event));
	int width = this->window->getXFromTick(MidiFileEvent_getTick(midi_event) + MidiFileNoteEvent_getDurationTicks(midi_event)) - x;
	int y = this->getYFromNote(MidiFileNoteEvent_getNote(midi_event));

	if (MidiFileEvent_isSelected(midi_event))
	{
		x += selected_events_x_offset;
		y += selected_events_y_offset;
	}

	return QRect(x, y, width, this->pixels_per_note);
}

int NoteLane::getYFromNote(int note)
{
	return this->height() - ((note + 1) * this->pixels_per_note) + this->scroll_y;
}

int NoteLane::getNoteFromY(int y)
{
	return (this->height() - y - 1 + this->scroll_y) / this->pixels_per_note;
}

