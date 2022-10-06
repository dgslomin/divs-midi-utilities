
#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif

#include <midiutil-common.h>
#include <midiutil-system.h>

struct MidiUtilLock
{
#ifdef _WIN32
	CRITICAL_SECTION critical_section;
	CONDITION_VARIABLE condition_variable;
#else
	pthread_mutex_t mutex;
	pthread_cond_t cond;
#endif
};

struct MidiUtilAlarm
{
	MidiUtilLock_t lock;
	MidiUtilLongArray_t end_time_msecs_array;
	MidiUtilPointerArray_t callback_array;
	MidiUtilPointerArray_t user_data_array;
	int start_shutdown;
	int finish_shutdown;
};

void MidiUtil_startThread(void (*callback)(void *user_data), void *user_data)
{
#ifdef _WIN32
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(callback), user_data, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, (void *(*)(void *user_data))(callback), user_data);
	pthread_detach(thread);
#endif
}

MidiUtilLock_t MidiUtilLock_new(void)
{
	MidiUtilLock_t lock = (MidiUtilLock_t)(malloc(sizeof (struct MidiUtilLock)));
#ifdef _WIN32
	InitializeCriticalSection(&(lock->critical_section));
	InitializeConditionVariable(&(lock->condition_variable));
#else
	pthread_mutex_init(&(lock->mutex), NULL);
	pthread_cond_init(&(lock->cond), NULL);
#endif
	return lock;
}

void MidiUtilLock_free(MidiUtilLock_t lock)
{
#ifdef _WIN32
	DeleteCriticalSection(&(lock->critical_section));
#else
	pthread_cond_destroy(&(lock->cond));
	pthread_mutex_destroy(&(lock->mutex));
#endif
	free(lock);
}

void MidiUtilLock_lock(MidiUtilLock_t lock)
{
#ifdef _WIN32
	EnterCriticalSection(&(lock->critical_section));
#else
	pthread_mutex_lock(&(lock->mutex));
#endif
}

void MidiUtilLock_unlock(MidiUtilLock_t lock)
{
#ifdef _WIN32
	LeaveCriticalSection(&(lock->critical_section));
#else
	pthread_mutex_unlock(&(lock->mutex));
#endif
}

void MidiUtilLock_wait(MidiUtilLock_t lock, long timeout_msecs)
{
#ifdef _WIN32
	SleepConditionVariableCS(&(lock->condition_variable), &(lock->critical_section), (timeout_msecs < 0) ? INFINITE : timeout_msecs);
#else
	if (timeout_msecs < 0)
	{
		pthread_cond_wait(&(lock->cond), &(lock->mutex));
	}
	else
	{
		struct timeval current_time;
		struct timespec target_time;

		gettimeofday(&current_time, NULL);
		target_time.tv_sec = current_time.tv_sec + (timeout_msecs / 1000);
		target_time.tv_nsec = (current_time.tv_usec + (((timeout_msecs + 1000) % 1000) * 1000)) * 1000;
		pthread_cond_timedwait(&(lock->cond), &(lock->mutex), &target_time);
	}
#endif
}

void MidiUtilLock_notify(MidiUtilLock_t lock)
{
#ifdef _WIN32
	WakeConditionVariable(&(lock->condition_variable));
#else
	pthread_cond_signal(&(lock->cond));
#endif
}

void MidiUtilLock_notifyAll(MidiUtilLock_t lock)
{
#ifdef _WIN32
	WakeAllConditionVariable(&(lock->condition_variable));
#else
	pthread_cond_broadcast(&(lock->cond));
#endif
}

void MidiUtil_sleep(long msecs)
{
#ifdef _WIN32
	Sleep(msecs);
#else
	usleep(msecs * 1000);
#endif
}

long MidiUtil_getCurrentTimeMsecs(void)
{
#ifdef _WIN32
	return (long)(timeGetTime());
#else
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	return ((long)(current_time.tv_sec) * 1000) + (long)(current_time.tv_usec / 1000);
#endif
}

void MidiUtil_getCurrentTimeString(char *current_time_string)
{
#ifdef _WIN32
	SYSTEMTIME current_time;
	GetLocalTime(&current_time);
	sprintf(current_time_string, "%d%02d%02d%02d%02d%02d", current_time.wYear, current_time.wMonth, current_time.wDay, current_time.wHour, current_time.wMinute, current_time.wSecond);
#else
	time_t current_time;
	struct tm *current_time_struct;
	current_time = time(NULL);
	current_time_struct = localtime(&current_time);
	strftime(current_time_string, 15, "%Y%m%d%H%M%S", current_time_struct);
#endif
}

static void (*interrupt_handler_callback)(void *user_data) = NULL;
static void *interrupt_handler_user_data = NULL;

#ifdef _WIN32
static BOOL WINAPI interrupt_handler_helper(DWORD control_type)
{
	interrupt_handler_callback(interrupt_handler_user_data);
	return TRUE;
}
#else
static void interrupt_handler_helper(int signal_number)
{
	interrupt_handler_callback(interrupt_handler_user_data);
	signal(SIGINT, interrupt_handler_helper);
}
#endif

void MidiUtil_setInterruptHandler(void (*callback)(void *user_data), void *user_data)
{
	interrupt_handler_callback = callback;
	interrupt_handler_user_data = user_data;

#ifdef _WIN32
	SetConsoleCtrlHandler(interrupt_handler_helper, (callback == NULL) ? FALSE : TRUE);
#else
	signal(SIGINT, (callback == NULL) ? SIG_DFL : interrupt_handler_helper);
#endif
}

/*
 * The ugly asymmetry here comes from the fact that Windows requires cleanup to
 * be done in the console control handler callback since it won't return to the
 * main thread in some cases, whereas Unix won't allow cleanup to be done in
 * the signal handler callback.
 */

static MidiUtilLock_t wait_for_exit_lock;
static int wait_for_exit_done = 0;

#ifdef _WIN32

static void (*wait_for_exit_callback)(void *user_data) = NULL;
static void *wait_for_exit_user_data = NULL;

static BOOL WINAPI wait_for_exit_helper(DWORD control_type)
{
	if (wait_for_exit_callback != NULL) wait_for_exit_callback(wait_for_exit_user_data);
	MidiUtilLock_lock(wait_for_exit_lock);
	wait_for_exit_done = 1;
	MidiUtilLock_notify(wait_for_exit_lock);
	MidiUtilLock_unlock(wait_for_exit_lock);
	return TRUE;
}

void MidiUtil_waitForExit(void (*callback)(void *user_data), void *user_data)
{
	wait_for_exit_lock = MidiUtilLock_new();
	wait_for_exit_callback = callback;
	wait_for_exit_user_data = user_data;
	SetConsoleCtrlHandler(wait_for_exit_helper, TRUE);
	MidiUtilLock_lock(wait_for_exit_lock);
	while (!wait_for_exit_done) MidiUtilLock_wait(wait_for_exit_lock, -1);
	MidiUtilLock_unlock(wait_for_exit_lock);
	MidiUtilLock_free(wait_for_exit_lock);
}

#else

static void wait_for_exit_helper(int signal_number)
{
	MidiUtilLock_lock(wait_for_exit_lock);
	wait_for_exit_done = 1;
	MidiUtilLock_notify(wait_for_exit_lock);
	MidiUtilLock_unlock(wait_for_exit_lock);
}

void MidiUtil_waitForExit(void (*callback)(void *user_data), void *user_data)
{
	wait_for_exit_lock = MidiUtilLock_new();
	signal(SIGINT, wait_for_exit_helper);
	MidiUtilLock_lock(wait_for_exit_lock);
	while (!wait_for_exit_done) MidiUtilLock_wait(wait_for_exit_lock, -1);
	MidiUtilLock_unlock(wait_for_exit_lock);
	if (callback != NULL) callback(user_data);
	MidiUtilLock_free(wait_for_exit_lock);
}

#endif

static void alarm_helper(void *user_data)
{
	MidiUtilAlarm_t alarm = (MidiUtilAlarm_t)(user_data);
	int should_shutdown = 0;

	while (!should_shutdown)
	{
		void (*callback)(int cancelled, void *user_data) = NULL;
		void *user_data = NULL;

		MidiUtilLock_lock(alarm->lock);

		if (alarm->start_shutdown)
		{
			should_shutdown = 1;
			alarm->finish_shutdown = 1;
			MidiUtilLock_notify(alarm->lock);
		}
		else if (MidiUtilPointerArray_getSize(alarm->callback_array) > 0)
		{
			long msecs = MidiUtilLongArray_get(alarm->end_time_msecs_array, 0) - MidiUtil_getCurrentTimeMsecs();

			if (msecs > 0)
			{
				MidiUtilLock_wait(alarm->lock, msecs);
			}
			else
			{
				callback = (void (*)(int cancelled, void *user_data))(MidiUtilPointerArray_get(alarm->callback_array, 0));
				user_data = MidiUtilPointerArray_get(alarm->user_data_array, 0);
				MidiUtilLongArray_remove(alarm->end_time_msecs_array, 0);
				MidiUtilPointerArray_remove(alarm->callback_array, 0);
				MidiUtilPointerArray_remove(alarm->user_data_array, 0);
			}
		}
		else
		{
			MidiUtilLock_wait(alarm->lock, -1);
		}

		MidiUtilLock_unlock(alarm->lock);
		if (callback != NULL) callback(0, user_data);
	}
}

MidiUtilAlarm_t MidiUtilAlarm_new(void)
{
	MidiUtilAlarm_t alarm = (MidiUtilAlarm_t)(malloc(sizeof(struct MidiUtilAlarm)));
	alarm->lock = MidiUtilLock_new();
	alarm->end_time_msecs_array = MidiUtilLongArray_new(128);
	alarm->callback_array = MidiUtilPointerArray_new(128);
	alarm->user_data_array = MidiUtilPointerArray_new(128);
	alarm->start_shutdown = 0;
	alarm->finish_shutdown = 0;
	MidiUtil_startThread(alarm_helper, alarm);
	return alarm;
}

void MidiUtilAlarm_free(MidiUtilAlarm_t alarm)
{
	MidiUtilLock_lock(alarm->lock);
	alarm->start_shutdown = 1;
	MidiUtilLock_notify(alarm->lock);
	MidiUtilLock_unlock(alarm->lock);

	MidiUtilLock_lock(alarm->lock);
	while (!(alarm->finish_shutdown)) MidiUtilLock_wait(alarm->lock, -1);
	MidiUtilLock_unlock(alarm->lock);

	MidiUtilLongArray_free(alarm->end_time_msecs_array);
	MidiUtilPointerArray_free(alarm->callback_array);
	MidiUtilPointerArray_free(alarm->user_data_array);
	MidiUtilLock_free(alarm->lock);
	free(alarm);
}

void MidiUtilAlarm_set(MidiUtilAlarm_t alarm, long msecs, void (*callback)(int cancelled, void *user_data), void *user_data)
{
	long end_time_msecs = MidiUtil_getCurrentTimeMsecs() + msecs;
	int number_of_events, event_number;

	MidiUtilLock_lock(alarm->lock);
	number_of_events = MidiUtilPointerArray_getSize(alarm->callback_array);

	for (event_number = 0; event_number < number_of_events; event_number++)
	{
		((void (*)(int cancelled, void *user_data))(MidiUtilPointerArray_get(alarm->callback_array, event_number)))(1, MidiUtilPointerArray_get(alarm->user_data_array, event_number));
	}

	MidiUtilLongArray_clear(alarm->end_time_msecs_array);
	MidiUtilPointerArray_clear(alarm->callback_array);
	MidiUtilPointerArray_clear(alarm->user_data_array);
	MidiUtilLongArray_add(alarm->end_time_msecs_array, end_time_msecs);
	MidiUtilPointerArray_add(alarm->callback_array, callback);
	MidiUtilPointerArray_add(alarm->user_data_array, user_data);
	MidiUtilLock_notify(alarm->lock);
	MidiUtilLock_unlock(alarm->lock);
}

void MidiUtilAlarm_add(MidiUtilAlarm_t alarm, long msecs, void (*callback)(int cancelled, void *user_data), void *user_data)
{
	long end_time_msecs = MidiUtil_getCurrentTimeMsecs() + msecs;
	int number_of_events, event_number;

	MidiUtilLock_lock(alarm->lock);
	number_of_events = MidiUtilPointerArray_getSize(alarm->callback_array);

	for (event_number = number_of_events - 1; event_number >= 0; event_number--)
	{
		if (end_time_msecs >= MidiUtilLongArray_get(alarm->end_time_msecs_array, event_number)) break;
	}

	MidiUtilLongArray_insert(alarm->end_time_msecs_array, event_number + 1, end_time_msecs);
	MidiUtilPointerArray_insert(alarm->callback_array, event_number + 1, callback);
	MidiUtilPointerArray_insert(alarm->user_data_array, event_number + 1, user_data);
	MidiUtilLock_notify(alarm->lock);
	MidiUtilLock_unlock(alarm->lock);
}

void MidiUtilAlarm_cancel(MidiUtilAlarm_t alarm)
{
	int number_of_events, event_number;

	MidiUtilLock_lock(alarm->lock);
	number_of_events = MidiUtilPointerArray_getSize(alarm->callback_array);

	for (event_number = 0; event_number < number_of_events; event_number++)
	{
		((void (*)(int cancelled, void *user_data))(MidiUtilPointerArray_get(alarm->callback_array, event_number)))(1, MidiUtilPointerArray_get(alarm->user_data_array, event_number));
	}

	MidiUtilLongArray_clear(alarm->end_time_msecs_array);
	MidiUtilPointerArray_clear(alarm->callback_array);
	MidiUtilPointerArray_clear(alarm->user_data_array);
	MidiUtilLock_notify(alarm->lock);
	MidiUtilLock_unlock(alarm->lock);
}

