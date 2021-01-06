#ifndef MIDI_UTIL_INCLUDED
#define MIDI_UTIL_INCLUDED

#include <rtmidi_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MidiUtilLock *MidiUtilLock_t;
typedef struct MidiUtilAlarm *MidiUtilAlarm_t;

typedef enum
{
	RTMIDI_MESSAGE_TYPE_NOTE_OFF = 0x80,
	RTMIDI_MESSAGE_TYPE_NOTE_ON = 0x90,
	RTMIDI_MESSAGE_TYPE_KEY_PRESSURE = 0xA0,
	RTMIDI_MESSAGE_TYPE_CONTROL_CHANGE = 0xB0,
	RTMIDI_MESSAGE_TYPE_PROGRAM_CHANGE = 0xC0,
	RTMIDI_MESSAGE_TYPE_CHANNEL_PRESSURE = 0xD0,
	RTMIDI_MESSAGE_TYPE_PITCH_WHEEL = 0xE0,
	RTMIDI_MESSAGE_TYPE_SYSEX = 0xF0
}
RtMidiMessageType;

typedef enum
{
	RTMIDI_MESSAGE_SIZE_NOTE_OFF = 3,
	RTMIDI_MESSAGE_SIZE_NOTE_ON = 3,
	RTMIDI_MESSAGE_SIZE_KEY_PRESSURE = 3,
	RTMIDI_MESSAGE_SIZE_CONTROL_CHANGE = 2,
	RTMIDI_MESSAGE_SIZE_PROGRAM_CHANGE = 2,
	RTMIDI_MESSAGE_SIZE_CHANNEL_PRESSURE = 2,
	RTMIDI_MESSAGE_SIZE_PITCH_WHEEL = 3,
	RTMIDI_MESSAGE_SIZE_MAX = 3
}
RtMidiMessageSize;


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
void MidiUtil_getCurrentTimeString(char *current_time_string); /* YYYYMMDDhhmmss */

MidiUtilAlarm_t MidiUtilAlarm_new(void);
void MidiUtilAlarm_set(MidiUtilAlarm_t alarm, long msecs, void (*callback)(void *user_data), void *user_data);
void MidiUtilAlarm_cancel(MidiUtilAlarm_t alarm);
void MidiUtilAlarm_free(MidiUtilAlarm_t alarm);

void MidiUtil_setInterruptHandler(void (*callback)(void *user_data), void *user_data);
void MidiUtil_waitForInterrupt(void);

int MidiUtil_clamp(int i, int low, int high);

RtMidiInPtr rtmidi_open_in_port(char *client_name, char *port_name, char *virtual_port_name, void (*callback)(double timestamp, const unsigned char *message, size_t message_size, void *user_data), void *user_data);
RtMidiOutPtr rtmidi_open_out_port(char *client_name, char *port_name, char *virtual_port_name);

RtMidiMessageType rtmidi_message_get_type(const unsigned char *message);
void rtmidi_message_set_type(unsigned char *message, RtMidiMessageType type);

int rtmidi_note_off_message_get_channel(const unsigned char *message);
void rtmidi_note_off_message_set_channel(unsigned char *message, int channel);
int rtmidi_note_off_message_get_note(const unsigned char *message);
void rtmidi_note_off_message_set_note(unsigned char *message, int note);
int rtmidi_note_off_message_get_velocity(const unsigned char *message);
void rtmidi_note_off_message_set_velocity(unsigned char *message, int velocity);

int rtmidi_note_on_message_get_channel(const unsigned char *message);
void rtmidi_note_on_message_set_channel(unsigned char *message, int channel);
int rtmidi_note_on_message_get_note(const unsigned char *message);
void rtmidi_note_on_message_set_note(unsigned char *message, int note);
int rtmidi_note_on_message_get_velocity(const unsigned char *message);
void rtmidi_note_on_message_set_velocity(unsigned char *message, int velocity);

int rtmidi_key_pressure_message_get_channel(const unsigned char *message);
void rtmidi_key_pressure_message_set_channel(unsigned char *message, int channel);
int rtmidi_key_pressure_message_get_note(const unsigned char *message);
void rtmidi_key_pressure_message_set_note(unsigned char *message, int note);
int rtmidi_key_pressure_message_get_amount(const unsigned char *message);
void rtmidi_key_pressure_message_set_amount(unsigned char *message, int amount);

int rtmidi_control_change_message_get_channel(const unsigned char *message);
void rtmidi_control_change_message_set_channel(unsigned char *message, int channel);
int rtmidi_control_change_message_get_number(const unsigned char *message);
void rtmidi_control_change_message_set_number(unsigned char *message, int number);
int rtmidi_control_change_message_get_value(const unsigned char *message);
void rtmidi_control_change_message_set_value(unsigned char *message, int value);

int rtmidi_program_change_message_get_channel(const unsigned char *message);
void rtmidi_program_change_message_set_channel(unsigned char *message, int channel);
int rtmidi_program_change_message_get_number(const unsigned char *message);
void rtmidi_program_change_message_set_number(unsigned char *message, int number);

int rtmidi_channel_pressure_message_get_channel(const unsigned char *message);
void rtmidi_channel_pressure_message_set_channel(unsigned char *message, int channel);
int rtmidi_channel_pressure_message_get_amount(const unsigned char *message);
void rtmidi_channel_pressure_message_set_amount(unsigned char *message, int amount);

int rtmidi_pitch_wheel_message_get_channel(const unsigned char *message);
void rtmidi_pitch_wheel_message_set_channel(unsigned char *message, int channel);
int rtmidi_pitch_wheel_message_get_value(const unsigned char *message);
void rtmidi_pitch_wheel_message_set_value(unsigned char *message, int value);

#ifdef __cplusplus
}
#endif

#endif
