#ifndef EVENT_LIST_INCLUDED
#define EVENT_LIST_INCLUDED

#define EVENT_LIST_NUMBER_OF_COLUMNS 8

class EventList;

#include <wx/wx.h>
#include "sequence-editor.h"

class EventList
{
public:
	SequenceEditor *sequence_editor;
	wxFont font;
	wxColour current_cell_border_color;
	long row_height;
	long column_widths[EVENT_LIST_NUMBER_OF_COLUMNS];

	EventList(SequenceEditor* sequence_editor);
	void RefreshData();
	void OnDraw(wxDC& dc);
	long GetVisibleWidth();
	long GetFirstVisibleRowNumber();
	long GetLastVisibleRowNumber();
	long GetLastVisiblePopulatedRowNumber();
	long GetColumnWidth(long column_number);
	long GetXFromColumnNumber(long column_number);
	long GetYFromRowNumber(long row_number);
	long GetRowNumberFromY(long y);
};

#endif
