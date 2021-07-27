
#include <QtWidgets>
#include "marker-lane.h"
#include "label-lane.h"
#include "lane.h"
#include "midifile.h"
#include "window.h"

MarkerLane::MarkerLane(Window* window): LabelLane(window, Lane::MARKER_LANE_TYPE)
{
}

void MarkerLane::populateLabels()
{
	this->labels.clear();

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isMarkerEvent(midi_event)) this->labels.append(Label(midi_event, MidiFileMarkerEvent_getText(midi_event)));
	}
}

MidiFileEvent_t MarkerLane::addEventAtXY(int x, int y)
{
	Q_UNUSED(y)
	return MidiFileTrack_createMarkerEvent(MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 1), this->window->getTickFromX(x), (char *)("marker"));
}

void MarkerLane::moveEventsByXY(int x_offset, int y_offset)
{
	Q_UNUSED(y_offset)
	if (x_offset == 0) return;

	for (MidiFileEvent_t midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file))
	{
		if (MidiFileEvent_isMarkerEvent(midi_event) && MidiFileEvent_isSelected(midi_event)) MidiFileEvent_setTick(midi_event, this->window->getTickFromX(this->window->getXFromTick(MidiFileEvent_getTick(midi_event)) + x_offset));
	}
}

