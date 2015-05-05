
#include <wx/wx.h>
#include <alsa/asoundlib.h>
#include <midifile.h>
#include <brainstorm-organizer-support.h>

class MidiOutput::Impl
{
public:
    snd_seq_t* alsaSequencer;
    int outputPort;
    int destinationNumber;
};

MidiOutput::MidiOutput()
{
    this->impl = new Impl();
    snd_seq_open(&(this->impl->alsaSequencer), "default", SND_SEQ_OPEN_OUTPUT, 0);
    snd_seq_set_client_name(this->impl->alsaSequencer, "Brainstorm Organizer");
    this->impl->outputPort = snd_seq_create_simple_port(this->impl->alsaSequencer, "Brainstorm Organizer playback port", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    this->impl->destinationNumber = wxNOT_FOUND;
}

MidiOutput::~MidiOutput()
{
    snd_seq_delete_simple_port(this->impl->alsaSequencer, this->impl->outputPort);
    snd_seq_close(this->impl->alsaSequencer);
    delete this->impl;
}

void MidiOutput::DisplayConfigDialog()
{
    wxDialog* dialog = new wxDialog(NULL, wxID_ANY, "Brainstorm Organizer");

    wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
    dialog->SetSizer(outerSizer);

    wxListBox* destinationList = new wxListBox(dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);
    outerSizer->Add(destinationList, 1, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM | wxLEFT, 4);

    wxSizer* buttonSizer = dialog->CreateButtonSizer(wxOK | wxCANCEL);
    outerSizer->Add(buttonSizer, 0, wxRIGHT | wxBOTTOM | wxLEFT, 4);
    
    snd_seq_client_info_t* destinationClientInfo;
    snd_seq_client_info_malloc(&destinationClientInfo);
    snd_seq_port_info_t* destinationPortInfo;
    snd_seq_port_info_malloc(&destinationPortInfo);

    int destinationNumber = 0;
    snd_seq_client_info_set_client(destinationClientInfo, -1);

	while (snd_seq_query_next_client(this->impl->alsaSequencer, destinationClientInfo) == 0)
	{
		int destinationClientId = snd_seq_client_info_get_client(destinationClientInfo);

        if (destinationClientId != SND_SEQ_CLIENT_SYSTEM)
        {
            snd_seq_port_info_set_client(destinationPortInfo, destinationClientId);
            snd_seq_port_info_set_port(destinationPortInfo, -1);

            while (snd_seq_query_next_port(this->impl->alsaSequencer, destinationPortInfo) == 0)
            {
                if ((snd_seq_port_info_get_capability(destinationPortInfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) != 0)
                {
                    destinationList->Append(wxString(snd_seq_port_info_get_name(destinationPortInfo)));
                    destinationNumber++;
                }
            }
        }
    }

    if (this->impl->destinationNumber != wxNOT_FOUND) destinationList->SetSelection(this->impl->destinationNumber);

    if (dialog->ShowModal() == wxID_OK)
    {
        destinationNumber = 0;
        snd_seq_client_info_set_client(destinationClientInfo, -1);

	    while (snd_seq_query_next_client(this->impl->alsaSequencer, destinationClientInfo) == 0)
	    {
		    int destinationClientId = snd_seq_client_info_get_client(destinationClientInfo);

            if (destinationClientId != SND_SEQ_CLIENT_SYSTEM)
            {
                snd_seq_port_info_set_client(destinationPortInfo, destinationClientId);
                snd_seq_port_info_set_port(destinationPortInfo, -1);

                while (snd_seq_query_next_port(this->impl->alsaSequencer, destinationPortInfo) == 0)
                {
                    if ((snd_seq_port_info_get_capability(destinationPortInfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) != 0)
                    {
                        int destinationPortId = snd_seq_port_info_get_port(destinationPortInfo);
                        if (destinationNumber == this->impl->destinationNumber) snd_seq_disconnect_to(this->impl->alsaSequencer, this->impl->outputPort, destinationClientId, destinationPortId);
                        if (destinationNumber == destinationList->GetSelection()) snd_seq_connect_to(this->impl->alsaSequencer, this->impl->outputPort, destinationClientId, destinationPortId);
                        destinationNumber++;
                    }
                }
            }
        }

        this->impl->destinationNumber = destinationList->GetSelection();
    }

    snd_seq_port_info_free(destinationPortInfo);
    snd_seq_client_info_free(destinationClientInfo);

    dialog->Destroy();
}

void MidiOutput::SendMessage(MidiFileEvent_t midiFileEvent)
{
    snd_seq_event_t alsaEvent;
    snd_seq_ev_clear(&alsaEvent);
    snd_seq_ev_set_source(&alsaEvent, this->impl->outputPort);
    snd_seq_ev_set_subs(&alsaEvent);
    snd_seq_ev_set_direct(&alsaEvent);

    switch (MidiFileEvent_getType(midiFileEvent))
    {
        case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
        {
            snd_seq_ev_set_noteoff(&alsaEvent, MidiFileNoteOffEvent_getChannel(midiFileEvent), MidiFileNoteOffEvent_getNote(midiFileEvent), MidiFileNoteOffEvent_getVelocity(midiFileEvent));
            snd_seq_event_output_direct(this->impl->alsaSequencer, &alsaEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_NOTE_ON:
        {
            snd_seq_ev_set_noteon(&alsaEvent, MidiFileNoteOnEvent_getChannel(midiFileEvent), MidiFileNoteOnEvent_getNote(midiFileEvent), MidiFileNoteOnEvent_getVelocity(midiFileEvent));
            snd_seq_event_output_direct(this->impl->alsaSequencer, &alsaEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
        {
            snd_seq_ev_set_keypress(&alsaEvent, MidiFileKeyPressureEvent_getChannel(midiFileEvent), MidiFileKeyPressureEvent_getNote(midiFileEvent), MidiFileKeyPressureEvent_getAmount(midiFileEvent));
            snd_seq_event_output_direct(this->impl->alsaSequencer, &alsaEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
        {
            snd_seq_ev_set_controller(&alsaEvent, MidiFileControlChangeEvent_getChannel(midiFileEvent), MidiFileControlChangeEvent_getNumber(midiFileEvent), MidiFileControlChangeEvent_getValue(midiFileEvent));
            snd_seq_event_output_direct(this->impl->alsaSequencer, &alsaEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
        {
            snd_seq_ev_set_pgmchange(&alsaEvent, MidiFileProgramChangeEvent_getChannel(midiFileEvent), MidiFileProgramChangeEvent_getNumber(midiFileEvent));
            snd_seq_event_output_direct(this->impl->alsaSequencer, &alsaEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
        {
            snd_seq_ev_set_chanpress(&alsaEvent, MidiFileChannelPressureEvent_getChannel(midiFileEvent), MidiFileChannelPressureEvent_getAmount(midiFileEvent));
            snd_seq_event_output_direct(this->impl->alsaSequencer, &alsaEvent);
            break;
        }
        case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
        {
            snd_seq_ev_set_pitchbend(&alsaEvent, MidiFilePitchWheelEvent_getChannel(midiFileEvent), MidiFilePitchWheelEvent_getValue(midiFileEvent));
            snd_seq_event_output_direct(this->impl->alsaSequencer, &alsaEvent);
            break;
        }
        default:
        {
            break;
        }
    }
}

