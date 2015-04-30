
#include <CoreMIDI/CoreMIDI.h>
#include <midifile.h>
#include <brainstorm-organizer-support.h>

class MidiSystem::Impl
{
public:
    MIDIClientRef midiClient;
    MIDIPortRef midiOutputPort;
    MIDIEndpointRef midiDestination;
};

MidiSystem::MidiSystem()
{
    this->impl = new Impl();
    MIDIClientCreate(CFSTR("Brainstorm Organizer"), NULL, NULL, &(this->impl->midiClient));
    MIDIOutputPortCreate(this->impl->midiClient, CFSTR("playback"), &(this->impl->midiOutputPort));
    this->impl->midiDestination = kMIDIUnknownEndpoint;
}

MidiSystem::~MidiSystem()
{
    MIDIPortDispose(this->impl->midiOutputPort);
    MIDIClientDispose(this->impl->midiClient);
    delete this->impl;
}

void MidiSystem::DisplayConfigDialog()
{
    // TODO: display modally; allow user to select; save in this->impl->midiDestination

    for (int destinationNumber = 0; destinationNumber < MIDIGetNumberOfDestinations(); destinationNumber++)
    {
        MIDIEndpointRef midiDestination = MIDIGetDestination(destinationNumber);
        CFStringRef midiDestinationName;
        MIDIObjectGetStringProperty(midiDestination, kMIDIPropertyDisplayName, &midiDestinationName);
    }
}

void MidiSystem::SendMessage(MidiFileEvent_t event)
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

