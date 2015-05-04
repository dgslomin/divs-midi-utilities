
#include <wx/wx.h>
#include <wx/osx/core/cfstring.h>
#include <CoreMIDI/CoreMIDI.h>
#include <midifile.h>
#include <brainstorm-organizer-support.h>

class MidiOutput::Impl
{
public:
    MIDIClientRef midiClient;
    MIDIPortRef midiOutputPort;
    MIDIEndpointRef midiDestination;
};

MidiOutput::MidiOutput()
{
    this->impl = new Impl();
    MIDIClientCreate(CFSTR("Brainstorm Organizer"), NULL, NULL, &(this->impl->midiClient));
    MIDIOutputPortCreate(this->impl->midiClient, CFSTR("playback"), &(this->impl->midiOutputPort));
    this->impl->midiDestination = kMIDIUnknownEndpoint;
}

MidiOutput::~MidiOutput()
{
    MIDIPortDispose(this->impl->midiOutputPort);
    MIDIClientDispose(this->impl->midiClient);
    delete this->impl;
}

void MidiOutput::DisplayConfigDialog()
{
    wxDialog *dialog = new wxDialog(NULL, wxID_ANY, "Brainstorm Organizer");

    wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
    dialog->SetSizer(outerSizer);

    wxListBox* destinationList = new wxListBox(dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);
    outerSizer->Add(destinationList, 1, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM | wxLEFT, 4);

    wxSizer* buttonSizer = dialog->CreateButtonSizer(wxOK | wxCANCEL);
    outerSizer->Add(buttonSizer, 0, wxRIGHT | wxBOTTOM | wxLEFT, 4);
    
    for (int destinationNumber = 0; destinationNumber < MIDIGetNumberOfDestinations(); destinationNumber++)
    {
        MIDIEndpointRef midiDestination = MIDIGetDestination(destinationNumber);
        CFStringRef midiDestinationName;
        MIDIObjectGetStringProperty(midiDestination, kMIDIPropertyDisplayName, &midiDestinationName);
        destinationList->Append(wxCFStringRef(midiDestinationName).AsString());
        if (midiDestination == this->impl->midiDestination) destinationList->SetSelection(destinationNumber);
    }

    if (dialog->ShowModal() == wxID_OK)
    {
        int destinationNumber = destinationList->GetSelection();
        if (destinationNumber != wxNOT_FOUND) this->impl->midiDestination = MIDIGetDestination(destinationNumber);
    }

    dialog->Destroy();
}

void MidiOutput::SendMessage(MidiFileEvent_t event)
{
    if ((this->impl->midiDestination == kMIDIUnknownEndpoint) || !MidiFileEvent_isVoiceEvent(event)) return;

    union
    {
        unsigned long messageAsUint32;
        Byte messageAsBytes[4];
    }
    u;

    u.messageAsUint32 = MidiFileVoiceEvent_getData(event);
    Byte midiPacketListBuffer[128];
    MIDIPacketList* midiPacketList = (MIDIPacketList*)(midiPacketListBuffer);
    MIDIPacket* midiPacket = MIDIPacketListInit(midiPacketList);
    MIDIPacketListAdd(midiPacketList, sizeof(midiPacketListBuffer), midiPacket, 0, MidiFileVoiceEvent_getDataLength(event), u.messageAsBytes);
    MIDISend(this->impl->midiOutputPort, this->impl->midiDestination, midiPacketList);
}

