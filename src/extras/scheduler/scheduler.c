
#include <glib.h>
#include "scheduler.h"

static gpointer scheduler_thread_main(gpointer user_data)
{
	Scheduler *scheduler = (Scheduler *)(user_data);

	while (TRUE)
	{
		(scheduler->callback)(scheduler->user_data);
		g_usleep(scheduler->interval);
	}

	return NULL;
}

Scheduler *scheduler_new(gulong interval, void (*callback)(gpointer user_data), gpointer user_data)
{
	Scheduler *scheduler = g_new(Scheduler, 1);
	scheduler->interval = interval;
	scheduler->callback = callback;
	scheduler->user_data = user_data;
	scheduler->thread = g_thread_create_full((GThreadFunc)(scheduler_thread_main), scheduler, 0, TRUE, TRUE, G_THREAD_PRIORITY_URGENT, NULL);
	return scheduler;
}

