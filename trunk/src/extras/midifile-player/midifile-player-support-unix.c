
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <midifile-player-support.h>

struct MidiFilePlayerWaitLock
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

MidiFilePlayerWaitLock_t MidiFilePlayerWaitLock_new(void)
{
	MidiFilePlayerWaitLock_t wait_lock = (MidiFilePlayerWaitLock_t)(malloc(sizeof (struct MidiFilePlayerWaitLock)));
	pthread_mutex_init(&(wait_lock->mutex), NULL);
	pthread_cond_init(&(wait_lock->cond), NULL);
	return wait_lock;
}

void MidiFilePlayerWaitLock_free(MidiFilePlayerWaitLock_t wait_lock)
{
	pthread_cond_destroy(&(wait_lock->cond));
	pthread_mutex_destroy(&(wait_lock->mutex));
	free(wait_lock);
}

void MidiFilePlayerWaitLock_wait(MidiFilePlayerWaitLock_t wait_lock, long time)
{
	struct timeval current_time;
	struct timespec target_time;
	pthread_mutex_lock(&(wait_lock->mutex));
	gettimeofday(&current_time, NULL);
	target_time.tv_sec = current_time.tv_sec + (time / 1000);
	target_time.tv_nsec = (current_time.tv_usec + (((time + 1000) % 1000) * 1000)) * 1000;
	pthread_cond_timedwait(&(wait_lock->cond), &(wait_lock->mutex), &target_time);
	pthread_mutex_unlock(&(wait_lock->mutex));
}

void MidiFilePlayerWaitLock_notify(MidiFilePlayerWaitLock_t wait_lock)
{
	pthread_mutex_lock(&(wait_lock->mutex));
	pthread_cond_signal(&(wait_lock->cond));
	pthread_mutex_unlock(&(wait_lock->mutex));
}

void MidiFilePlayer_startThread(void *(*callback)(void *arg), void *arg)
{
	pthread_t thread;
	pthread_create(&thread, NULL, callback, arg);
	pthread_detach(thread);
}

long MidiFilePlayer_getCurrentTime(void)
{
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	return ((long)(current_time.tv_sec) * 1000) + (long)(current_time.tv_usec / 1000);
}

