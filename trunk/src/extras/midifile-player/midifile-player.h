#ifndef MIDI_FILE_PLAYER_INCLUDED
#define MIDI_FILE_PLAYER_INCLUDED

#include <midifile.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MidiFilePlayer *MidiFilePlayer_t;

MidiFilePlayer_t MidiFilePlayer_new(MidiFileEventVisitorCallback_t visitor_callback, void *user_data);
int MidiFilePlayer_free(MidiFilePlayer_t player);

int MidiFilePlayer_setMidiFile(MidiFilePlayer_t player, MidiFile_t midi_file);
MidiFile_t MidiFilePlayer_getMidiFile(MidiFilePlayer_t player);

int MidiFilePlayer_play(MidiFilePlayer_t player);
int MidiFilePlayer_pause(MidiFilePlayer_t player);
int MidiFilePlayer_isRunning(MidiFilePlayer_t player);

long MidiFilePlayer_getTick(MidiFilePlayer_t player);
int MidiFilePlayer_setTick(MidiFilePlayer_t player, long tick);

#ifdef __cplusplus
}
#endif

#endif
