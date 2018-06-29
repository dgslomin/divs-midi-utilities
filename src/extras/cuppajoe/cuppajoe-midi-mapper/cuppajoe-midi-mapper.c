
/*
 * This is the middle software layer of Cuppajoe, responsible for mapping from
 * raw physical controls (abstract buttons and joysticks) to musical concepts
 * (notes in their proper key and octave, "expression", etc), although both
 * sides are expressed in MIDI messages.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alsa/asoundlib.h>

#define MIDI_CLIENT_NAME "Cuppajoe MIDI Mapper"
#define MIDI_INPUT_PORT_NAME "in"
#define MIDI_OUTPUT_PORT_NAME "out"
#define MIDI_CHANNEL 0
#define FIRST_NOTE_BUTTON_NOTE_NUMBER 64 
#define NUMBER_OF_NOTE_BUTTONS 12
int note_button_to_scale_degree_map[] = {10, 11, 5, 4, 8, 9, 3, 2, 6, 7, 1, 0};
#define ACTION_BUTTON_0_NOTE_NUMBER 63
#define ACTION_BUTTON_1_NOTE_NUMBER 62
#define ACTION_BUTTON_2_NOTE_NUMBER 60
#define ACTION_BUTTON_3_NOTE_NUMBER 61
#define LEFT_JOYSTICK_X_CONTROLLER_NUMBER 16
#define LEFT_JOYSTICK_Y_CONTROLLER_NUMBER 17
#define RIGHT_JOYSTICK_X_CONTROLLER_NUMBER 18
#define RIGHT_JOYSTICK_Y_CONTROLLER_NUMBER 19
#define EXPRESSION_CONTROLLER_NUMBER 11

snd_seq_t *midi_client;
int midi_input_port;
int midi_output_port;
snd_seq_event_t *midi_input_event;
snd_seq_event_t midi_output_event;
int held_notes[NUMBER_OF_NOTE_BUTTONS];
int current_key = 0;
int current_lowest_scale_degree = 0;
int current_octave = 60;
int current_expression_value = 0;
int set_key = 0;
int set_lowest_scale_degree = 0;
int octave_joystick_x = 0;
int octave_joystick_y = 0;
int left_joystick_x = 64;
int left_joystick_y = 64;
int right_joystick_x = 64;
int right_joystick_y = 64;

void transpose_held_notes(void)
{
	for (int scale_degree = 0; scale_degree < NUMBER_OF_NOTE_BUTTONS; scale_degree++)
	{
		if (held_notes[scale_degree] >= 0)
		{
			int new_note_number = current_octave + current_key + scale_degree - (scale_degree < current_lowest_scale_degree ? 12 : 0);

			if (held_notes[scale_degree] != new_note_number)
			{
				snd_seq_ev_clear(&midi_output_event);
				snd_seq_ev_set_source(&midi_output_event, midi_output_port);
				snd_seq_ev_set_subs(&midi_output_event);
				snd_seq_ev_set_noteoff(&midi_output_event, MIDI_CHANNEL, held_notes[scale_degree], 0);
				snd_seq_event_output_direct(midi_client, &midi_output_event);

				held_notes[scale_degree] = new_note_number;

				snd_seq_ev_clear(&midi_output_event);
				snd_seq_ev_set_source(&midi_output_event, midi_output_port);
				snd_seq_ev_set_subs(&midi_output_event);
				snd_seq_ev_set_noteon(&midi_output_event, MIDI_CHANNEL, new_note_number, 127);
				snd_seq_event_output_direct(midi_client, &midi_output_event);
			}
		}
	}
}

void handle_note(int note_number, int pressed)
{
	if ((note_number >= FIRST_NOTE_BUTTON_NOTE_NUMBER) && (note_number < FIRST_NOTE_BUTTON_NOTE_NUMBER + NUMBER_OF_NOTE_BUTTONS))
	{
		int note_button_number = note_number - FIRST_NOTE_BUTTON_NOTE_NUMBER;
		int scale_degree = note_button_to_scale_degree_map[note_button_number];

		if (set_key)
		{
			if (pressed)
			{
				current_key = scale_degree;
				transpose_held_notes();
			}
		}
		else if (set_lowest_scale_degree)
		{
			if (pressed)
			{
				current_lowest_scale_degree = scale_degree;
				transpose_held_notes();
			}
		}
		else
		{
			int output_note_number = current_octave + current_key + scale_degree - (scale_degree < current_lowest_scale_degree ? 12 : 0);

			snd_seq_ev_clear(&midi_output_event);
			snd_seq_ev_set_source(&midi_output_event, midi_output_port);
			snd_seq_ev_set_subs(&midi_output_event);

			if (pressed)
			{
				held_notes[scale_degree] = output_note_number;
				snd_seq_ev_set_noteon(&midi_output_event, MIDI_CHANNEL, output_note_number, 127);
			}
			else
			{
				held_notes[scale_degree] = -1;
				snd_seq_ev_set_noteoff(&midi_output_event, MIDI_CHANNEL, output_note_number, 0);
			}

			snd_seq_event_output_direct(midi_client, &midi_output_event);
		}
	}
	else
	{
		switch (note_number)
		{
			case ACTION_BUTTON_0_NOTE_NUMBER:
			{
				set_key = pressed;
				break;
			}
			case ACTION_BUTTON_1_NOTE_NUMBER:
			{
				set_lowest_scale_degree = pressed;
				break;
			}
			case ACTION_BUTTON_2_NOTE_NUMBER:
			{
				/* TODO */
				break;
			}
			case ACTION_BUTTON_3_NOTE_NUMBER:
			{
				/* TODO */
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

void handle_expression_knob(double knob_value)
{
	int expression_value = (int)(127 * knob_value);

	if (current_expression_value != expression_value)
	{
		current_expression_value = expression_value;
		snd_seq_ev_clear(&midi_output_event);
		snd_seq_ev_set_source(&midi_output_event, midi_output_port);
		snd_seq_ev_set_subs(&midi_output_event);
		snd_seq_ev_set_controller(&midi_output_event, MIDI_CHANNEL, EXPRESSION_CONTROLLER_NUMBER, expression_value);
		snd_seq_event_output_direct(midi_client, &midi_output_event);
	}
}

void handle_joystick_as_knob(int x_controller_value, int y_controller_value, void (*knob_handler)(double knob_value))
{
	double x = (float)(x_controller_value) / 64.0 - 1.0;
	double y = (float)(y_controller_value) / 64.0 - 1.0;
	double amplitude = sqrt((x * x) + (y * y));
	double angle = atan2(y, x);

	/* amplitude threshold ensures that you mean to trigger it at all */

	if (amplitude > 0.5)
	{
		/* one eighth of a turn dead zone means you don't accidentally wrap from max to min */

		if ((angle > M_PI / 8.0) && (angle < M_PI * 15.0 / 8.0))
		{
			double knob_value = (angle - (M_PI / 8.0)) / (M_PI * 7.0 / 4.0);
			(*knob_handler)(knob_value);
		}
	}
}

/* y is tap to jump, x is hold to temporarily shift */
void handle_octave_joystick(int x_controller_value, int y_controller_value)
{
	if (y_controller_value < 32)
	{
		if (octave_joystick_y != -1)
		{
			octave_joystick_y = -1;
			current_octave--;
			transpose_held_notes();
		}
	}
	else if (y_controller_value > 96)
	{
		if (octave_joystick_y != 1)
		{
			octave_joystick_y = 1;
			current_octave++;
			transpose_held_notes();
		}
	}
	else if ((y_controller_value > 48) && (y_controller_value < 80))
	{
		octave_joystick_y = 0;
	}

	if (x_controller_value < 32)
	{
		if (octave_joystick_x != -1)
		{
			octave_joystick_x = -1;
			current_octave--;
			transpose_held_notes();
		}
	}
	else if (x_controller_value > 96)
	{
		if (octave_joystick_x != 1)
		{
			octave_joystick_x = 1;
			current_octave++;
			transpose_held_notes();
		}
	}
	else if ((x_controller_value > 48) && (x_controller_value < 80))
	{
		if (octave_joystick_x != 0)
		{
			current_octave -= octave_joystick_x;
			octave_joystick_x = 0;
			transpose_held_notes();
		}
	}
}

void handle_controller(int controller_number, int value)
{
	switch (controller_number)
	{
		case LEFT_JOYSTICK_X_CONTROLLER_NUMBER:
		{
			left_joystick_x = value;
			handle_joystick_as_knob(left_joystick_x, left_joystick_y, &handle_expression_knob);
			break;
		}
		case LEFT_JOYSTICK_Y_CONTROLLER_NUMBER:
		{
			left_joystick_y = value;
			handle_joystick_as_knob(left_joystick_x, left_joystick_y, &handle_expression_knob);
			break;
		}
		case RIGHT_JOYSTICK_X_CONTROLLER_NUMBER:
		{
			right_joystick_x = value;
			handle_octave_joystick(right_joystick_x, right_joystick_y);
			break;
		}
		case RIGHT_JOYSTICK_Y_CONTROLLER_NUMBER:
		{
			right_joystick_y = value;
			handle_octave_joystick(right_joystick_x, right_joystick_y);
			break;
		}
		default:
		{
			break;
		}
	}
}

int main(int argc, char **argv)
{
	for (int scale_degree = 0; scale_degree < NUMBER_OF_NOTE_BUTTONS; scale_degree++)
	{
		held_notes[scale_degree] = -1;
	}

	snd_seq_open(&midi_client, "default", SND_SEQ_OPEN_DUPLEX, 0);
	snd_seq_set_client_name(midi_client, MIDI_CLIENT_NAME);
	midi_input_port = snd_seq_create_simple_port(midi_client, MIDI_INPUT_PORT_NAME, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
	midi_output_port = snd_seq_create_simple_port(midi_client, MIDI_OUTPUT_PORT_NAME, SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

	while (snd_seq_event_input(midi_client, &midi_input_event) >= 0)
	{
		switch (midi_input_event->type)
		{
			case SND_SEQ_EVENT_NOTEON:
			{
				if (midi_input_event->data.note.velocity > 0)
				{
					handle_note(midi_input_event->data.note.note, 1);
				}
				else
				{
					handle_note(midi_input_event->data.note.note, 0);
				}

				break;
			}
			case SND_SEQ_EVENT_NOTEOFF:
			{
				handle_note(midi_input_event->data.note.note, 0);
				break;
			}
			case SND_SEQ_EVENT_CONTROLLER:
			{
				handle_controller(midi_input_event->data.control.param, midi_input_event->data.control.value);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	snd_seq_delete_simple_port(midi_client, midi_output_port);
	snd_seq_delete_simple_port(midi_client, midi_input_port);
	snd_seq_close(midi_client);
	return 0;
}

