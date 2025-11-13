
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jansson.h>
#include "hardware.h"
#include "midi.h"
#include "utils.h"

#define MAX_RETRIG_TIME 0.05
#define RETRIG_VELOCITY 64
#define FINE_CHANGE_TIME 0.5
#define COARSE_CHANGE_TIME 0.025

typedef struct SyntinaDriver *SyntinaDriver_t;
typedef struct KeyFunction *KeyFunction_t;

typedef enum
{
	KEY_FUNCTION_TYPE_NONE = 0,
	KEY_FUNCTION_TYPE_NOTE,
	KEY_FUNCTION_TYPE_RETRIG,
	KEY_FUNCTION_TYPE_CC_MOMENTARY,
	KEY_FUNCTION_TYPE_CC_INCREASE,
	KEY_FUNCTION_TYPE_CC_DECREASE,
	KEY_FUNCTION_TYPE_FINE,
	KEY_FUNCTION_TYPE_PANIC,
	KEY_FUNCTION_TYPE_ALT,
	KEY_FUNCTION_TYPE_PROGRAM,
	KEY_FUNCTION_TYPE_TRANSPOSE,
	KEY_FUNCTION_TYPE_LEFT_TRANSPOSE,
	KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE,
	KEY_FUNCTION_TYPE_PRESET,
	KEY_FUNCTION_TYPE_TARE,
	KEY_FUNCTION_TYPE_TILT_ENABLE,
	KEY_FUNCTION_TYPE_SYSTEM_COMMAND,
}
KeyFunctionType_t;

struct KeyFunction
{
	KeyFunctionType_t type;

	union
	{
		int note;
		int cc_momentary;
		int cc_increase;
		int cc_decrease;
		int alt;

		struct
		{
			int relative;
			int value;
		}
		program;

		struct
		{
			int relative;
			int value;
		}
		transpose;

		struct
		{
			int relative;
			int value;
		}
		left_transpose;

		struct
		{
			int relative;
			int value;
		}
		right_transpose;

		const char *preset;

		struct
		{
			int x;
			int y;
		}
		tilt_enable;

		const char *system_command;
	}
	u;
};

struct SyntinaDriver
{
	MidiOut_t midi_out;
	json_t *config;
	Keyboard_t left_keyboard;
	Keyboard_t right_keyboard;
	SqueezeSensor_t squeeze_sensor;
	TiltSensor_t tilt_sensor;
	int transpose;
	int left_transpose;
	int right_transpose;
	int squeeze_cc;
	int tilt_x_enable;
	int tilt_y_enable;
	int tilt_y_cc;
	struct KeyFunction key_function[256][16];
	KeyFunction_t key_down_function[256];
	int key_down_transpose[256];
	int note_down_count[128];
	float note_off_time[128];
	int alt;
	int alt_down_count;
	int note_retrig_count[128];
	int program;
	int last_squeeze_amount;
	int last_tilt_x_amount;
	int last_tilt_y_amount;
	int cc_value[128];
	Set_t cc_increase_set;
	float cc_increase_time[128];
	Set_t cc_decrease_set;
	float cc_decrease_time[128];
	int fine_down_count;
};

SyntinaDriver_t SyntinaDriver_new(char *midi_out_port_name, char *config_filename)
{
	SyntinaDriver_t syntina_driver = (SyntinaDriver_t)(malloc(sizeof (struct SyntinaDriver)));
	syntina_driver->midi_out = MidiOut_open(midi_out_port_name);;
	syntina_driver->config = json_load_file(config_filename, 0, NULL);
	if (syntina_driver->config == NULL) fprintf(stderr, "Warning: cannot load config file \"%s\"\n", config_filename);
	syntina_driver->left_keyboard = Keyboard_openLeft();
	syntina_driver->right_keyboard = Keyboard_openRight();
	syntina_driver->squeeze_sensor = SqueezeSensor_open();
	syntina_driver->tilt_sensor = TiltSensor_open();
	syntina_driver->transpose = 0;
	syntina_driver->left_transpose = 0;
	syntina_driver->right_transpose = 0;
	syntina_driver->squeeze_cc = -1;
	syntina_driver->tilt_x_enable = 0;
	syntina_driver->tilt_y_enable = 0;
	syntina_driver->tilt_y_cc = -1;

	for (int alt = 0; alt < 16; alt++)
	{
		for (int key = 0; key < 256; key++) syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_NONE;
	}

	for (int key = 0; key < 256; key++) syntina_driver->key_down_function[key] = NULL;
	for (int key = 0; key < 256; key++) syntina_driver->key_down_transpose[key] = 0;
	for (int note = 0; note < 128; note++) syntina_driver->note_down_count[note] = 0;
	for (int note = 0; note < 128; note++) syntina_driver->note_off_time[note] = 0;
	syntina_driver->alt = 0;
	syntina_driver->alt_down_count = 0;
	for (int note = 0; note < 128; note++) syntina_driver->note_retrig_count[note] = 0;
	syntina_driver->program = 0;
	for (int cc = 0; cc < 128; cc++) syntina_driver->cc_value[cc] = 0;
	syntina_driver->cc_increase_set = Set_new(128);
	for (int cc = 0; cc < 128; cc++) syntina_driver->cc_increase_time[cc] = 0;
	syntina_driver->cc_decrease_set = Set_new(128);
	for (int cc = 0; cc < 128; cc++) syntina_driver->cc_decrease_time[cc] = 0;
	syntina_driver->fine_down_count = 0;
	return syntina_driver;
}

void SyntinaDriver_free(SyntinaDriver_t syntina_driver)
{
	MidiOut_close(syntina_driver->midi_out);
	json_decref(syntina_driver->config);
	Keyboard_close(syntina_driver->left_keyboard);
	Keyboard_close(syntina_driver->right_keyboard);
	SqueezeSensor_close(syntina_driver->squeeze_sensor);
	TiltSensor_close(syntina_driver->tilt_sensor);
	Set_free(syntina_driver->cc_increase_set);
	Set_free(syntina_driver->cc_decrease_set);
	free(syntina_driver);
}

void SyntinaDriver_loadPreset(SyntinaDriver_t syntina_driver, const char *preset_name)
{
	json_t *preset_json = json_object_get(syntina_driver->config, preset_name);
	if (!preset_json) return;

	json_t *include_json = json_object_get(preset_json, "include");

	for (int include_number = 0; include_number < json_array_size(include_json); include_number++)
	{
		const char *include_preset_name = json_string_value(json_array_get(include_json, include_number));
		SyntinaDriver_loadPreset(syntina_driver, include_preset_name);
	}

	json_t *program_json = json_object_get(preset_json, "program");

	if (program_json)
	{
		syntina_driver->program = json_integer_value(program_json);
		MidiOut_sendProgramChange(syntina_driver->midi_out, 0, syntina_driver->program);
	}

	json_t *transpose_json = json_object_get(preset_json, "transpose");
	if (transpose_json) syntina_driver->transpose = json_integer_value(transpose_json);

	json_t *left_transpose_json = json_object_get(preset_json, "left-transpose");
	if (left_transpose_json) syntina_driver->left_transpose = json_integer_value(left_transpose_json);

	json_t *right_transpose_json = json_object_get(preset_json, "right-transpose");
	if (right_transpose_json) syntina_driver->right_transpose = json_integer_value(right_transpose_json);

	json_t *squeeze_cc_json = json_object_get(preset_json, "squeeze-cc");
	if (squeeze_cc_json) syntina_driver->squeeze_cc = json_integer_value(squeeze_cc_json);

	json_t *tilt_x_enable_json = json_object_get(preset_json, "tilt-x-enable");
	if (tilt_x_enable_json) syntina_driver->tilt_x_enable = json_boolean_value(tilt_x_enable_json);

	json_t *tilt_x_dead_zone_json = json_object_get(preset_json, "tilt-x-dead-zone");
	if (tilt_x_dead_zone_json) TiltSensor_setDeadZoneSize(syntina_driver->tilt_sensor, json_real_value(tilt_x_dead_zone_json));

	json_t *tilt_y_enable_json = json_object_get(preset_json, "tilt-y-enable");
	if (tilt_y_enable_json) syntina_driver->tilt_y_enable = json_boolean_value(tilt_y_enable_json);

	json_t *tilt_y_cc_json = json_object_get(preset_json, "tilt-y-cc");
	if (tilt_y_cc_json) syntina_driver->tilt_y_cc = json_integer_value(tilt_y_cc_json);

	json_t *mappings_json = json_object_get(preset_json, "mappings");

	for (int mapping_number = 0; mapping_number < json_array_size(mappings_json); mapping_number++)
	{
		json_t *mapping_json = json_array_get(mappings_json, mapping_number);
		json_t *from_json = json_object_get(mapping_json, "from");
		json_t *to_json = json_object_get(mapping_json, "to");

		json_t *key_json = json_object_get(from_json, "key");
		if (!key_json) continue;
		int key = json_integer_value(key_json);

		int alt = json_integer_value(json_object_get(from_json, "alt"));

		syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_NONE;

		const char *key_function = json_string_value(json_object_get(to_json, "function"));
		if (!key_function) continue;

		if (strcmp(key_function, "note") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_NOTE;
			syntina_driver->key_function[key][alt].u.note = json_integer_value(json_object_get(to_json, "note"));
		}
		else if (strcmp(key_function, "retrig") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_RETRIG;
		}
		else if (strcmp(key_function, "cc") == 0)
		{
			const char *behavior = json_string_value(json_object_get(to_json, "behavior"));
			if (!behavior) behavior = "momentary";

			if (strcmp(behavior, "momentary") == 0)
			{
				syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_CC_MOMENTARY;
				syntina_driver->key_function[key][alt].u.cc_momentary = json_integer_value(json_object_get(to_json, "number"));
			}
			else if (strcmp(behavior, "increase") == 0)
			{
				syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_CC_INCREASE;
				syntina_driver->key_function[key][alt].u.cc_increase = json_integer_value(json_object_get(to_json, "number"));
			}
			else if (strcmp(behavior, "decrease") == 0)
			{
				syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_CC_DECREASE;
				syntina_driver->key_function[key][alt].u.cc_decrease = json_integer_value(json_object_get(to_json, "number"));
			}
		}
		else if (strcmp(key_function, "fine") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_FINE;
		}
		else if (strcmp(key_function, "panic") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_PANIC;
		}
		else if (strcmp(key_function, "alt") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_ALT;
			syntina_driver->key_function[key][alt].u.alt = json_integer_value(json_object_get(to_json, "number"));
		}
		else if (strcmp(key_function, "program") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_PROGRAM;
			syntina_driver->key_function[key][alt].u.program.relative = json_boolean_value(json_object_get(to_json, "relative"));
			syntina_driver->key_function[key][alt].u.program.value = json_integer_value(json_object_get(to_json, "value"));
		}
		else if (strcmp(key_function, "transpose") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_TRANSPOSE;
			syntina_driver->key_function[key][alt].u.transpose.relative = json_boolean_value(json_object_get(to_json, "relative"));
			syntina_driver->key_function[key][alt].u.transpose.value = json_integer_value(json_object_get(to_json, "value"));
		}
		else if (strcmp(key_function, "left-transpose") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_LEFT_TRANSPOSE;
			syntina_driver->key_function[key][alt].u.left_transpose.relative = json_boolean_value(json_object_get(to_json, "relative"));
			syntina_driver->key_function[key][alt].u.left_transpose.value = json_integer_value(json_object_get(to_json, "value"));
		}
		else if (strcmp(key_function, "right-transpose") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE;
			syntina_driver->key_function[key][alt].u.right_transpose.relative = json_boolean_value(json_object_get(to_json, "relative"));
			syntina_driver->key_function[key][alt].u.right_transpose.value = json_integer_value(json_object_get(to_json, "value"));
		}
		else if (strcmp(key_function, "preset") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_PRESET;
			syntina_driver->key_function[key][alt].u.preset = json_string_value(json_object_get(to_json, "name"));
		}
		else if (strcmp(key_function, "tare") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_TARE;
		}
		else if (strcmp(key_function, "tilt-enable") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_TILT_ENABLE;
			syntina_driver->key_function[key][alt].u.tilt_enable.x = YesNoToggle_parse(json_string_value(json_object_get(to_json, "x")));
			syntina_driver->key_function[key][alt].u.tilt_enable.y = YesNoToggle_parse(json_string_value(json_object_get(to_json, "y")));
		}
		else if (strcmp(key_function, "system") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_SYSTEM_COMMAND;
			syntina_driver->key_function[key][alt].u.system_command = json_string_value(json_object_get(to_json, "command"));
		}
	}
}

void SyntinaDriver_keyDown(SyntinaDriver_t syntina_driver, int key)
{
	switch (syntina_driver->key_function[key][syntina_driver->alt].type)
	{
		case KEY_FUNCTION_TYPE_NOTE:
		{
			syntina_driver->key_down_transpose[key] = syntina_driver->transpose + (key < 100 ? syntina_driver->left_transpose : syntina_driver->right_transpose);
			int note = syntina_driver->key_function[key][syntina_driver->alt].u.note + syntina_driver->key_down_transpose[key];
			if (syntina_driver->note_down_count[note] > 0) MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
			MidiOut_sendNoteOn(syntina_driver->midi_out, 0, note, 127);
			syntina_driver->note_down_count[note]++;
			break;
		}
		case KEY_FUNCTION_TYPE_RETRIG:
		{
			float current_time = Time_getTime();

			for (int note = 0; note < 128; note++)
			{
				int should_retrig = 0;

				if (syntina_driver->note_down_count[note] > 0)
				{
					MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
					should_retrig = 1;
				}
				else if (current_time - syntina_driver->note_off_time[note] < MAX_RETRIG_TIME)
				{
					should_retrig = 1;
				}

				if (should_retrig)
				{
					MidiOut_sendNoteOn(syntina_driver->midi_out, 0, note, RETRIG_VELOCITY);
					syntina_driver->note_down_count[note]++;
					syntina_driver->note_retrig_count[note]++;
				}
			}

			break;
		}
		case KEY_FUNCTION_TYPE_CC_MOMENTARY:
		{
			int cc = syntina_driver->key_function[key][syntina_driver->alt].u.cc_momentary;
			syntina_driver->cc_value[cc] = 127;
			MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, 127);
			break;
		}
		case KEY_FUNCTION_TYPE_CC_INCREASE:
		{
			int cc = syntina_driver->key_function[key][syntina_driver->alt].u.cc_increase;
			syntina_driver->cc_value[cc] = clamp_int(syntina_driver->cc_value[cc] + 1, 0, 127);
			Set_add(syntina_driver->cc_increase_set, cc);
			MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, syntina_driver->cc_value[cc]);
			break;
		}
		case KEY_FUNCTION_TYPE_CC_DECREASE:
		{
			int cc = syntina_driver->key_function[key][syntina_driver->alt].u.cc_decrease;
			syntina_driver->cc_value[cc] = clamp_int(syntina_driver->cc_value[cc] - 1, 0, 127);
			Set_add(syntina_driver->cc_decrease_set, cc);
			MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, syntina_driver->cc_value[cc]);
			break;
		}
		case KEY_FUNCTION_TYPE_ALT:
		{
			syntina_driver->alt = syntina_driver->key_function[key][syntina_driver->alt].u.alt;
			syntina_driver->alt_down_count++;
			break;
		}
		case KEY_FUNCTION_TYPE_PROGRAM:
		{
			syntina_driver->program = clamp_int((syntina_driver->key_function[key][syntina_driver->alt].u.program.relative ? syntina_driver->program : 0) + syntina_driver->key_function[key][syntina_driver->alt].u.program.value, 0, 127);
			MidiOut_sendProgramChange(syntina_driver->midi_out, 0, syntina_driver->program);
			break;
		}
		case KEY_FUNCTION_TYPE_TRANSPOSE:
		{
			syntina_driver->transpose = (syntina_driver->key_function[key][syntina_driver->alt].u.transpose.relative ? syntina_driver->transpose : 0) + syntina_driver->key_function[key][syntina_driver->alt].u.transpose.value;
			break;
		}
		case KEY_FUNCTION_TYPE_LEFT_TRANSPOSE:
		{
			syntina_driver->left_transpose = (syntina_driver->key_function[key][syntina_driver->alt].u.left_transpose.relative ? syntina_driver->left_transpose : 0) + syntina_driver->key_function[key][syntina_driver->alt].u.left_transpose.value;
			break;
		}
		case KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE:
		{
			syntina_driver->right_transpose = (syntina_driver->key_function[key][syntina_driver->alt].u.right_transpose.relative ? syntina_driver->right_transpose : 0) + syntina_driver->key_function[key][syntina_driver->alt].u.right_transpose.value;
			break;
		}
		case KEY_FUNCTION_TYPE_FINE:
		{
			syntina_driver->fine_down_count++;
			break;
		}
		case KEY_FUNCTION_TYPE_PANIC:
		{
			for (int note = 0; note < 128; note++)
			{
				MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
				syntina_driver->note_down_count[note] = 0;
				syntina_driver->note_off_time[note] = 0;
				syntina_driver->note_retrig_count[note] = 0;
			}

			for (int key = 0; key < 256; key++)
			{
				syntina_driver->key_down_function[key] = NULL;
				syntina_driver->key_down_transpose[key] = 0;
			}

			syntina_driver->alt = 0;
			syntina_driver->alt_down_count = 0;
			Set_clear(syntina_driver->cc_increase_set);
			Set_clear(syntina_driver->cc_decrease_set);

			Keyboard_reconnect(syntina_driver->left_keyboard);
			Keyboard_reconnect(syntina_driver->right_keyboard);
			SqueezeSensor_reconnect(syntina_driver->squeeze_sensor);
			TiltSensor_reconnect(syntina_driver->tilt_sensor);

			break;
		}
		case KEY_FUNCTION_TYPE_PRESET:
		{
			SyntinaDriver_loadPreset(syntina_driver, syntina_driver->key_function[key][syntina_driver->alt].u.preset);
			break;
		}
		case KEY_FUNCTION_TYPE_TARE:
		{
			SqueezeSensor_tare(syntina_driver->squeeze_sensor);
			TiltSensor_tare(syntina_driver->tilt_sensor);
			break;
		}
		case KEY_FUNCTION_TYPE_TILT_ENABLE:
		{
			syntina_driver->tilt_x_enable = YesNoToggle_apply(syntina_driver->tilt_x_enable, syntina_driver->key_function[key][syntina_driver->alt].u.tilt_enable.x);
			syntina_driver->tilt_y_enable = YesNoToggle_apply(syntina_driver->tilt_y_enable, syntina_driver->key_function[key][syntina_driver->alt].u.tilt_enable.y);
			break;
		}
		case KEY_FUNCTION_TYPE_SYSTEM_COMMAND:
		{
			system(syntina_driver->key_function[key][syntina_driver->alt].u.system_command);
			break;
		}
		default:
		{
			break;
		}
	}

	syntina_driver->key_down_function[key] = &(syntina_driver->key_function[key][syntina_driver->alt]);
}

void SyntinaDriver_keyUp(SyntinaDriver_t syntina_driver, int key)
{
	KeyFunction_t key_function = syntina_driver->key_down_function[key];
	if (!key_function) return;

	switch (key_function->type)
	{
		case KEY_FUNCTION_TYPE_NOTE:
		{
			int note = key_function->u.note + syntina_driver->key_down_transpose[key];
			syntina_driver->note_down_count[note]--;

			if (syntina_driver->note_down_count[note] == 0)
			{
				MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
				syntina_driver->note_off_time[note] = Time_getTime();
			}

			break;
		}
		case KEY_FUNCTION_TYPE_RETRIG:
		{
			for (int note = 0; note < 128; note++)
			{
				if (syntina_driver->note_retrig_count[note] > 0)
				{
					syntina_driver->note_retrig_count[note]--;
					syntina_driver->note_down_count[note]--;
	
					if (syntina_driver->note_down_count[note] == 0)
					{
						MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
					}
				}
			}

			break;
		}
		case KEY_FUNCTION_TYPE_CC_MOMENTARY:
		{
			int cc = key_function->u.cc_momentary;
			MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, 0);
			syntina_driver->cc_value[cc] = 0;
			break;
		}
		case KEY_FUNCTION_TYPE_CC_INCREASE:
		{
			int cc = key_function->u.cc_increase;
			Set_remove(syntina_driver->cc_increase_set, cc);
			break;
		}
		case KEY_FUNCTION_TYPE_CC_DECREASE:
		{
			int cc = key_function->u.cc_decrease;
			Set_remove(syntina_driver->cc_decrease_set, cc);
			break;
		}
		case KEY_FUNCTION_TYPE_ALT:
		{
			syntina_driver->alt_down_count--;
			if (syntina_driver->alt_down_count == 0) syntina_driver->alt = 0;
			break;
		}
		case KEY_FUNCTION_TYPE_FINE:
		{
			syntina_driver->fine_down_count--;
			break;
		}
		default:
		{
			break;
		}
	}

	syntina_driver->key_down_function[key] = NULL;
}

void SyntinaDriver_run(SyntinaDriver_t syntina_driver)
{
	SyntinaDriver_loadPreset(syntina_driver, "default");

	while (1)
	{
		int key, down;

		while (Keyboard_read(syntina_driver->left_keyboard, &key, &down))
		{
			SyntinaDriver_keyUp(syntina_driver, key);
			if (down) SyntinaDriver_keyDown(syntina_driver, key);
		}

		while (Keyboard_read(syntina_driver->right_keyboard, &key, &down))
		{
			SyntinaDriver_keyUp(syntina_driver, key);
			if (down) SyntinaDriver_keyDown(syntina_driver, key);
		}

		if (syntina_driver->squeeze_cc >= 0)
		{
			float raw_squeeze_amount;

			if (SqueezeSensor_read(syntina_driver->squeeze_sensor, &raw_squeeze_amount))
			{
				int squeeze_amount = roundf(raw_squeeze_amount * 127.0);

				if (squeeze_amount != syntina_driver->last_squeeze_amount)
				{
					MidiOut_sendControlChange(syntina_driver->midi_out, 0, syntina_driver->squeeze_cc, squeeze_amount);
					syntina_driver->last_squeeze_amount = squeeze_amount;
				}
			}
		}

		if (syntina_driver->tilt_x_enable || syntina_driver->tilt_y_enable)
		{
			float raw_tilt_x_amount, raw_tilt_y_amount;

			if (TiltSensor_read(syntina_driver->tilt_sensor, &raw_tilt_x_amount, &raw_tilt_y_amount))
			{
				if (syntina_driver->tilt_x_enable)
				{
					int tilt_x_amount = (int)(roundf(raw_tilt_x_amount * 8191.5 + 8191.5));

					if (tilt_x_amount != syntina_driver->last_tilt_x_amount)
					{
						MidiOut_sendPitchBend(syntina_driver->midi_out, 0, tilt_x_amount);
						syntina_driver->last_tilt_x_amount = tilt_x_amount;
					}
				}

				if (syntina_driver->tilt_y_enable && syntina_driver->tilt_y_cc >= 0)
				{
					int tilt_y_amount = (int)(roundf(raw_tilt_y_amount * 127.0));

					if (tilt_y_amount != syntina_driver->last_tilt_y_amount)
					{
						MidiOut_sendControlChange(syntina_driver->midi_out, 0, syntina_driver->tilt_y_cc, tilt_y_amount);
						syntina_driver->last_tilt_y_amount = tilt_y_amount;
					}
				}
			}
		}

		for (int i = 0; i < Set_count(syntina_driver->cc_increase_set); i++)
		{
			int cc = Set_nth(syntina_driver->cc_increase_set, i);
			float now = Time_getTime();

			if (now - syntina_driver->cc_increase_time[cc] > (syntina_driver->fine_down_count == 0 ? COARSE_CHANGE_TIME : FINE_CHANGE_TIME))
			{
				syntina_driver->cc_increase_time[cc] = now;
				syntina_driver->cc_value[cc] = clamp_int(syntina_driver->cc_value[cc] + 1, 0, 127);
				MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, syntina_driver->cc_value[cc]);
			}
		}

		for (int i = 0; i < Set_count(syntina_driver->cc_decrease_set); i++)
		{
			int cc = Set_nth(syntina_driver->cc_decrease_set, i);
			float now = Time_getTime();

			if (now - syntina_driver->cc_increase_time[cc] > (syntina_driver->fine_down_count == 0 ? COARSE_CHANGE_TIME : FINE_CHANGE_TIME))
			{
				syntina_driver->cc_increase_time[cc] = now;
				syntina_driver->cc_value[cc] = clamp_int(syntina_driver->cc_value[cc] - 1, 0, 127);
				MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, syntina_driver->cc_value[cc]);
			}
		}
	}
}

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --out <port> --config <syntina-driver.conf>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char *midi_out_port_name = NULL;
	char *config_filename = NULL;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_out_port_name = argv[i];
		}
		else if (strcmp(argv[i], "--config") == 0)
		{
			if (++i == argc) usage(argv[0]);
			config_filename = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_out_port_name == NULL) || (config_filename == NULL)) usage(argv[0]);
	SyntinaDriver_t syntina_driver = SyntinaDriver_new(midi_out_port_name, config_filename);
	SyntinaDriver_run(syntina_driver);
	SyntinaDriver_free(syntina_driver);
}

