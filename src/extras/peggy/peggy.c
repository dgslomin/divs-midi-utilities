
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midiutil-system.h>

struct Peggy
{
	void *user_data;
	float tempo;
	float steps_per_quarter_note;
	PeggyMode_t mode;
	int octaves;
	float swing;
	float duty_cycle;
	int free_running;
	int hold;
	int fixed_length;
	void (*note_on_handler)(Peggy_t peggy, int note);
	void (*note_off_handler)(Peggy_t peggy, int note);
	long last_step_time;
	int step_number;
	int number_of_input_notes;
	int input_notes[128];
	int sorted_input_notes[128];
};

Peggy_t Peggy_new(void)
{
	Peggy_t peggy = (Peggy_t)(malloc(sizeof (struct Peggy)));
	peggy->user_data = NULL;
	peggy->tempo = 100;
	peggy->steps_per_quarter_note = 2;
	peggy->mode = PEGGY_MODE_UP;
	peggy->octaves = 1;
	peggy->swing = 0.5;
	peggy->duty_cycle = 1.0;
	peggy->free_running = 1;
	peggy->hold = 0;
	peggy->fixed_length = 0;
	peggy->note_on_handler = NULL;
	peggy->note_off_handler = NULL;
	peggy->last_step_time = 0;
	peggy->step_number = 0;
	peggy->number_of_input_notes = 0;
	Peggy_resetTransport(peggy);
	return peggy;
}

void Peggy_free(Peggy_t peggy)
{
	free(peggy);
}

void Peggy_setUserData(Peggy_t peggy, void *user_data)
{
	peggy->user_data = user_data;
}

void *Peggy_getUserData(Peggy_t peggy)
{
	return peggy->user_data;
}

void Peggy_resetTransport(Peggy_t peggy)
{
	peggy->last_step_time = MidiUtil_getCurrentTimeMsecs() - (long)(60000 / (peggy->steps_per_quarter_note * peggy->tempo));
	peggy->step_number = -1;
}

void Peggy_setTempo(Peggy_t peggy, float tempo)
{
	peggy->tempo = tempo;
}

float Peggy_getTempo(Peggy_t peggy)
{
	return peggy->tempo;
}

void Peggy_tapTempo(Peggy_t peggy)
{
	// TODO
}

void Peggy_setStepsPerQuarterNote(Peggy_t peggy, float steps_per_quarter_note)
{
	peggy->steps_per_quarter_note = steps_per_quarter_note;
}

float Peggy_getStepsPerQuarterNote(Peggy_t peggy)
{
	return peggy->steps_per_quarter_note;
}

void Peggy_setMode(Peggy_t peggy, PeggyMode_t mode)
{
	peggy->mode = mode;
}

PeggyMode_t Peggy_getMode(Peggy_t peggy)
{
	return peggy->mode;
}

void Peggy_setOctaves(Peggy_t peggy, int octaves)
{
	peggy->octaves = octaves;
}

int Peggy_getOctaves(Peggy_t peggy)
{
	return peggy->octaves;
}

void Peggy_setSwing(Peggy_t peggy, float swing)
{
	peggy->swing = swing;
}

float Peggy_getSwing(Peggy_t peggy)
{
	return peggy->swing;
}

void Peggy_setDutyCycle(Peggy_t peggy, float duty_cycle)
{
	peggy->duty_cycle = duty_cycle;
}

float Peggy_getDutyCycle(Peggy_t peggy)
{
	return peggy->duty_cycle;
}

void Peggy_setFreeRunning(Peggy_t peggy, int free_running)
{
	peggy->free_running = free_running;
}

int Peggy_getFreeRunning(Peggy_t peggy)
{
	return peggy->free_running;
}

void Peggy_setHold(Peggy_t peggy_t, int hold)
{
	peggy->hold = hold;
}

int Peggy_getHold(Peggy_t peggy_t)
{
	return peggy->hold;
}

void Peggy_setFixedLength(Peggy_t peggy, int fixed_length)
{
	peggy->fixed_length = fixed_length;
}

int Peggy_getFixedLength(Peggy_t peggy)
{
	return peggy->fixed_length;
}

void Peggy_setNoteOnHandler(Peggy_t peggy, void (*note_on_handler)(Peggy_t peggy, int note))
{
	peggy->note_on_handler = note_on_handler;
}

void Peggy_setNoteOffHandler(Peggy_t peggy, void (*note_off_handler)(Peggy_t peggy, int note))
{
	peggy->note_off_handler = note_off_handler;
}

void Peggy_noteOn(Peggy_t peggy, int note)
{
	int i;

	for (i = 0; i < peggy->number_of_input_notes; i++)
	{
		if (note == peggy->sorted_input_notes[i]) return;

		if (note < peggy->sorted_input_notes[i])
		{
			memmove(&(peggy->sorted_input_notes[i + 1]), &(peggy->sorted_input_notes[i]), sizeof (int));
			break;
		}
	}

	peggy->input_notes[peggy->number_of_input_notes] = note;
	peggy->sorted_input_notes[i] = note;
	(peggy->number_of_input_notes)++;
	if (!peggy->free_running && peggy->number_of_input_notes == 1) Peggy_resetTransport(peggy);
}

void Peggy_noteOff(Peggy_t peggy, int note)
{
	// TODO: hold

	int i;

	for (i = 0; i < peggy->number_of_input_notes; i++)
	{
		if (note == peggy->input_notes[i])
		{
			memmove(&(peggy->input_notes[i]), &(peggy->input_notes[i + 1]), sizeof (int));
			break;
		}
	}

	if (i == peggy->number_of_input_notes) return;

	for (i = 0; i < peggy->number_of_input_notes; i++)
	{
		if (note == peggy->sorted_input_notes[i])
		{
			memmove(&(peggy->sorted_input_notes[i]), &(peggy->sorted_input_notes[i + 1]), sizeof (int));
			break;
		}
	}

	(peggy->number_of_input_notes)--;
}

void Peggy_tick(Peggy_t peggy)
{
	long now = MidiUtil_getCurrentTimeMsecs();
	long msecs_per_step = (long)(60000 / (peggy->steps_per_quarter_note * peggy->tempo));
	long msecs_per_duty_cycle = (long)(msecs_per_step * peggy->duty_cycle);
	long msecs_since_last_step = now - peggy->last_step_time;
	if (msecs_since_last_step < msecs_per_duty_cycle) return;

	// TODO: note offs for last step

	if (msecs_since_last_step < msecs_per_step) return;
	peggy->last_step_time = now - (msecs_since_last_step % msecs_per_step); // backdate to avoid drift
	peggy->step_number = (peggy->step_number + 1) % (peggy->fixed_length > 0 ? peggy->fixed_length : peggy->number_of_input_steps > 0 ? peggy->number_of_input_steps : 1);

	if (peggy->number_of_input_notes == 0) return;
	
	switch (peggy->mode)
	{
		case PEGGY_MODE_UP:
		{
			int number_of_steps = peggy->number_of_input_notes * peggy->octaves;
			int note = peggy->input_notes[peggy->next_step_number % peggy->octaves];
			int octave = peggy->next_step_number / peggy->octaves;
			break;
		}
		case PEGGY_MODE_DOWN:
		{
			break;
		}
		case PEGGY_MODE_UP_DOWN_INC:
		{
			break;
		}
		case PEGGY_MODE_UP_DOWN_EXC:
		{
			break;
		}
		case PEGGY_MODE_DOWN_UP_INC:
		{
			break;
		}
		case PEGGY_MODE_DOWN_UP_EXC:
		{
			break;
		}
		case PEGGY_MODE_ORDER:
		{
			break;
		}
		case PEGGY_MODE_RANDOM:
		{
			break;
		}
		case PEGGY_MODE_CHORD:
		{
			break;
		}
	}
}

