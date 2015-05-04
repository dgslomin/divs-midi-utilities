#ifndef BRAINSTORM_ORGANIZER_SUPPORT_INCLUDED
#define BRAINSTORM_ORGANIZER_SUPPORT_INCLUDED

#include <midifile.h>

class MidiOutput
{
public:
    MidiOutput();
    ~MidiOutput();
    void DisplayConfigDialog();
    void SendMessage(MidiFileEvent_t event);

private:
    class Impl;
    Impl* impl;
};

#endif
