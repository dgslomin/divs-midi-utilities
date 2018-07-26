#ifndef KEY_SIGNATURE_EVENT_INCLUDED
#define KEY_SIGNATURE_EVENT_INCLUDED

class KeySignatureEventType;
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
};

class KeySignatureEventTimeCell: public Cell
{
public:
	KeySignatureEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class KeySignatureEventKeySignatureCell: public Cell
{
public:
	KeySignatureEventKeySignatureCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif
