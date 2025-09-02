
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include "hardware.h"
#include "midi.h"
#include "utils.h"

#define MAX_RETRIG_TIME 0.05
#define RETRIG_VELOCITY 64

typedef struct SyntinaDriver *SyntinaDriver_t;

typedef enum
{
	KEY_FUNCTION_TYPE_NONE = 0,
	KEY_FUNCTION_TYPE_NOTE,
	KEY_FUNCTION_TYPE_RETRIG,
	KEY_FUNCTION_TYPE_CC,
	KEY_FUNCTION_TYPE_PANIC,
	KEY_FUNCTION_TYPE_ALT,
	KEY_FUNCTION_TYPE_PROGRAM,
	KEY_FUNCTION_TYPE_TRANSPOSE,
	KEY_FUNCTION_TYPE_LEFT_TRANSPOSE,
	KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE,
	KEY_FUNCTION_TYPE_PRESET,
	KEY_FUNCTION_TYPE_TARE
}
KeyFunctionType_t;

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
	int tilt_x_cc;
	int tilt_y_cc;

	struct
	{
		KeyFunctionType_t type;

		union
		{
			int note;
			int cc;
			int alt;
			int program;
			int transpose;
			int left_transpose;
			int right_transpose;
			const char *preset;
		}
		u;
	}
	key_function[256][16];

	int key_down_note[256];
	int note_down_count[128];
	float note_off_time[128];
	int key_down_cc[256];
	int alt;
	int key_down_alt[256];
	int alt_down_count;
	int note_retrig_count[128];
	int key_down_retrig[256];
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
	syntina_driver->tilt_x_cc = -1;
	syntina_driver->tilt_y_cc = -1;

	for (int alt = 0; alt < 16; alt++)
	{
		for (int key = 0; key < 256; key++) syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_NONE;
	}

	for (int key = 0; key < 256; key++) syntina_driver->key_down_note[key] = -1;
	for (int note = 0; note < 128; note++) syntina_driver->note_down_count[note] = 0;
	for (int note = 0; note < 128; note++) syntina_driver->note_off_time[note] = 0;
	for (int key = 0; key < 256; key++) syntina_driver->key_down_cc[key] = -1;
	syntina_driver->alt = 0;
	for (int key = 0; key < 256; key++) syntina_driver->key_down_alt[key] = 0;
	syntina_driver->alt_down_count = 0;
	for (int note = 0; note < 128; note++) syntina_driver->note_retrig_count[note] = 0;
	for (int key = 0; key < 256; key++) syntina_driver->key_down_retrig[key] = 0;
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
	if (program_json) MidiOut_sendProgramChange(syntina_driver->midi_out, 0, json_integer_value(program_json));

	json_t *transpose_json = json_object_get(preset_json, "transpose");
	if (transpose_json) syntina_driver->transpose = json_integer_value(transpose_json);

	json_t *left_transpose_json = json_object_get(preset_json, "left-transpose");
	if (left_transpose_json) syntina_driver->left_transpose = json_integer_value(left_transpose_json);

	json_t *right_transpose_json = json_object_get(preset_json, "right-transpose");
	if (right_transpose_json) syntina_driver->right_transpose = json_integer_value(right_transpose_json);

	json_t *squeeze_cc_json = json_object_get(preset_json, "squeeze-cc");
	if (squeeze_cc_json) syntina_driver->squeeze_cc = json_integer_value(squeeze_cc_json);

	json_t *tilt_x_cc_json = json_object_get(preset_json, "tilt-x-cc");
	if (tilt_x_cc_json) syntina_driver->tilt_x_cc = json_integer_value(tilt_x_cc_json);

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
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_CC;
			syntina_driver->key_function[key][alt].u.cc = json_integer_value(json_object_get(to_json, "number"));
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
			syntina_driver->key_function[key][alt].u.program = json_integer_value(json_object_get(to_json, "number"));
		}
		else if (strcmp(key_function, "transpose") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_TRANSPOSE;
			syntina_driver->key_function[key][alt].u.transpose = json_integer_value(json_object_get(to_json, "amount"));
		}
		else if (strcmp(key_function, "left-transpose") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_LEFT_TRANSPOSE;
			syntina_driver->key_function[key][alt].u.left_transpose = json_integer_value(json_object_get(to_json, "amount"));
		}
		else if (strcmp(key_function, "right-transpose") == 0)
		{
			syntina_driver->key_function[key][alt].type = KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE;
			syntina_driver->key_function[key][alt].u.right_transpose = json_integer_value(json_object_get(to_json, "amount"));
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
	}
}

void SyntinaDriver_keyDown(SyntinaDriver_t syntina_driver, int key)
{
	switch (syntina_driver->key_function[key][syntina_driver->alt].type)
	{
		case KEY_FUNCTION_TYPE_NOTE:
		{
			int note = syntina_driver->key_function[key][syntina_driver->alt].u.note + syntina_driver->transpose + (key < 100 ? syntina_driver->left_transpose : syntina_driver->right_transpose);
			if (syntina_driver->note_down_count[note] > 0) MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
			MidiOut_sendNoteOn(syntina_driver->midi_out, 0, note, 127);
			syntina_driver->note_down_count[note]++;
			syntina_driver->key_down_note[key] = note;
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

			syntina_driver->key_down_retrig[key] = 1;
			break;
		}
		case KEY_FUNCTION_TYPE_CC:
		{
			int cc = syntina_driver->key_function[key][syntina_driver->alt].u.cc;
			MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, 127);
			syntina_driver->key_down_cc[key] = cc;
			break;
		}
		case KEY_FUNCTION_TYPE_ALT:
		{
			syntina_driver->alt = syntina_driver->key_function[key][syntina_driver->alt].u.alt;
			syntina_driver->alt_down_count++;
			syntina_driver->key_down_alt[key] = 1;
			break;
		}
		case KEY_FUNCTION_TYPE_PROGRAM:
		{
			MidiOut_sendProgramChange(syntina_driver->midi_out, 0, syntina_driver->key_function[key][syntina_driver->alt].u.program);
			break;
		}
		case KEY_FUNCTION_TYPE_TRANSPOSE:
		{
			syntina_driver->transpose = syntina_driver->key_function[key][syntina_driver->alt].u.transpose;
			break;
		}
		case KEY_FUNCTION_TYPE_LEFT_TRANSPOSE:
		{
			syntina_driver->left_transpose = syntina_driver->key_function[key][syntina_driver->alt].u.left_transpose;
			break;
		}
		case KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE:
		{
			syntina_driver->right_transpose = syntina_driver->key_function[key][syntina_driver->alt].u.right_transpose;
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
				syntina_driver->key_down_note[key] = -1;
				syntina_driver->key_down_cc[key] = -1;
				syntina_driver->key_down_alt[key] = 0;
				syntina_driver->key_down_retrig[key] = 0;
			}

			syntina_driver->alt = 0;
			syntina_driver->alt_down_count = 0;

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
		default:
		{
			break;
		}
	}
}

void SyntinaDriver_keyUp(SyntinaDriver_t syntina_driver, int key)
{
	int note = syntina_driver->key_down_note[key];

	if (note >= 0)
	{
		syntina_driver->note_down_count[note]--;

		if (syntina_driver->note_down_count[note] == 0)
		{
			MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
			syntina_driver->note_off_time[note] = Time_getTime();
		}

		syntina_driver->key_down_note[key] = -1;
		return;
	}

	if (syntina_driver->key_down_retrig[key])
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

		syntina_driver->key_down_retrig[key] = 0;
		return;
	}

	int cc = syntina_driver->key_down_cc[key];

	if (cc >= 0)
	{
		MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, 0);
		syntina_driver->key_down_cc[key] = -1;
		return;
	}

	if (syntina_driver->key_down_alt[key])
	{
		syntina_driver->alt_down_count--;
		if (syntina_driver->alt_down_count == 0) syntina_driver->alt = 0;
		syntina_driver->key_down_alt[key] = 0;
		return;
	}
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

		int value;

		if (SqueezeSensor_read(syntina_driver->squeeze_sensor, &value))
		{
			if (syntina_driver->squeeze_cc >= 0) MidiOut_sendControlChange(syntina_driver->midi_out, 0, syntina_driver->squeeze_cc, value);
		}

		int tilt_x, tilt_y;

		if (TiltSensor_read(syntina_driver->tilt_sensor, &tilt_x, &tilt_y))
		{
			if (syntina_driver->tilt_x_cc >= 0) MidiOut_sendControlChange(syntina_driver->midi_out, 0, syntina_driver->tilt_x_cc, tilt_x);
			if (syntina_driver->tilt_y_cc >= 0) MidiOut_sendControlChange(syntina_driver->midi_out, 0, syntina_driver->tilt_y_cc, tilt_y);
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

