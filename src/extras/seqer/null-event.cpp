
#include <wx/wx.h>
#include <midifile.h>
#include "null-event.h"
#include "sequence-editor.h"

NullEventType* NullEventType::GetInstance()
{
	static NullEventType* instance = new NullEventType();
	return instance;
}

NullEventType::NullEventType()
{
	this->cells[0] = new Cell();
	this->cells[1] = new NullEventTimeCell();
	this->cells[2] = new Cell();
	this->cells[3] = new Cell();
	this->cells[4] = new Cell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

void NullEventType::DeleteRow(SequenceEditor* sequence_editor, Row* row)
{
}

NullEventTimeCell::NullEventTimeCell()
{
	this->label = wxString("Time");
}

wxString NullEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(sequence_editor->step_size->GetTickFromStep(row->step_number));
}

