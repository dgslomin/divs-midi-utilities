#ifndef MIDIUTIL_RTMIDI_INCLUDED
#define MIDIUTIL_RTMIDI_INCLUDED

/* Common helpers that work with rtmidi. */

#include <rtmidi_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

RtMidiInPtr rtmidi_open_in_port(char *client_name, char *port_name, char *virtual_port_name, void (*callback)(double timestamp, const unsigned char *message, size_t message_size, void *user_data), void *user_data);
RtMidiOutPtr rtmidi_open_out_port(char *client_name, char *port_name, char *virtual_port_name);

#ifdef __cplusplus
}
#endif

#endif
