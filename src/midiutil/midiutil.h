#ifndef MIDI_UTIL_INCLUDED
#define MIDI_UTIL_INCLUDED

#include <rtmidi_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MidiUtilLock *MidiUtilLock_t;

void MidiUtil_startThread(void (*callback)(void *user_data), void *user_data);

MidiUtilLock_t MidiUtilLock_new(void);
void MidiUtilLock_free(MidiUtilLock_t lock);
void MidiUtilLock_lock(MidiUtilLock_t lock);
void MidiUtilLock_unlock(MidiUtilLock_t lock);
void MidiUtilLock_wait(MidiUtilLock_t lock, long timeout_msecs);
void MidiUtilLock_notify(MidiUtilLock_t lock);
void MidiUtilLock_notifyAll(MidiUtilLock_t lock);

void MidiUtil_sleep(long msecs);
long MidiUtil_getCurrentTimeMsecs(void);

void MidiUtil_setInterruptHandler(void (*callback)(void *user_data), void *user_data);
void MidiUtil_waitForInterrupt(void);

RtMidiInPtr rtmidi_open_in_port(char *client_name, char *port_name, char *virtual_port_name, void (*callback)(double timestamp, const unsigned char *message, size_t message_size, void *user_data), void *user_data);
RtMidiOutPtr rtmidi_open_out_port(char *client_name, char *port_name, char *virtual_port_name);

#ifdef __cplusplus
}
#endif

#endif
