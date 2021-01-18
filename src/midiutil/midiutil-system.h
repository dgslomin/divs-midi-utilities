#ifndef MIDIUTIL_SYSTEM_INCLUDED
#define MIDIUTIL_SYSTEM_INCLUDED

/* Common helpers that have system dependencies. */

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MidiUtilLock *MidiUtilLock_t;
typedef struct MidiUtilAlarm *MidiUtilAlarm_t;

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

void MidiUtil_setInterruptHandler(void (*callback)(void *user_data), void *user_data);
void MidiUtil_waitForExit(void (*callback)(void *user_data), void *user_data);

MidiUtilAlarm_t MidiUtilAlarm_new(void);
void MidiUtilAlarm_free(MidiUtilAlarm_t alarm);
void MidiUtilAlarm_set(MidiUtilAlarm_t alarm, long msecs, void (*callback)(int cancelled, void *user_data), void *user_data);
void MidiUtilAlarm_add(MidiUtilAlarm_t alarm, long msecs, void (*callback)(int cancelled, void *user_data), void *user_data);
void MidiUtilAlarm_cancel(MidiUtilAlarm_t alarm);

#ifdef __cplusplus
}
#endif

#endif
