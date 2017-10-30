#ifndef KEY_SIGNATURE_EVENT_INCLUDED
#define KEY_SIGNATURE_EVENT_INCLUDED

class KeySignatureEventType;
class KeySignatureEventRow;
class KeySignatureEventTimeCell;
class KeySignatureEventKeySignatureCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class KeySignatureEventType: public EventType
{
public:
	static KeySignatureEventType* GetInstance();

private:
	KeySignatureEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
	Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class KeySignatureEventRow: public Row
{
public:
	KeySignatureEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class KeySignatureEventTimeCell: public Cell
{
public:
	KeySignatureEventTimeCell(Row* row);
	wxString GetValueText();
};

class KeySignatureEventKeySignatureCell: public Cell
{
public:
	KeySignatureEventKeySignatureCell(Row* row);
	wxString GetValueText();
};

#endif
