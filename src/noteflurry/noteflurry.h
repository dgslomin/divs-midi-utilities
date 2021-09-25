#ifndef NOTEFLURRY_INCLUDED
#define NOTEFLURRY_INCLUDED

struct Player
{
	MidiFileEvent_t event;
	long start_time_msecs;
	long stop_time_msecs;
	int base_channel;
	int base_note;
	int base_velocity;
};

typedef struct Player *Player_t;

extern MidiFile_t midi_file;
extern float tempo_bpm;
extern int note_velocity[16][128];
extern int note_sustain[16][128];
extern int channel_sustain[16];

void send_note_on(int channel, int note, int velocity);
void send_note_off(int channel, int note, int velocity);

#endif
