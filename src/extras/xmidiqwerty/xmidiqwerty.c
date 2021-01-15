
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <X11/Xlib.h>
#define XK_LATIN1
#include <X11/keysymdef.h>
#undef XK_LATIN1

#include "midimsg.h"

#ifndef STDIN_FD
#define STDIN_FD 0
#endif

static Display *display;
static int pressed_note;

void XFakeKeypress(Display *display, int keysym)
{
   XKeyEvent event;
   Window current_focus_window;
   int current_focus_revert;

   XGetInputFocus(/* display = */ display, /* focus_return = */ 
      &current_focus_window, /* revert_to_return = */ &current_focus_revert);
       
   event.type = /* (const) */ KeyPress;
   event.display = display;
   event.window = current_focus_window;
   event.root = DefaultRootWindow(/* display = */ display);
   event.subwindow = /* (const) */ None;
   event.time = 1000 * time(/* tloc = */ NULL);
   event.x = 0;
   event.y = 0;
   event.x_root = 0;
   event.y_root = 0;
   event.state = /* (const) */ ShiftMask;
   event.keycode = XKeysymToKeycode(/* display = */ display, 
      /* keysym = */ keysym);
   event.same_screen = /* (const) */ True;

   XSendEvent(/* display = */ display, /* w = (const) */ InputFocus,
      /* propagate = (const) */ True, /* event_mask = (const) */ 
      KeyPressMask, /* event_send = */ (XEvent *)(&event));

   event.type = /* (const) */ KeyRelease;
   event.time = 1000 * time(/* tloc = */ NULL);
   
   XSendEvent(/* display = */ display, /* w = (const) */ InputFocus,
      /* propagate = (const) */ True, /* event_mask = (const) */ 
      KeyReleaseMask, /* event_send = */ (XEvent *)(&event));
}

void handleAlarm(int signum)
{
   /* Map the 26 notes starting at middle C to the letters of the alphabet. */

   if ((60 <= pressed_note) && (pressed_note <= 85))
   {
      XFakeKeypress(/* display = (global) */ display, /* keysym = */ 
         XK_A + pressed_note - 60);
      XFlush(/* display = (global) */ display);
   }

   /*
   ** Under Linux a signal is reset to its default behaviour when raised.  
   ** The following keeps the custom handler in place instead. 
   */

   signal(/* signum = */ SIGALRM, /* handler = */ handleAlarm);
}

int main(int argc, char *argv[])
{
   Byte midi_message[3];
   struct itimerval timer_settings;

   signal(/* signum = */ SIGALRM, /* handler = */ handleAlarm);

   /* output will go to the display set in the DISPLAY environment variable */
   display = XOpenDisplay(/* display_name = */ NULL);

   /*
   ** This should eventually be user-adjustable.  For my style of piano 
   ** playing, 50ms is a good cutoff point for the how close in time two
   ** consecutive notes have to be to be treated as a chord.
   */
   timer_settings.it_interval.tv_sec = 0;
   timer_settings.it_interval.tv_usec = 0;
   timer_settings.it_value.tv_sec = 0;
   timer_settings.it_value.tv_usec = 50000;

   while (True)
   {
      midimsgRead(/* fd = */ STDIN_FD, /* message_return = */ midi_message);

      if (midimsgGetMessageType(midi_message) == MIDIMSG_NOTE_ON) 
      {
         pressed_note = midimsgGetPitch(midi_message);
         setitimer(/* which = */ ITIMER_REAL, /* value = */ &timer_settings,
            /* ovalue = */ NULL);
      }
   }
 
   XCloseDisplay(/* display = */ display);

   return EXIT_SUCCESS;
}

