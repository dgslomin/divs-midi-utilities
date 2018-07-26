#ifndef TEXT_EVENT_INCLUDED
#define TEXT_EVENT_INCLUDED

class TextEventType;
class TextEventTimeCell;
class TextEventTrackCell;
class TextEventTextCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class TextEventType: public EventType
{
public:
	static TextEventType* GetInstance();

private:
	TextEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class TextEventTimeCell: public Cell
{
public:
	TextEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class TextEventTrackCell: public Cell
{
public:
	TextEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class TextEventTextCell: public Cell
{
public:
	TextEventTextCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif
