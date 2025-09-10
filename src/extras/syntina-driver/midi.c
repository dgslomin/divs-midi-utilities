
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-rtmidi.h>
#include "midi.h"

struct MidiOut
{
	RtMidiOutPtr rtmidi_out;
};

MidiOut_t MidiOut_open(char *port_name)
{
	MidiOut_t midi_out = (MidiOut_t)(malloc(sizeof (struct MidiOut)));

	while (1)
	{
		midi_out->rtmidi_out = rtmidi_open_out_port("syntina", port_name, "syntina");

		if (midi_out->rtmidi_out != NULL)
		{
			fprintf(stderr, "Info: connected to MIDI output %s\n", port_name);
			return midi_out;
		}

		sleep(0.5);
	}
}

void MidiOut_close(MidiOut_t midi_out)
{
	rtmidi_close_port(midi_out->rtmidi_out);
}

void MidiOut_sendNoteOn(MidiOut_t midi_out, int channel, int note, int velocity)
{
	if (midi_out == NULL) return;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
	MidiUtilMessage_setNoteOn(message, channel, note, velocity);
	rtmidi_out_send_message(midi_out->rtmidi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
}

void MidiOut_sendNoteOff(MidiOut_t midi_out, int channel, int note, int velocity)
{
	if (midi_out == NULL) return;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
	MidiUtilMessage_setNoteOff(message, channel, note, velocity);
	rtmidi_out_send_message(midi_out->rtmidi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
}

void MidiOut_sendControlChange(MidiOut_t midi_out, int channel, int number, int value)
{
	if (midi_out == NULL) return;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE];
	MidiUtilMessage_setControlChange(message, channel, number, value);
	rtmidi_out_send_message(midi_out->rtmidi_out, message, MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE);
}

void MidiOut_sendProgramChange(MidiOut_t midi_out, int channel, int number)
{
	if (midi_out == NULL) return;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_PROGRAM_CHANGE];
	MidiUtilMessage_setProgramChange(message, channel, number);
	rtmidi_out_send_message(midi_out->rtmidi_out, message, MIDI_UTIL_MESSAGE_SIZE_PROGRAM_CHANGE);
}

void MidiOut_sendPitchBend(MidiOut_t midi_out, int channel, int amount)
{
	if (midi_out == NULL) return;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_PITCH_WHEEL];
	MidiUtilMessage_setPitchWheel(message, channel, amount);
	rtmidi_out_send_message(midi_out->rtmidi_out, message, MIDI_UTIL_MESSAGE_SIZE_PITCH_WHEEL);
}

