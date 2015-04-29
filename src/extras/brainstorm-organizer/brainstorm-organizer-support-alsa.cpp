
#include <alsa/asoundlib.h>
#include <brainstorm-organizer-support.h>

class MidiSystem::Impl
{
public:
    snd_seq_t* seq;
    int port;
}

MidiSystem::MidiSystem()
{
    this->impl = new Impl();
    snd_seq_open(&(this->impl->seq), "default", SND_SEQ_OPEN_OUTPUT, 0);
    snd_seq_set_client_name(this->impl->seq, "Brainstorm Organizer");
    this->impl->port = snd_seq_create_simple_port(this->impl->seq, "playback", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
}

MidiSystem::~MidiSystem()
{
    snd_seq_delete_simple_port(this->impl->seq, this->impl->port);
    snd_seq_close(this->impl->seq);
    delete this->impl;
}

void MidiSystem::DisplayConfigDialog()
{
}

void MidiSystem::SendMessage(MidiFileEvent_t event)
{
    snd_seq_ev_t alsaSequencerEvent;
    snd_seq_ev_clear(&alsaSequencerEvent);

    switch (MidiFileEvent_getType(event))
    {
        case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
        {
            snd_seq_ev_set_noteoff(&alsaSequencerEvent, MidiFileNoteOffEvent_getChannel(event), MidiFileNoteOffEvent_getNote(event), MidiFileNoteOffEvent_getVelocity(event));
            snd_seq_event_output_direct(this->impl->seq, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_NOTE_ON:
        {
            snd_seq_ev_set_noteon(&alsaSequencerEvent, MidiFileNoteOnEvent_getChannel(event), MidiFileNoteOnEvent_getNote(event), MidiFileNoteOnEvent_getVelocity(event));
            snd_seq_event_output_direct(this->impl->seq, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
        {
            snd_seq_ev_set_keypress(&alsaSequencerEvent, MidiFileKeyPressureEvent_getChannel(event), MidiFileKeyPressureEvent_getNote(event), MidiFileKeyPressureEvent_getAmount(event));
            snd_seq_event_output_direct(this->impl->seq, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
        {
            snd_seq_ev_set_controller(&alsaSequencerEvent, MidiFileControlChangeEvent_getChannel(event), MidiFileControlChangeEvent_getNumber(event), MidiFileControlChangeEvent_getValue(event));
            snd_seq_event_output_direct(this->impl->seq, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
        {
            snd_seq_ev_set_pgmchange(&alsaSequencerEvent, MidiFileProgramChangeEvent_getChannel(event), MidiFileProgramChangeEvent_getNumber(event));
            snd_seq_event_output_direct(this->impl->seq, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
        {
            snd_seq_ev_set_chanpress(&alsaSequencerEvent, MidiFileChannelPressureEvent_getChannel(event), MidiFileChannelPressureEvent_getAmount(event));
            snd_seq_event_output_direct(this->impl->seq, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
        {
            snd_seq_ev_set_pitchbend(&alsaSequencerEvent, MidiFilePitchWheelEvent_getChannel(event), MidiFilePitchWheelEvent_getValue(event));
            snd_seq_event_output_direct(this->impl->seq, &alsaSequencerEvent);
            break;
        }
        default:
        {
            break;
        }
    }
}

