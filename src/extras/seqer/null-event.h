#ifndef NULL_EVENT_INCLUDED
#define NULL_EVENT_INCLUDED

class NullEventType;
class NullEventTimeCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class NullEventType: public EventType
{
public:
	static NullEventType* GetInstance();

private:
	NullEventType();

public:
    void Delete(SequenceEditor* sequence_editor, Row* row);
};

class NullEventTimeCell: public Cell
{
public:
	NullEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif
