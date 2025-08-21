
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include "hardware.h"
#include "midi.h"

typedef struct SyntinaDriver *SyntinaDriver_t;

typedef enum
{
	KEY_FUNCTION_TYPE_NONE = 0,
	KEY_FUNCTION_TYPE_NOTE,
	KEY_FUNCTION_TYPE_CC,
	KEY_FUNCTION_TYPE_PANIC,
	KEY_FUNCTION_TYPE_TRANSPOSE,
	KEY_FUNCTION_TYPE_LEFT_TRANSPOSE,
	KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE,
	KEY_FUNCTION_TYPE_PRESET
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
	int tilt_cc;

	struct
	{
		KeyFunctionType_t type;

		union
		{
			int note;
			int cc;

			struct
			{
				int amount;
				int relative;
			}
			transpose;

			struct
			{
				int amount;
				int relative;
			}
			left_transpose;

			struct
			{
				int amount;
				int relative;
			}
			right_transpose;

			const char *preset;
		}
		u;
	}
	key_function[256];

	int key_down_note[256];
	int note_down_count[128];
	int key_down_cc[256];
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
	syntina_driver->squeeze_cc = 0;
	syntina_driver->tilt_cc = 0;
	for (int key = 0; key < 256; key++) syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_NONE;
	for (int key = 0; key < 256; key++) syntina_driver->key_down_note[key] = -1;
	for (int note = 0; note < 128; note++) syntina_driver->note_down_count[note] = 0;
	for (int key = 0; key < 256; key++) syntina_driver->key_down_cc[key] = -1;
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
	json_t *preset_config = json_object_get(syntina_driver->config, preset_name);
	if (!preset_config) return;

	json_t *include = json_object_get(preset_config, "include");

	for (int i = 0; i < json_array_size(include); i++)
	{
		const char *include_preset_name = json_string_value(json_array_get(include, 0));
		SyntinaDriver_loadPreset(syntina_driver, include_preset_name);
	}

	json_t *transpose = json_object_get(preset_config, "transpose");
	if (transpose) syntina_driver->transpose = json_integer_value(transpose);

	json_t *left_transpose = json_object_get(preset_config, "left-transpose");
	if (left_transpose) syntina_driver->left_transpose = json_integer_value(left_transpose);

	json_t *right_transpose = json_object_get(preset_config, "right-transpose");
	if (right_transpose) syntina_driver->right_transpose = json_integer_value(right_transpose);

	json_t *squeeze_cc = json_object_get(preset_config, "squeeze-cc");
	if (squeeze_cc) syntina_driver->squeeze_cc = json_integer_value(squeeze_cc);

	json_t *tilt_cc = json_object_get(preset_config, "tilt-cc");
	if (tilt_cc) syntina_driver->tilt_cc = json_integer_value(tilt_cc);

	for (int key = 0; key < 256; key++)
	{
		char key_string[4];
		sprintf(key_string, "key-%d", key);
		json_t *key_config = json_object_get(preset_config, key_string);
		if (!key_config) continue;

		syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_NONE;

		const char *key_function = json_string_value(json_object_get(key_config, "function"));
		if (!key_function) continue;

		if (strcmp(key_function, "note") == 0)
		{
			syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_NOTE;
			syntina_driver->key_function[key].u.note = json_integer_value(json_object_get(key_config, "note"));
		}
		else if (strcmp(key_function, "cc") == 0)
		{
			syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_CC;
			syntina_driver->key_function[key].u.cc = json_integer_value(json_object_get(key_config, "cc"));
		}
		else if (strcmp(key_function, "panic") == 0)
		{
			syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_PANIC;
		}
		else if (strcmp(key_function, "transpose") == 0)
		{
			syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_TRANSPOSE;
			syntina_driver->key_function[key].u.transpose.amount = json_integer_value(json_object_get(key_config, "amount"));
			syntina_driver->key_function[key].u.transpose.relative = json_boolean_value(json_object_get(key_config, "relative"));
		}
		else if (strcmp(key_function, "left-transpose") == 0)
		{
			syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_LEFT_TRANSPOSE;
			syntina_driver->key_function[key].u.left_transpose.amount = json_integer_value(json_object_get(key_config, "amount"));
			syntina_driver->key_function[key].u.left_transpose.relative = json_boolean_value(json_object_get(key_config, "relative"));
		}
		else if (strcmp(key_function, "right-transpose") == 0)
		{
			syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE;
			syntina_driver->key_function[key].u.right_transpose.amount = json_integer_value(json_object_get(key_config, "amount"));
			syntina_driver->key_function[key].u.right_transpose.relative = json_boolean_value(json_object_get(key_config, "relative"));
		}
		else if (strcmp(key_function, "preset") == 0)
		{
			syntina_driver->key_function[key].type = KEY_FUNCTION_TYPE_PRESET;
			syntina_driver->key_function[key].u.preset = json_string_value(json_object_get(key_config, "preset"));
		}
	}
}

void SyntinaDriver_keyDown(SyntinaDriver_t syntina_driver, int key)
{
	switch (syntina_driver->key_function[key].type)
	{
		case KEY_FUNCTION_TYPE_NOTE:
		{
			int note = syntina_driver->key_function[key].u.note + syntina_driver->transpose + (key < 100 ? syntina_driver->left_transpose : syntina_driver->right_transpose);
			if (syntina_driver->note_down_count[note] > 0) MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
			MidiOut_sendNoteOn(syntina_driver->midi_out, 0, note, 127);
			syntina_driver->note_down_count[note]++;
			syntina_driver->key_down_note[key] = note;
			break;
		}
		case KEY_FUNCTION_TYPE_CC:
		{
			int cc = syntina_driver->key_function[key].u.cc;
			MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, 127);
			syntina_driver->key_down_cc[key] = cc;
			break;
		}
		case KEY_FUNCTION_TYPE_TRANSPOSE:
		{
			syntina_driver->transpose = (syntina_driver->key_function[key].u.transpose.relative ? syntina_driver->transpose : 0) + syntina_driver->key_function[key].u.transpose.amount;
			break;
		}
		case KEY_FUNCTION_TYPE_LEFT_TRANSPOSE:
		{
			syntina_driver->left_transpose = (syntina_driver->key_function[key].u.left_transpose.relative ? syntina_driver->left_transpose : 0) + syntina_driver->key_function[key].u.left_transpose.amount;
			break;
		}
		case KEY_FUNCTION_TYPE_RIGHT_TRANSPOSE:
		{
			syntina_driver->right_transpose = (syntina_driver->key_function[key].u.right_transpose.relative ? syntina_driver->right_transpose : 0) + syntina_driver->key_function[key].u.right_transpose.amount;
			break;
		}
		case KEY_FUNCTION_TYPE_PANIC:
		{
			for (int note = 0; note < 128; note++) MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
			break;
		}
		case KEY_FUNCTION_TYPE_PRESET:
		{
			SyntinaDriver_loadPreset(syntina_driver, syntina_driver->key_function[key].u.preset);
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
		if (syntina_driver->note_down_count[note] == 0) MidiOut_sendNoteOff(syntina_driver->midi_out, 0, note, 0);
		syntina_driver->key_down_note[key] = -1;
		return;
	}

	int cc = syntina_driver->key_down_cc[key];

	if (cc >= 0)
	{
		MidiOut_sendControlChange(syntina_driver->midi_out, 0, cc, 0);
		syntina_driver->key_down_cc[key] = -1;
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
			MidiOut_sendControlChange(syntina_driver->midi_out, 0, syntina_driver->squeeze_cc, value);
		}

		if (TiltSensor_read(syntina_driver->tilt_sensor, &value))
		{
			MidiOut_sendControlChange(syntina_driver->midi_out, 0, syntina_driver->tilt_cc, value);
		}
	}
}

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --out <port> --config <syntina.conf>\n", program_name);
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

	SyntinaDriver_t syntina_driver = SyntinaDriver_new(midi_out_port_name, config_filename);
	SyntinaDriver_run(syntina_driver);
	SyntinaDriver_free(syntina_driver);
}

