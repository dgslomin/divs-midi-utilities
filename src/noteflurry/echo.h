#ifndef ECHO_INCLUDED
#define ECHO_INCLUDED

void echo_init(void);
void echo_cleanup(void);
void echo_note_on(long current_time_msecs, int channel, int note, int velocity);
void echo_note_off(long current_time_msecs, int channel, int note, int velocity);
void echo_update_players(long current_time_msecs);

#endif
