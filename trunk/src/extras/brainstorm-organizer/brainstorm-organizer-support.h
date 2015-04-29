#ifndef BRAINSTORM_ORGANIZER_SUPPORT_INCLUDED
#define BRAINSTORM_ORGANIZER_SUPPORT_INCLUDED

class MidiSystem
{
public:
    MidiSystem();
    ~MidiSystem();
    void DisplayConfigDialog();
    void SendMessage(MidiFileEvent_t event);

private:
    class Impl;
    Impl* impl;
};

#endif
