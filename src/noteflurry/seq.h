#ifndef SEQ_INCLUDED
#define SEQ_INCLUDED

void seq_init(void);
void seq_cleanup(void);
void seq_note_on(long current_time_msecs, int channel, int note, int velocity);
void seq_note_off(long current_time_msecs, int channel, int note, int velocity);
void seq_update_players(long current_time_msecs);

#endif
