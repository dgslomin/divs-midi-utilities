#ifndef PIANO_ROLL_INCLUDED
#define PIANO_ROLL_INCLUDED

class PianoRoll;

#include <wx/wx.h>
#include "sequence-editor.h"

class PianoRoll
{
public:
	SequenceEditor *sequence_editor;
	long first_note;
	long last_note;
	long key_width;
	wxColour darker_line_color;
	wxColour lighter_line_color;
	wxColour lightest_line_color;
	wxColour white_key_color;
	wxColour black_key_color;
	wxColour shadow_color;

	PianoRoll(SequenceEditor* sequence_editor);
	void RefreshData();
	void OnDraw(wxDC& dc);
	long GetWidth();
	long GetYFromStepNumber(double step_number);
};

#endif
