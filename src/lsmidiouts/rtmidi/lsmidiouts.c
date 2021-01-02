
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>

int main(int argc, char **argv)
{
	RtMidiOutPtr midi_out = rtmidi_out_create(RTMIDI_API_UNSPECIFIED, "lsmidiouts");
	int number_of_ports = rtmidi_get_port_count(midi_out);
	int port_number;

	for (port_number = 0; port_number < number_of_ports; port_number++)
	{
		printf("%3d %s\n", port_number, rtmidi_get_port_name(midi_out, port_number));
	}

	rtmidi_close_port(midi_out);
}

