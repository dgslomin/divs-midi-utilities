#ifndef PEGGY_INCLUDED
#define PEGGY_INCLUDED

// A mostly traditional arpeggiator.

typedef struct Peggy *Peggy_t;

typedef enum
{
	PEGGY_MODE_UP,
	PEGGY_MODE_DOWN,
	PEGGY_MODE_UP_DOWN_INC,
	PEGGY_MODE_UP_DOWN_EXC,
	PEGGY_MODE_DOWN_UP_INC,
	PEGGY_MODE_DOWN_UP_EXC,
	PEGGY_MODE_ORDER,
	PEGGY_MODE_RANDOM,
	PEGGY_MODE_CHORD,
}
PeggyMode_t;

Peggy_t Peggy_new(void);
void Peggy_free(Peggy_t peggy);

void Peggy_setUserData(Peggy_t peggy, void *user_data);
void *Peggy_getUserData(Peggy_t peggy);

void Peggy_resetTransport(Peggy_t peggy);
void Peggy_setTempo(Peggy_t peggy, float tempo);
float Peggy_getTempo(Peggy_t peggy);
void Peggy_tapTempo(Peggy_t peggy);

void Peggy_setStepsPerQuarterNote(Peggy_t peggy, float steps_per_quarter_note);
float Peggy_getStepsPerQuarterNote(Peggy_t peggy);

void Peggy_setMode(Peggy_t peggy, PeggyMode_t mode);
PeggyMode_t Peggy_getMode(Peggy_t peggy);

void Peggy_setOctaves(Peggy_t peggy, int octaves);
int Peggy_getOctaves(Peggy_t peggy);

void Peggy_setSwing(Peggy_t peggy, float swing);
float Peggy_getSwing(Peggy_t peggy);

void Peggy_setDutyCycle(Peggy_t peggy, float duty_cycle);
float Peggy_getDutyCycle(Peggy_t peggy);

void Peggy_setFreeRunning(Peggy_t peggy, int free_running);
int Peggy_getFreeRunning(Peggy_t peggy);

void Peggy_setHold(Peggy_t peggy_t, int hold);
int Peggy_getHold(Peggy_t peggy_t);

void Peggy_setFixedLength(Peggy_t peggy, int fixed_length);
int Peggy_getFixedLength(Peggy_t peggy);

void Peggy_setNoteOnHandler(Peggy_t peggy, void (*note_on_handler)(Peggy_t peggy, int note));
void Peggy_setNoteOffHandler(Peggy_t peggy, void (*note_off_handler)(Peggy_t peggy, int note));

void Peggy_noteOn(Peggy_t peggy, int note);
void Peggy_noteOff(Peggy_t peggy, int note);

void Peggy_tick(Peggy_t peggy);

#endif
