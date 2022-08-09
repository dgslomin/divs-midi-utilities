
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static RtMidiOutPtr second_midi_out = NULL;
static int note_velocity[128];
static int key_pressure_is_on[128];
static int channel_pressure_is_on = 0;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> --second-out <port>\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			int channel = MidiUtilKeyPressureMessage_getChannel(message);
			int note = MidiUtilNoteOffMessage_getNote(message);

			if (channel_pressure_is_on)
			{
				unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
				MidiUtilMessage_setNoteOff(new_message, channel, note, 0);
				rtmidi_out_send_message(second_midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
			}

			rtmidi_out_send_message(midi_out, message, message_size);
			note_velocity[note] = 0;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			int channel = MidiUtilKeyPressureMessage_getChannel(message);
			int note = MidiUtilNoteOnMessage_getNote(message);
			int velocity = MidiUtilNoteOnMessage_getVelocity(message);

			if (channel_pressure_is_on)
			{
				unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
				MidiUtilMessage_setNoteOff(new_message, channel, note, velocity);
				rtmidi_out_send_message(second_midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
			}

			rtmidi_out_send_message(midi_out, message, message_size);
			note_velocity[note] = velocity;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE:
		{
			int channel = MidiUtilKeyPressureMessage_getChannel(message);
			int note = MidiUtilKeyPressureMessage_getNote(message);
			int amount = MidiUtilKeyPressureMessage_getAmount(message);

			if ((amount >= 96) && !key_pressure_is_on[note] && (note_velocity[note] > 0))
			{
				unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
				MidiUtilMessage_setNoteOn(new_message, channel, note, note_velocity[note]);
				rtmidi_out_send_message(second_midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
				key_pressure_is_on[note] = 1;
			}
			else if ((amount < 32) && key_pressure_is_on[note])
			{
				unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
				MidiUtilMessage_setNoteOff(new_message, channel, note, 0);
				rtmidi_out_send_message(second_midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
				key_pressure_is_on[note] = 0;
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CHANNEL_PRESSURE:
		{
			int channel = MidiUtilChannelPressureMessage_getChannel(message);
			int amount = MidiUtilChannelPressureMessage_getAmount(message);

			if ((amount >= 96) && !channel_pressure_is_on)
			{
				for (int note = 0; note < 128; note++)
				{
					if (note_velocity[note] > 0)
					{
						unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
						MidiUtilMessage_setNoteOn(new_message, channel, note, note_velocity[note]);
						rtmidi_out_send_message(second_midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
					}
				}

				channel_pressure_is_on = 1;
			}
			else if ((amount < 32) && channel_pressure_is_on)
			{
				for (int note = 0; note < 128; note++)
				{
					if (note_velocity[note] > 0)
					{
						unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
						MidiUtilMessage_setNoteOff(new_message, channel, note, 0);
						rtmidi_out_send_message(second_midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
					}
				}

				channel_pressure_is_on = 0;
			}

			break;
		}
		default:
		{
			rtmidi_out_send_message(midi_out, message, message_size);
			break;
		}
	}
}

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
}

int main(int argc, char **argv)
{
	int i;

	for (i = 0; i < 128; i++)
	{
		note_velocity[i] = 0;
		key_pressure_is_on[i] = 0;
	}

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("secondtouch", argv[i], "secondtouch", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("secondtouch", argv[i], "secondtouch")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--second-out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((second_midi_out = rtmidi_open_out_port("secondtouch", argv[i], "secondtouch")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (midi_out == NULL) || (second_midi_out == NULL)) usage(argv[0]);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}

