#ifndef MULTISEQ_INCLUDED
#define MULTISEQ_INCLUDED

void multiseq_init(void);
void multiseq_cleanup(void);
void multiseq_note_on(long current_time_msecs, int channel, int note, int velocity);
void multiseq_note_off(long current_time_msecs, int channel, int note, int velocity);
void multiseq_update_players(long current_time_msecs);

#endif
