#ifndef BEATBOX_INCLUDED
#define BEATBOX_INCLUDED

typedef struct beatbox *Beatbox_t;

Beatbox_t Beatbox_new(int number_of_voices);
void Beatbox_free(Beatbox_t beatbox);

void Beatbox_setInstrumentWaveform(Beatbox_t beatbox, char *instrument_name, char *waveform_filename);
void Beatbox_setNoteInstrument(Beatbox_t beatbox, int note, char *instrument_name);
void Beatbox_setNoteCutoffGroup(Beatbox_t beatbox, int note, char *cutoff_group_name);
void Beatbox_loadConfig(Beatbox_t beatbox, char *filename);

void Beatbox_noteOn(Beatbox_t beatbox, int note, int velocity);
void Beatbox_computeAudio(Beatbox_t beatbox, float *data, unsigned long frames);

#endif
