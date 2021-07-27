
#include <QtWidgets>
#include "continuous-value-lane.h"
#include "lane.h"
#include "midifile.h"
#include "tempo-lane.h"
#include "window.h"

TempoLane::TempoLane(Window* window): ContinuousValueLane(window, Lane::TEMPO_LANE_TYPE)
{
	this->track_number = 0;
}

bool TempoLane::shouldIncludeEvent(MidiFileEvent_t midi_event)
{
	return MidiFileEvent_isTempoEvent(midi_event);
}

MidiFileEvent_t TempoLane::addEvent(long tick, float value)
{
	return MidiFileTrack_createTempoEvent(MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 1), tick, value);
}

float TempoLane::getEventValue(MidiFileEvent_t midi_event)
{
	return MidiFileTempoEvent_getTempo(midi_event);
}

void TempoLane::setEventValue(MidiFileEvent_t midi_event, float value)
{
	MidiFileTempoEvent_setTempo(midi_event, value);
}

