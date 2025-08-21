#ifndef SYNTINA_DRIVER_MIDI_INCLUDED
#define SYNTINA_DRIVER_MIDI_INCLUDED

typedef struct MidiOut *MidiOut_t;
MidiOut_t MidiOut_open(char *port_name);
void MidiOut_close(MidiOut_t midi_out);
void MidiOut_sendNoteOn(MidiOut_t midi_out, int channel, int note, int velocity);
void MidiOut_sendNoteOff(MidiOut_t midi_out, int channel, int note, int velocity);
void MidiOut_sendControlChange(MidiOut_t midi_out, int channel, int number, int value);

#endif
