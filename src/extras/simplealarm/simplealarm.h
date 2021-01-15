#ifndef SIMPLE_ALARM_INCLUDED
#define SIMPLE_ALARM_INCLUDED

/* A simple resettable alarm.  Invokes a callback in its own thread after a specified number of milliseconds. */

typedef struct SimpleAlarm *SimpleAlarm_t;

SimpleAlarm_t SimpleAlarm_new(void);
int SimpleAlarm_set(SimpleAlarm_t alarm, int timeout, void (*callback)(void *user_data), void *user_data);
int SimpleAlarm_cancel(SimpleAlarm_t alarm);
int SimpleAlarm_free(SimpleAlarm_t alarm);

#endif
