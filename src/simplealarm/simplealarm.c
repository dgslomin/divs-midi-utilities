
#include <stdlib.h>
#include <windows.h>
#include <simplealarm.h>

enum SimpleAlarmState
{
	SIMPLE_ALARM_STATE_INITIAL,
	SIMPLE_ALARM_STATE_WAIT_FOR_TIMEOUT,
	SIMPLE_ALARM_STATE_DO_CALLBACK,
	SIMPLE_ALARM_STATE_SHUTDOWN
};

typedef enum SimpleAlarmState SimpleAlarmState_t;

struct SimpleAlarm
{
	int timeout;
	void (*callback)(void *user_data);
	void *user_data;
	HANDLE mutex;
	HANDLE event;
	SimpleAlarmState_t state;
};

static SimpleAlarmState_t SimpleAlarm_getState(SimpleAlarm_t alarm)
{
	SimpleAlarmState_t state;

	WaitForSingleObject(alarm->mutex, INFINITE);
	state = alarm->state;
	ReleaseMutex(alarm->mutex);

	return state;
}

static void SimpleAlarm_setState(SimpleAlarm_t alarm, SimpleAlarmState_t state)
{
	WaitForSingleObject(alarm->mutex, INFINITE);
	alarm->state = state;
	ReleaseMutex(alarm->mutex);
}

static void SimpleAlarm_setStateAndNotify(SimpleAlarm_t alarm, SimpleAlarmState_t state)
{
	WaitForSingleObject(alarm->mutex, INFINITE);
	alarm->state = state;
	PulseEvent(alarm->event);
	ReleaseMutex(alarm->mutex);
}

static DWORD WINAPI SimpleAlarm_threadMain(LPVOID user_data)
{
	SimpleAlarm_t alarm = (SimpleAlarm_t)(user_data);

	while (1)
	{
		switch (SimpleAlarm_getState(alarm))
		{
			case SIMPLE_ALARM_STATE_INITIAL:
			{
				WaitForSingleObject(alarm->event, INFINITE);
				break;
			}
			case SIMPLE_ALARM_STATE_WAIT_FOR_TIMEOUT:
			{
				SimpleAlarm_setState(alarm, SIMPLE_ALARM_STATE_DO_CALLBACK);
				WaitForSingleObject(alarm->event, alarm->timeout);
				break;
			}
			case SIMPLE_ALARM_STATE_DO_CALLBACK:
			{
				(alarm->callback)(alarm->user_data);
				SimpleAlarm_setState(alarm, SIMPLE_ALARM_STATE_INITIAL);
				break;
			}
			case SIMPLE_ALARM_STATE_SHUTDOWN:
			{
				CloseHandle(alarm->mutex);
				CloseHandle(alarm->event);
				free(alarm);
				return 0;
			}
		}
	}

	return 0;
}

SimpleAlarm_t SimpleAlarm_new(void)
{
	SimpleAlarm_t alarm;
	DWORD thread_id;

	alarm = (SimpleAlarm_t)(malloc(sizeof(struct SimpleAlarm)));
	alarm->timeout = 0;
	alarm->callback = NULL;
	alarm->user_data = NULL;
	alarm->mutex = CreateMutex(NULL, FALSE, NULL);
	alarm->event = CreateEvent(NULL, FALSE, FALSE, NULL);
	alarm->state = SIMPLE_ALARM_STATE_INITIAL;
	CreateThread(NULL, 0, SimpleAlarm_threadMain, alarm, 0, &thread_id);

	return alarm;
}

int SimpleAlarm_set(SimpleAlarm_t alarm, int timeout, void (*callback)(void *user_data), void *user_data)
{
	if ((alarm == NULL) || (timeout <= 0) || (callback == NULL)) return -1;

	alarm->timeout = timeout;
	alarm->callback = callback;
	alarm->user_data = user_data;
	SimpleAlarm_setStateAndNotify(alarm, SIMPLE_ALARM_STATE_WAIT_FOR_TIMEOUT);

	return 0;
}

int SimpleAlarm_cancel(SimpleAlarm_t alarm)
{
	if (alarm == NULL) return -1;

	SimpleAlarm_setStateAndNotify(alarm, SIMPLE_ALARM_STATE_INITIAL);

	return 0;
}

int SimpleAlarm_free(SimpleAlarm_t alarm)
{
	if (alarm == NULL) return -1;

	SimpleAlarm_setStateAndNotify(alarm, SIMPLE_ALARM_STATE_SHUTDOWN);

	return 0;
}

