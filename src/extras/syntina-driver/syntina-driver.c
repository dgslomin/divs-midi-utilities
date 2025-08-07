
/*
 * References:
 * https://docs.kernel.org/i2c/dev-interface.html
 * https://github.com/torvalds/linux/blob/master/drivers/input/keyboard/tca8418_keypad.c
 * https://www.ti.com/document-viewer/TCA8418/datasheet
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <rtmidi_c.h>
#include <jansson.h>
#include <midiutil-common.h>
#include <midiutil-rtmidi.h>

#define LEFT_I2C_DEVICE_FILENAME "/dev/i2c-1"
#define RIGHT_I2C_DEVICE_FILENAME "/dev/i2c-3"
#define KEYPAD_MATRIX_ADDRESS 0x34
#define LOAD_CELL_ADDRESS 0x2A
#define ACCELEROMETER_ADDRESS 0x19
#define SWAP_ROWS_AND_COLUMNS

static void performance_timer_start(struct timespec *performance_timer)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, performance_timer);
}

static void performance_timer_elapsed(struct timespec *performance_timer, char *prefix)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now);
	uint64_t elapsed = (now.tv_sec - performance_timer->tv_sec) * 1000000 + (now.tv_nsec - performance_timer->tv_nsec) / 1000;
	fprintf(stderr, "%s%ld\n", prefix, elapsed);
}

static int open_keypad_matrix(char *i2c_device_filename)
{
	int fd = open(i2c_device_filename, O_RDWR);

	if (fd < 0)
	{
		fprintf(stderr, "Warning: cannot open keypad matrix on %s (bus not found)\n", i2c_device_filename);
		return -1;
	}

	ioctl(fd, I2C_SLAVE, KEYPAD_MATRIX_ADDRESS);

	if (i2c_smbus_read_byte_data(fd, 0x03) & 0x80 != 0)
	{
		close(fd);
		fprintf(stderr, "Warning: cannot open keypad matrix on %s (target not found)\n", i2c_device_filename);
		return -1;
	}

#ifdef SWAP_ROWS_AND_COLUMNS
	i2c_smbus_write_byte_data(fd, 0x1D, 0x7F); /* 7 rows */
	i2c_smbus_write_byte_data(fd, 0x1E, 0x1F); /* 5 columns */
#else
	i2c_smbus_write_byte_data(fd, 0x1D, 0x1F); /* 5 rows */
	i2c_smbus_write_byte_data(fd, 0x1E, 0x7F); /* 7 columns */
#endif

	i2c_smbus_write_byte_data(fd, 0x1F, 0x00); /* 0 additional columns */

	return fd;
}

static void close_keypad_matrix(int fd)
{
	if (fd < 0) return;
	close(fd);
}

static int read_keypad_matrix(int fd, int key_offset, int *key_p, int *down_p)
{
	if (fd < 0) return 0;
	uint8_t data = i2c_smbus_read_byte_data(fd, 0x04);
	if (!data) return 0;

#ifdef SWAP_ROWS_AND_COLUMNS
	int row = 4 - (((data & 0x7F) % 10) - 1);
	int column = (data & 0x7F) / 10;
#else
	int row = 4 - ((data & 0x7F) / 10);
	int column = ((data & 0x7F) % 10) - 1;
#endif

	*key_p = (row * 10) + column + key_offset;
	*down_p = ((data & 0x80) != 0);
	return 1;
}

static int open_load_cell(char *i2c_device_filename)
{
	int fd = open(i2c_device_filename, O_RDWR);

	if (fd < 0)
	{
		fprintf(stderr, "Warning: cannot open load cell on %s (bus not found)\n", i2c_device_filename);
		return -1;
	}

	ioctl(fd, I2C_SLAVE, LOAD_CELL_ADDRESS);
	return fd;
}

static void close_load_cell(int fd)
{
	if (fd < 0) return;
	close(fd);
}

static int read_load_cell(int fd)
{
	if (fd < 0) return -1;
	// TODO
	return -1;
}

static int open_accelerometer(char *i2c_device_filename)
{
	int fd = open(i2c_device_filename, O_RDWR);

	if (fd < 0)
	{
		fprintf(stderr, "Warning: cannot open accelerometer on %s (bus not found)\n", i2c_device_filename);
		return -1;
	}

	ioctl(fd, I2C_SLAVE, ACCELEROMETER_ADDRESS);
	return fd;
}

static void close_accelerometer(int fd)
{
	if (fd < 0) return;
	close(fd);
}

static int read_accelerometer(int fd)
{
	if (fd < 0) return -1;
	// TODO
	return -1;
}

static RtMidiOutPtr open_midi_out(char *port_name)
{
	while (1)
	{
		RtMidiOutPtr midi_out = rtmidi_open_out_port("syntina", port_name, "syntina");

		if (midi_out)
		{
			fprintf(stderr, "Info: connected to MIDI output %s\n", port_name);
			return midi_out;
		}

		sleep(0.5);
	}
}

static void close_midi_out(RtMidiOutPtr midi_out)
{
	rtmidi_close_port(midi_out);
}

static void send_note_on(RtMidiOutPtr midi_out, int channel, int note, int velocity)
{
	if (!midi_out) return;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
	MidiUtilMessage_setNoteOn(message, channel, note, velocity);
	rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
}

static void send_note_off(RtMidiOutPtr midi_out, int channel, int note)
{
	if (!midi_out) return;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
	MidiUtilMessage_setNoteOff(message, channel, note, 0);
	rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
}

static void send_cc(RtMidiOutPtr midi_out, int channel, int cc, int value)
{
	if (!midi_out) return;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE];
	MidiUtilMessage_setControlChange(message, channel, cc, value);
	rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE);
}

typedef struct
{
	RtMidiOutPtr midi_out;
	json_t *config;
	int left_keypad_matrix_fd;
	int right_keypad_matrix_fd;
	int load_cell_fd;
	int accelerometer_fd;

	int transpose;
	int right_transpose;
	int key_note[256];
	int key_cc[256];

	int key_down_note[256];
	int note_down_count[128];
	int key_down_cc[256];
}
syntina_driver_t;

static void load_config_preset(syntina_driver_t *syntina_driver, char *preset_name)
{
	json_t *preset_config = json_object_get(syntina_driver->config, preset_name);
	if (!preset_config) return;
	json_t *transpose = json_object_get(preset_config, "transpose");
	if (transpose) syntina_driver->transpose = json_integer_value(transpose);
	json_t *right_transpose = json_object_get(preset_config, "rightTranspose");
	if (right_transpose) syntina_driver->right_transpose = json_integer_value(right_transpose);

	for (int key = 0; key < 256; key++)
	{
		char key_string[4];
		sprintf(key_string, "%d", key);
		json_t *key_config = json_object_get(preset_config, key_string);
		const char *key_function = json_string_value(json_object_get(key_config, "function"));
		if (!key_function) continue;

		if (strcmp(key_function, "note") == 0)
		{
			syntina_driver->key_note[key] = json_integer_value(json_object_get(key_config, "note"));
		}
		else if (strcmp(key_function, "cc") == 0)
		{
			syntina_driver->key_cc[key] = json_integer_value(json_object_get(key_config, "cc"));
		}
	}
}

static void key_down(syntina_driver_t *syntina_driver, int key)
{
	int note = syntina_driver->key_note[key];

	if (note >= 0)
	{
		note += syntina_driver->transpose + (key >= 100 ? syntina_driver->right_transpose : 0);
		if (syntina_driver->note_down_count[note] > 0) send_note_off(syntina_driver->midi_out, 0, note);
		send_note_on(syntina_driver->midi_out, 0, note, 127);
		syntina_driver->note_down_count[note]++;
		syntina_driver->key_down_note[key] = note;
		return;
	}

	int cc = syntina_driver->key_cc[key];

	if (cc >= 0)
	{
		send_cc(syntina_driver->midi_out, 0, cc, 127);
		syntina_driver->key_down_cc[key] = cc;
		return;
	}
}

static void key_up(syntina_driver_t *syntina_driver, int key)
{
	int note = syntina_driver->key_down_note[key];

	if (note >= 0)
	{
		send_note_off(syntina_driver->midi_out, 0, note);
		syntina_driver->note_down_count[note]--;
		syntina_driver->key_down_note[key] = -1;
		return;
	}

	int cc = syntina_driver->key_down_cc[key];

	if (cc >= 0)
	{
		send_cc(syntina_driver->midi_out, 0, cc, 0);
		syntina_driver->key_down_cc[key] = -1;
		return;
	}
}

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --out <port> --config <syntina.conf>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	syntina_driver_t syntina_driver;
	syntina_driver.midi_out = NULL;
	syntina_driver.config = NULL;
	syntina_driver.left_keypad_matrix_fd = open_keypad_matrix(LEFT_I2C_DEVICE_FILENAME);
	syntina_driver.right_keypad_matrix_fd = open_keypad_matrix(RIGHT_I2C_DEVICE_FILENAME);
	syntina_driver.load_cell_fd = open_load_cell(LEFT_I2C_DEVICE_FILENAME);
	syntina_driver.accelerometer_fd = open_accelerometer(RIGHT_I2C_DEVICE_FILENAME);
	syntina_driver.transpose = 0;
	syntina_driver.right_transpose = 0;
	for (int key = 0; key < 256; key++) syntina_driver.key_note[key] = -1;
	for (int key = 0; key < 256; key++) syntina_driver.key_cc[key] = -1;
	for (int key = 0; key < 256; key++) syntina_driver.key_down_note[key] = -1;
	for (int note = 0; note < 128; note++) syntina_driver.note_down_count[note] = 0;
	for (int key = 0; key < 256; key++) syntina_driver.key_down_cc[key] = -1;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			syntina_driver.midi_out = open_midi_out(argv[i]);
		}
		else if (strcmp(argv[i], "--config") == 0)
		{
			if (++i == argc) usage(argv[0]);
			syntina_driver.config = json_load_file(argv[i], 0, NULL);

			if (!syntina_driver.config)
			{
				fprintf(stderr, "Error: cannot load config file %s\n", argv[i]);
				exit(1);
			}
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (!syntina_driver.midi_out || !syntina_driver.config) usage(argv[0]);
	load_config_preset(&syntina_driver, "default");

	while (1)
	{
		int key, down;

		while (read_keypad_matrix(syntina_driver.left_keypad_matrix_fd, 0, &key, &down))
		{
			key_up(&syntina_driver, key);
			if (down) key_down(&syntina_driver, key);
		}

		while (read_keypad_matrix(syntina_driver.right_keypad_matrix_fd, 100, &key, &down))
		{
			key_up(&syntina_driver, key);
			if (down) key_down(&syntina_driver, key);
		}

		read_load_cell(syntina_driver.load_cell_fd);
		read_accelerometer(syntina_driver.accelerometer_fd);
	}

	close_midi_out(syntina_driver.midi_out);
	close_keypad_matrix(syntina_driver.left_keypad_matrix_fd);
	close_keypad_matrix(syntina_driver.right_keypad_matrix_fd);
	close_load_cell(syntina_driver.load_cell_fd);
	close_accelerometer(syntina_driver.accelerometer_fd);
	json_decref(syntina_driver.config);
}

