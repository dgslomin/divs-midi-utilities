
#include <wx/wx.h>
#include "label.h"
#include "label-lane.h"
#include "marker-lane.h"
#include "midifile.h"

MarkerLane::MarkerLane(Window* window): LabelLane(window)
{
}

MarkerLane::~MarkerLane()
{
}

void MarkerLane::PopulateLabels()
{
	this->labels.clear();

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isMarkerEvent(midi_event)) this->labels.push_back(Label(midi_event, wxString(MidiFileMarkerEvent_getText(midi_event))));
	}
}
