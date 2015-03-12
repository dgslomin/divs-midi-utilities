
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile-player-support.h>

struct MidiFilePlayerWaitLock
{
	HANDLE event;
};

MidiFilePlayerWaitLock_t MidiFilePlayerWaitLock_new(void)
{
	MidiFilePlayerWaitLock_t wait_lock = (MidiFilePlayerWaitLock_t)(malloc(sizeof (struct MidiFilePlayerWaitLock)));
	wait_lock->event = CreateEvent(NULL, FALSE, FALSE, NULL);
	return wait_lock;
}

void MidiFilePlayerWaitLock_free(MidiFilePlayerWaitLock_t wait_lock)
{
	CloseHandle(wait_lock->event);
	free(wait_lock);
}

void MidiFilePlayerWaitLock_wait(MidiFilePlayerWaitLock_t wait_lock, long time)
{
	WaitForSingleObject(wait_lock->event, time);
}

void MidiFilePlayerWaitLock_notify(MidiFilePlayerWaitLock_t wait_lock)
{
	PulseEvent(wait_lock->event);
}

void MidiFilePlayer_startThread(void *(*callback)(void *arg), void *arg)
{
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(callback), arg, 0, NULL);
}

long MidiFilePlayer_getCurrentTime(void)
{
	return (long)(timeGetTime());
}

