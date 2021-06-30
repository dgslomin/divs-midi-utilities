
#include <wx/wx.h>
#include "midifile.h"
#include "numeric-value-lane.h"
#include "tempo-lane.h"
#include "window.h"

TempoLane::TempoLane(wxWindow* parent, Window* window): NumericValueLane(parent, window)
{
}

TempoLane::~TempoLane()
{
}

bool TempoLane::ShouldIncludeEvent(MidiFileEvent_t midi_event)
{
	return MidiFileEvent_isTempoEvent(midi_event);
}

MidiFileEvent_t TempoLane::AddEvent(long tick, float value)
{
	return MidiFileTrack_createTempoEvent(MidiFile_getTrackByNumber(this->window->sequence->midi_file, 0, 1), tick, value);
}

float TempoLane::GetEventValue(MidiFileEvent_t midi_event)
{
	return MidiFileTempoEvent_getTempo(midi_event);
}

void TempoLane::SetEventValue(MidiFileEvent_t midi_event, float value)
{
	MidiFileTempoEvent_setTempo(midi_event, value);
}

