
#include "controller-lane.h"
#include "midifile.h"
#include "numeric-value-lane.h"
#include "window.h"

ControllerLane::ControllerLane(Window* window): NumericValueLane(window)
{
	this->draw_connecting_lines = true;
}

void ControllerLane::paintValueLines(QPainter* painter)
{
	painter->setPen(this->grid_line_pen);
	int min_y = this->getYFromValue(0) + (this->handle_size / 2);
	int max_y = this->getYFromValue(127) + (this->handle_size / 2);
	painter->drawLine(0, min_y, this->width(), min_y);
	painter->drawLine(0, max_y, this->width(), max_y);
}

bool ControllerLane::shouldIncludeEvent(MidiFileEvent_t midi_event)
{
	return (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE) && (MidiFileControlChangeEvent_getNumber(midi_event) == this->controller_number);
}

MidiFileEvent_t ControllerLane::addEvent(long tick, float value)
{
	return MidiFileTrack_createControlChangeEvent(MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 1), tick, this->channel, this->controller_number, (int)(value));
}

float ControllerLane::getEventValue(MidiFileEvent_t midi_event)
{
	return (float)(MidiFileControlChangeEvent_getValue(midi_event));
}

void ControllerLane::setEventValue(MidiFileEvent_t midi_event, float value)
{
	MidiFileControlChangeEvent_setValue(midi_event, (int)(value));
}

