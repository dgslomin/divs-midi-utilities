#ifndef MIDI_FILE_PLAYER_SUPPORT_INCLUDED
#define MIDI_FILE_PLAYER_SUPPORT_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MidiFilePlayerWaitLock *MidiFilePlayerWaitLock_t;

MidiFilePlayerWaitLock_t MidiFilePlayerWaitLock_new(void);
void MidiFilePlayerWaitLock_free(MidiFilePlayerWaitLock_t wait_lock);
void MidiFilePlayerWaitLock_wait(MidiFilePlayerWaitLock_t wait_lock, long time);
void MidiFilePlayerWaitLock_notify(MidiFilePlayerWaitLock_t wait_lock);
void MidiFilePlayer_startThread(void *(*callback)(void *arg), void *arg);
long MidiFilePlayer_getCurrentTime(void);

#ifdef __cplusplus
}
#endif

#endif
