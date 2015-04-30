
#include <Windows.h>
#include <brainstorm-organizer-support.h>

class MidiSystem::Impl
{
public:
    HMIDIOUT midiOut;
};

MidiSystem::MidiSystem()
{
    int midiOutNumber = MIDI_MAPPER;
    this->impl = new Impl();
	midiOutOpen(&(this->impl->midiOut), midiOutNumber, (DWORD_PTR)(NULL), (DWORD_PTR)(NULL), CALLBACK_NULL);
}

MidiSystem::~MidiSystem()
{
    midiOutClose(this->impl->midiOut);
    delete this->impl;
}

void MidiSystem::DisplayConfigDialog()
{
}

void MidiSystem::SendMessage(MidiFileEvent_t event)
{
    if (MidiFileEvent_isVoiceEvent(event))
    {
	    midiOutShortMsg(this->impl->midiOut, (DWORD)(MidiFileVoiceEvent_getData(event)));
    }
}

