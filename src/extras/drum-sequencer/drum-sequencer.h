
typedef struct DrumSequencer *DrumSequencer_t;
typedef struct Port *Port_t;
typedef struct Instrument *Instrument_t;
typedef struct Pattern *Pattern_t;
typedef struct Line *Line_t;
typedef struct Trigger *Trigger_t;

DrumSequencer_t DrumSequencer_new(void);
void DrumSequencer_free(DrumSequencer_t sequencer);

void DrumSequencer_setNumberOfBeats(DrumSequencer_t sequencer, int number_of_beats);
void DrumSequencer_setTriggerPortAndChannel(DrumSequencer_t sequencer, char *port_name, int channel);
void DrumSequencer_addPort(DrumSequencer_t sequencer, char *name);
void DrumSequencer_addInstrument(DrumSequencer_t drum_sequencer, char *name, int channel, int note, int velocity);
void DrumSequencer_addPattern(DrumSequencer_t sequencer, char *name, int number_of_steps);
void DrumSequencer_addLine(DrumSequencer_t sequencer, char *instrument_name);
void DrumSequencer_addEvent(DrumSequencer_t sequencer, int event);
void DrumSequencer_addTrigger(DrumSequencer_t sequencer, int note, char *pattern_name);

DrumSequencer_t DrumSequencer_load(char *filename);

void DrumSequencer_run(DrumSequencer_t sequencer);
void DrumSequencer_stop(DrumSequencer_t sequencer);

