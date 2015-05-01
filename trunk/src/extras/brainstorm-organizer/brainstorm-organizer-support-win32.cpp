
#include <Windows.h>
#include <brainstorm-organizer-support.h>

class MidiOutput::Impl
{
public:
    HMIDIOUT midiOut;
};

MidiOutput::MidiOutput()
{
    int midiOutNumber = MIDI_MAPPER;
    this->impl = new Impl();
	midiOutOpen(&(this->impl->midiOut), midiOutNumber, (DWORD_PTR)(NULL), (DWORD_PTR)(NULL), CALLBACK_NULL);
}

MidiOutput::~MidiOutput()
{
    midiOutClose(this->impl->midiOut);
    delete this->impl;
}

void MidiOutput::DisplayConfigDialog()
{
    // TODO: display modally; allow user to select
    // midiOutClose(this->impl->midiOut);
	// midiOutOpen(&(this->impl->midiOut), midiOutNumber, (DWORD_PTR)(NULL), (DWORD_PTR)(NULL), CALLBACK_NULL);

    for (int midiOutNumber = 0; midiOutNumber < midiOutGetNumDevs(); midiOutNumber++)
    {
        MIDIOUTCAPS midiOutCaps;
        midiOutGetDevCaps(midiOutNumber, &midiOutCaps, sizeof(midiOutCaps));
        char* midiOutName = midiOutCaps.szPname;
    }
}

void MidiOutput::SendMessage(MidiFileEvent_t event)
{
    if (MidiFileEvent_isVoiceEvent(event))
    {
	    midiOutShortMsg(this->impl->midiOut, (DWORD)(MidiFileVoiceEvent_getData(event)));
    }
}

