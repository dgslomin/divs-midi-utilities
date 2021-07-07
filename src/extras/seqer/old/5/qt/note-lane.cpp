
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

	for (int note = this->getNoteFromY(height), last_note = this->getNoteFromY(0); note < last_note; note++)
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
			if (rect.contains(x, y)) return midi_event;
		}
	}

	return NULL;
}

MidiFileEvent_t NoteLane::addEventAtXY(int x, int y)
{
	int start_tick = this->window->getTickFromX(x);
	int end_tick = MidiFile_getTickFromBeat(this->window->sequence->midi_file, MidiFile_getBeatFromTick(this->window->sequence->midi_file, start_tick) + 1);
	int note = this->getNoteFromY(y);
	return MidiFileTrack_createNoteStartAndEndEvents(this->track, start_tick, end_tick, this->channel, note, this->velocity, 0);
}

void NoteLane::moveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset)
{
	MidiFileEvent_t note_end_midi_event = MidiFileNoteStartEvent_getNoteEndEvent(midi_event);
	MidiFileEvent_setTick(midi_event, this->window->getTickFromX(this->window->getXFromTick(MidiFileEvent_getTick(midi_event)) + x_offset));
	MidiFileEvent_setTick(note_end_midi_event, this->window->getTickFromX(this->window->getXFromTick(MidiFileEvent_getTick(note_end_midi_event)) + x_offset));
	MidiFileNoteStartEvent_setNote(midi_event, this->getNoteFromY(this->getYFromNote(MidiFileNoteStartEvent_getNote(midi_event)) + y_offset));
}

void NoteLane::selectEventsInRect(int x, int y, int width, int height)
{
	QRect bounds(x, height - y, width, height);

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
	return this->height() - ((note * this->pixels_per_note) - this->scroll_y);
}

int NoteLane::getNoteFromY(int y)
{
	return (this->height() - (y + this->scroll_y)) / this->pixels_per_note;
}

