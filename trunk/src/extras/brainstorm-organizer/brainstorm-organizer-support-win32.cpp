
#include <wx/wx.h>
#include <Windows.h>
#include <brainstorm-organizer-support.h>

class MidiOutput::Impl
{
public:
	int midiOutNumber;
	HMIDIOUT midiOut;
};

MidiOutput::MidiOutput()
{
	this->impl = new Impl();
	this->impl->midiOutNumber = MIDI_MAPPER;
	midiOutOpen(&(this->impl->midiOut), this->impl->midiOutNumber, (DWORD_PTR)(NULL), (DWORD_PTR)(NULL), CALLBACK_NULL);
}

MidiOutput::~MidiOutput()
{
	midiOutClose(this->impl->midiOut);
	delete this->impl;
}

void MidiOutput::DisplayConfigDialog()
{
	wxDialog* dialog = new wxDialog(NULL, wxID_ANY, "Brainstorm Organizer");

	wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
	dialog->SetSizer(outerSizer);

	wxListBox* midiOutList = new wxListBox(dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);
	outerSizer->Add(midiOutList, 1, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM | wxLEFT, 4);

	wxSizer* buttonSizer = dialog->CreateButtonSizer(wxOK | wxCANCEL);
	outerSizer->Add(buttonSizer, 0, wxRIGHT | wxBOTTOM | wxLEFT, 4);
	
	for (int midiOutNumber = 0; midiOutNumber < midiOutGetNumDevs(); midiOutNumber++)
	{
		MIDIOUTCAPS midiOutCaps;
		midiOutGetDevCaps(midiOutNumber, &midiOutCaps, sizeof(midiOutCaps));
		midiOutList->Append(wxString(midiOutCaps.szPname));
		if (midiOutNumber == this->impl->midiOutNumber) midiOutList->SetSelection(midiOutNumber);
	}

	if (dialog->ShowModal() == wxID_OK)
	{
		midiOutClose(this->impl->midiOut);
		this->impl->midiOutNumber = midiOutList->GetSelection();
		midiOutOpen(&(this->impl->midiOut), this->impl->midiOutNumber, (DWORD_PTR)(NULL), (DWORD_PTR)(NULL), CALLBACK_NULL);
	}

	dialog->Destroy();
}

void MidiOutput::SendMessage(MidiFileEvent_t event)
{
	if (MidiFileEvent_isVoiceEvent(event))
	{
		midiOutShortMsg(this->impl->midiOut, (DWORD)(MidiFileVoiceEvent_getData(event)));
	}
}

