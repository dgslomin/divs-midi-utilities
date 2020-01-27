#ifndef NOTE_LANE_INCLUDED
#define NOTE_LANE_INCLUDED

class ControlChaneLane;

#include <wx/wx.h>
#include "lane.h"
#include "midifile.h"
#include "window.h"

class NoteLane: public Lane
{
public:
	MidiFileTrack_t track;
	int channel;
	int velocity;
	int pixels_per_note = 4;
	int scroll_y;

	NoteLane(wxWindow* parent, Window* window);
	~NoteLane();
	virtual void PaintBackground(wxDC& dc, int width, int height);
	virtual void PaintEvents(wxDC& dc, int width, int height, int selected_events_x_offset, int selected_events_y_offset);
	virtual MidiFileEvent_t GetEventFromXY(int x, int y);
	virtual MidiFileEvent_t AddEventAtXY(int x, int y);
	virtual void MoveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset);
	virtual void SelectEventsInRect(int x, int y, int width, int height);
	virtual wxRect GetRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset);
	virtual int GetYFromNote(int note);
	virtual int GetNoteFromY(int y);
};

#endif

