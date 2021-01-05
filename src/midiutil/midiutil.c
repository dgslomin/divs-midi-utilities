
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

#include <rtmidi_c.h>
#include <midiutil.h>

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

typedef enum
{
	MIDI_UTIL_ALARM_STATE_IDLE,
	MIDI_UTIL_ALARM_STATE_ACTIVE,
	MIDI_UTIL_ALARM_STATE_START_SHUTDOWN,
	MIDI_UTIL_ALARM_STATE_FINISH_SHUTDOWN
} MidiUtilAlarmState_t;

struct MidiUtilAlarm
{
	MidiUtilLock_t lock;
	long end_time_msecs;
	void (*callback)(void *user_data);
	void *user_data;
	MidiUtilAlarmState_t state;
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

static void alarm_helper(void *user_data)
{
	MidiUtilAlarm_t alarm = (MidiUtilAlarm_t)(user_data);
	int should_shutdown = 0;

	while (!should_shutdown)
	{
		MidiUtilLock_lock(alarm->lock);

		switch (alarm->state)
		{
			case MIDI_UTIL_ALARM_STATE_IDLE:
			{
				MidiUtilLock_wait(alarm->lock, -1);
				break;
			}
			case MIDI_UTIL_ALARM_STATE_ACTIVE:
			{
				long msecs = alarm->end_time_msecs - MidiUtil_getCurrentTimeMsecs();

				if (msecs > 0)
				{
					MidiUtilLock_wait(alarm->lock, msecs);
				}
				else
				{
					(alarm->callback)(alarm->user_data);
					alarm->state = MIDI_UTIL_ALARM_STATE_IDLE;
				}

				break;
			}
			case MIDI_UTIL_ALARM_STATE_START_SHUTDOWN:
			{
				should_shutdown = 1;
				alarm->state = MIDI_UTIL_ALARM_STATE_FINISH_SHUTDOWN;
				MidiUtilLock_notify(alarm->lock);
				break;
			}
			default:
			{
				break;
			}
		}

		MidiUtilLock_unlock(alarm->lock);
	}
}

MidiUtilAlarm_t MidiUtilAlarm_new(void)
{
	MidiUtilAlarm_t alarm = (MidiUtilAlarm_t)(malloc(sizeof(struct MidiUtilAlarm)));
	alarm->lock = MidiUtilLock_new();
	alarm->end_time_msecs = 0;
	alarm->callback = NULL;
	alarm->user_data = NULL;
	alarm->state = MIDI_UTIL_ALARM_STATE_IDLE;
	MidiUtil_startThread(alarm_helper, alarm);
	return alarm;
}

void MidiUtilAlarm_set(MidiUtilAlarm_t alarm, long msecs, void (*callback)(void *user_data), void *user_data)
{
	MidiUtilLock_lock(alarm->lock);
	alarm->end_time_msecs = MidiUtil_getCurrentTimeMsecs() + msecs;
	alarm->callback = callback;
	alarm->user_data = user_data;
	alarm->state = MIDI_UTIL_ALARM_STATE_ACTIVE;
	MidiUtilLock_notify(alarm->lock);
	MidiUtilLock_unlock(alarm->lock);
}

void MidiUtilAlarm_cancel(MidiUtilAlarm_t alarm)
{
	MidiUtilLock_lock(alarm->lock);
	alarm->state = MIDI_UTIL_ALARM_STATE_IDLE;
	MidiUtilLock_notify(alarm->lock);
	MidiUtilLock_unlock(alarm->lock);
}

void MidiUtilAlarm_free(MidiUtilAlarm_t alarm)
{
	MidiUtilLock_lock(alarm->lock);
	alarm->state = MIDI_UTIL_ALARM_STATE_START_SHUTDOWN;
	MidiUtilLock_notify(alarm->lock);
	MidiUtilLock_unlock(alarm->lock);

	MidiUtilLock_lock(alarm->lock);
	while (alarm->state != MIDI_UTIL_ALARM_STATE_FINISH_SHUTDOWN) MidiUtilLock_wait(alarm->lock, -1);
	MidiUtilLock_unlock(alarm->lock);

	MidiUtilLock_free(alarm->lock);
	free(alarm);
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

static MidiUtilLock_t wait_for_interrupt_lock;
static int was_interrupted;

static void wait_for_interrupt_helper(void *user_data)
{
	MidiUtilLock_lock(wait_for_interrupt_lock);
	was_interrupted = 1;
	MidiUtilLock_notify(wait_for_interrupt_lock);
	MidiUtilLock_unlock(wait_for_interrupt_lock);
}

void MidiUtil_waitForInterrupt(void)
{
	wait_for_interrupt_lock = MidiUtilLock_new();
	was_interrupted = 0;
	MidiUtil_setInterruptHandler(wait_for_interrupt_helper, NULL);
	MidiUtilLock_lock(wait_for_interrupt_lock);
	while (!was_interrupted) MidiUtilLock_wait(wait_for_interrupt_lock, -1);
	MidiUtilLock_unlock(wait_for_interrupt_lock);
	MidiUtil_setInterruptHandler(NULL, NULL);
	MidiUtilLock_free(wait_for_interrupt_lock);
}

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

