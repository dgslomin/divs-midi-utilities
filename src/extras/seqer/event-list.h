#ifndef EVENT_LIST_INCLUDED
#define EVENT_LIST_INCLUDED

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
	long column_widths[8];

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
	wxString GetCellText(long row_number, long column_number);
	wxString GetColumnLabel(long row_number, long column_number);
};

#endif
