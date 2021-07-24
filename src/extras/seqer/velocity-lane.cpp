
#include <QtWidgets>
#include "midifile.h"
#include "numeric-value-lane.h"
#include "velocity-lane.h"
#include "window.h"

VelocityLane::VelocityLane(Window* window): NumericValueLane(window)
{
	this->draw_as_boxes = true;
}

bool VelocityLane::shouldIncludeEvent(MidiFileEvent_t midi_event)
{
	return (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE);
}

MidiFileEvent_t VelocityLane::addEvent(long tick, float value)
{
	// not supported
	Q_UNUSED(tick)
	Q_UNUSED(value)
	return NULL;
}

float VelocityLane::getEventValue(MidiFileEvent_t midi_event)
{
	return (float)(MidiFileNoteEvent_getVelocity(midi_event));
}

void VelocityLane::setEventValue(MidiFileEvent_t midi_event, float value)
{
	MidiFileNoteEvent_setVelocity(midi_event, (int)(value));
}

