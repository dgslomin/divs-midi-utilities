
#include <alsa/asoundlib.h>
#include <brainstorm-organizer-support.h>

class MidiSystem::Impl
{
public:
    snd_seq_t* alsaSequencerOutput;
    int alsaSequencerOutputPort;
};

MidiSystem::MidiSystem()
{
    this->impl = new Impl();
    snd_seq_open(&(this->impl->alsaSequencerOutput), "default", SND_SEQ_OPEN_OUTPUT, 0);
    snd_seq_set_client_name(this->impl->alsaSequencerOutput, "Brainstorm Organizer");
    this->impl->alsaSequencerOutputPort = snd_seq_create_simple_port(this->impl->alsaSequencerOutput, "playback", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
}

MidiSystem::~MidiSystem()
{
    snd_seq_delete_simple_port(this->impl->alsaSequencerOutput, this->impl->alsaSequencerOutputPort);
    snd_seq_close(this->impl->alsaSequencerOutput);
    delete this->impl;
}

void MidiSystem::DisplayConfigDialog()
{
    // TODO: display modally; allow user to select; remember to disconnect the existing connection, and to support selecting none
	// snd_seq_connect_to(alsaSequencerOutput, alsaSequencerOutputPort, alsaSequencerDestinationClientId, alsaSequencerDestinationPortId);

    snd_seq_client_info_t* alsaSequencerDestinationClientInfo;
    snd_seq_client_info_malloc(&alsaSequencerDestinationClientInfo);
    snd_seq_port_info_t* alsaSequencerDestinationPortInfo;
    snd_seq_port_info_malloc(&alsaSequencerDestinationPortInfo);
    snd_seq_client_info_set_client(alsaSequencerDestinationClientInfo, -1);

	while (snd_seq_query_next_client(alsaSequencerOutput, alsaSequencerDestinationClientInfo) == 0)
	{
		int alsaSequencerDestinationClientId = snd_seq_client_info_get_client(alsaSequencerDestinationClientInfo);
        char* alsaSequencerDestinationClientName = snd_seq_client_info_get_name(alsaSequencerDestinationClientInfo);
        snd_seq_port_info_set_client(alsaSequencerDestinationPortInfo, alsaSequencerDestinationClientId);
        snd_seq_port_info_set_port(alsaSequencerDestinationPortInfo, -1);

        while (snd_seq_query_next_port(alsaSequencerOutput, alsaSequencerDestinationPortInfo) == 0)
        {
            if (send_seq_port_info_get_capability(alsaSequencerDestinationPortInfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE) != 0)
            {
                int alsaSequencerDestinationPortId = snd_seq_port_info_get_port(alsaSequencerDestinationPortInfo);
                char* alsaSequencerDestinationPortName = snd_seq_port_info_get_name(alsaSequencerDestinationPortInfo);
            }
        }
    }

    snd_seq_port_info_free(alsaSequencerDestinationPortInfo);
    snd_seq_client_info_free(alsaSequencerDestinationClientInfo);
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
            snd_seq_event_output_direct(this->impl->alsaSequencerOutput, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_NOTE_ON:
        {
            snd_seq_ev_set_noteon(&alsaSequencerEvent, MidiFileNoteOnEvent_getChannel(event), MidiFileNoteOnEvent_getNote(event), MidiFileNoteOnEvent_getVelocity(event));
            snd_seq_event_output_direct(this->impl->alsaSequencerOutput, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
        {
            snd_seq_ev_set_keypress(&alsaSequencerEvent, MidiFileKeyPressureEvent_getChannel(event), MidiFileKeyPressureEvent_getNote(event), MidiFileKeyPressureEvent_getAmount(event));
            snd_seq_event_output_direct(this->impl->alsaSequencerOutput, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
        {
            snd_seq_ev_set_controller(&alsaSequencerEvent, MidiFileControlChangeEvent_getChannel(event), MidiFileControlChangeEvent_getNumber(event), MidiFileControlChangeEvent_getValue(event));
            snd_seq_event_output_direct(this->impl->alsaSequencerOutput, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
        {
            snd_seq_ev_set_pgmchange(&alsaSequencerEvent, MidiFileProgramChangeEvent_getChannel(event), MidiFileProgramChangeEvent_getNumber(event));
            snd_seq_event_output_direct(this->impl->alsaSequencerOutput, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
        {
            snd_seq_ev_set_chanpress(&alsaSequencerEvent, MidiFileChannelPressureEvent_getChannel(event), MidiFileChannelPressureEvent_getAmount(event));
            snd_seq_event_output_direct(this->impl->alsaSequencerOutput, &alsaSequencerEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
        {
            snd_seq_ev_set_pitchbend(&alsaSequencerEvent, MidiFilePitchWheelEvent_getChannel(event), MidiFilePitchWheelEvent_getValue(event));
            snd_seq_event_output_direct(this->impl->alsaSequencerOutput, &alsaSequencerEvent);
            break;
        }
        default:
        {
            break;
        }
    }
}

