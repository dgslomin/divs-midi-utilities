
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

struct Bus
{
	int number_of_midi_ins;
	RtMidiInPtr midi_ins[16];
	int number_of_midi_outs;
	RtMidiOutPtr midi_outs[16];
	int channel_output_bus_number[16];
	int channel_output_channel_number[16];
};

typedef struct Bus *Bus_t;

static int number_of_busses = 1;
static struct Bus busses[16];

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --bus | --in <port> | --out <port> | --virtual-in <port> | --virtual-out <port> ] ... [ --channel <input bus number> <input channel number> <output bus number> <output channel number> ] ...\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	Bus_t input_bus = (Bus_t)(user_data);
	int input_channel = MidiUtilMessage_getChannel(message);

	if (input_channel < 0)
	{
		int midi_out_number;

		for (midi_out_number = 0; midi_out_number < input_bus->number_of_midi_outs; midi_out_number++)
		{
			rtmidi_out_send_message(input_bus->midi_outs[midi_out_number], message, message_size);
		}
	}
	else
	{
		Bus_t output_bus = &(busses[input_bus->channel_output_bus_number[input_channel]]);
		int output_channel = input_bus->channel_output_channel_number[input_channel];
		unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_SHORT_MESSAGE];
		int midi_out_number;

		memcpy(new_message, message, message_size);
		MidiUtilMessage_setChannel(new_message, output_channel);

		for (midi_out_number = 0; midi_out_number < output_bus->number_of_midi_outs; midi_out_number++)
		{
			rtmidi_out_send_message(output_bus->midi_outs[midi_out_number], new_message, message_size);
		}
	}
}

static void handle_exit(void *user_data)
{
	while (number_of_busses > 0)
	{
		Bus_t bus = &(busses[--number_of_busses]);
		while (bus->number_of_midi_ins > 0) rtmidi_close_port(bus->midi_ins[--(bus->number_of_midi_ins)]);
		while (bus->number_of_midi_outs > 0) rtmidi_close_port(bus->midi_outs[--(bus->number_of_midi_outs)]);
	}
}

int main(int argc, char **argv)
{
	int i;

	{
		int bus_number, channel_number;

		for (bus_number = 0; bus_number < 16; bus_number++)
		{
			busses[bus_number].number_of_midi_ins = 0;
			busses[bus_number].number_of_midi_outs = 0;

			for (channel_number = 0; channel_number < 16; channel_number++)
			{
				busses[bus_number].channel_output_bus_number[channel_number] = bus_number;
				busses[bus_number].channel_output_channel_number[channel_number] = channel_number;
			}
		}
	}

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--bus") == 0)
		{
			number_of_busses++;
		}
		else if (strcmp(argv[i], "--in") == 0)
		{
			Bus_t bus = &(busses[number_of_busses - 1]);
			if (++i == argc) usage(argv[0]);

			if ((bus->midi_ins[(bus->number_of_midi_ins)++] = rtmidi_open_in_port("midithru", argv[i], "midithru", handle_midi_message, bus)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			Bus_t bus = &(busses[number_of_busses - 1]);
			if (++i == argc) usage(argv[0]);

			if ((bus->midi_outs[(bus->number_of_midi_outs)++] = rtmidi_open_out_port("midithru", argv[i], "midithru")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--virtual-in") == 0)
		{
			Bus_t bus = &(busses[number_of_busses - 1]);
			if (++i == argc) usage(argv[0]);
			bus->midi_ins[(bus->number_of_midi_ins)++] = rtmidi_open_in_port("midithru", NULL, argv[i], handle_midi_message, bus);
		}
		else if (strcmp(argv[i], "--virtual-out") == 0)
		{
			Bus_t bus = &(busses[number_of_busses - 1]);
			if (++i == argc) usage(argv[0]);
			bus->midi_outs[(bus->number_of_midi_outs)++] = rtmidi_open_out_port("midithru", NULL, argv[i]);
		}
		else if (strcmp(argv[i], "--channel") == 0)
		{
			int input_bus_number, input_channel_number, output_bus_number, output_channel_number;
			if (++i == argc) usage(argv[0]);
			input_bus_number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			input_channel_number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			output_bus_number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			output_channel_number = atoi(argv[i]);
			busses[input_bus_number].channel_output_bus_number[input_channel_number] = output_bus_number;
			busses[input_bus_number].channel_output_channel_number[input_channel_number] = output_channel_number;
		}
		else
		{
			usage(argv[0]);
		}
	}

	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}

