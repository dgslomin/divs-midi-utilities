#ifndef SCHEDULER_INCLUDED
#define SCHEDULER_INCLUDED

G_BEGIN_DECLS

typedef struct 
{
	gulong interval;
	void (*callback)(gpointer user_data);
	gpointer user_data;
	GThread *thread;
}
Scheduler;

/* the interval is in microseconds */

Scheduler *scheduler_new(gulong interval, void (*callback)(gpointer user_data), gpointer user_data);

G_END_DECLS

#endif
