
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-rtmidi.h>

static int rtmidi_open_port_helper(RtMidiPtr device, char *port_name, char *virtual_port_name)
{
	if (port_name == NULL)
	{
		rtmidi_open_virtual_port(device, (const char *)(virtual_port_name));
	}
	else
	{
		int number_of_ports = rtmidi_get_port_count(device);
		int port_number;

		for (port_number = 0; port_number < number_of_ports; port_number++)
		{
			if (strcmp(rtmidi_get_port_name(device, port_number), port_name) == 0) break;
		}

		if (port_number == number_of_ports) sscanf(port_name, "%d", &port_number);

		if (port_number >= number_of_ports)
		{
			return -1;
		}

		rtmidi_open_port(device, port_number, (const char *)(virtual_port_name));
	}

	return 0;
}

RtMidiInPtr rtmidi_open_in_port(char *client_name, char *port_name, char *virtual_port_name, void (*callback)(double timestamp, const unsigned char *message, size_t message_size, void *user_data), void *user_data)
{
	RtMidiInPtr midi_in = rtmidi_in_create(RTMIDI_API_UNSPECIFIED, (const char *)(client_name), 100);
	rtmidi_in_set_callback(midi_in, callback, user_data);

	if (rtmidi_open_port_helper(midi_in, port_name, virtual_port_name) < 0)
	{
		rtmidi_close_port(midi_in);
		return NULL;
	}

	return midi_in;
}

RtMidiOutPtr rtmidi_open_out_port(char *client_name, char *port_name, char *virtual_port_name)
{
	RtMidiOutPtr midi_out = rtmidi_out_create(RTMIDI_API_UNSPECIFIED, (const char *)(client_name));

	if (rtmidi_open_port_helper(midi_out, port_name, virtual_port_name) < 0)
	{
		rtmidi_close_port(midi_out);
		return NULL;
	}

	return midi_out;
}

