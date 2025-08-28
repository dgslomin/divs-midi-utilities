
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <NAU7802.h>
#include "hardware.h"
#include "utils.h"

#define LEFT_I2C_BUS_NUMBER 1
#define RIGHT_I2C_BUS_NUMBER 3
#define KEYBOARD_I2C_DEVICE_NUMBER 0x34
#define SQUEEZE_SENSOR_I2C_DEVICE_NUMBER 0x2A
#define TILT_SENSOR_I2C_DEVICE_NUMBER 0x19
#define SWAP_ROWS_AND_COLUMNS
#define SMOOTHER_SAMPLES 8
#define SQUEEZE_SENSOR_CALIBRATION_FACTOR 800000

struct Keyboard
{
	int i2c_bus_number;
	int key_offset;
	int fd;
};

static void Keyboard_disconnect(Keyboard_t keyboard)
{
	if (keyboard->fd < 0) return;
	close(keyboard->fd);
	keyboard->fd = -1;
}

static void Keyboard_connect(Keyboard_t keyboard)
{
	char i2c_device_filename[128];
	sprintf(i2c_device_filename, "/dev/i2c-%d", keyboard->i2c_bus_number);
	keyboard->fd = open(i2c_device_filename, O_RDWR);

	if (keyboard->fd < 0)
	{
		fprintf(stderr, "Warning: cannot connect to keyboard on %s\n (bus not found)", i2c_device_filename);
		return;
	}

	ioctl(keyboard->fd, I2C_SLAVE, KEYBOARD_I2C_DEVICE_NUMBER);

	if (i2c_smbus_read_byte_data(keyboard->fd, 0x03) & 0x80 != 0)
	{
		fprintf(stderr, "Warning: cannot connect to keyboard on %s (device not found)\n", i2c_device_filename);
		Keyboard_disconnect(keyboard);
		return;
	}

#ifdef SWAP_ROWS_AND_COLUMNS
	i2c_smbus_write_byte_data(keyboard->fd, 0x1D, 0x7F); /* 7 rows */
	i2c_smbus_write_byte_data(keyboard->fd, 0x1E, 0x1F); /* 5 columns */
#else
	i2c_smbus_write_byte_data(keyboard->fd, 0x1D, 0x1F); /* 5 rows */
	i2c_smbus_write_byte_data(keyboard->fd, 0x1E, 0x7F); /* 7 columns */
#endif

	i2c_smbus_write_byte_data(keyboard->fd, 0x1F, 0x00); /* 0 additional columns */

	fprintf(stderr, "Info: connected keyboard on %s\n", i2c_device_filename);
}

static Keyboard_t Keyboard_open(int i2c_bus_number, int key_offset)
{
	Keyboard_t keyboard = (Keyboard_t)(malloc(sizeof (struct Keyboard)));
	keyboard->i2c_bus_number = i2c_bus_number;
	keyboard->key_offset = key_offset;
	Keyboard_connect(keyboard);
	return keyboard;
}

Keyboard_t Keyboard_openLeft(void)
{
	return Keyboard_open(LEFT_I2C_BUS_NUMBER, 0);
}

Keyboard_t Keyboard_openRight(void)
{
	return Keyboard_open(RIGHT_I2C_BUS_NUMBER, 100);
}

void Keyboard_close(Keyboard_t keyboard)
{
	Keyboard_disconnect(keyboard);
	free(keyboard);
}

void Keyboard_reconnect(Keyboard_t keyboard)
{
	if (keyboard->fd >= 0) close(keyboard->fd);
	Keyboard_connect(keyboard);
}

int Keyboard_read(Keyboard_t keyboard, int *key_p, int *down_p)
{
	if (keyboard->fd < 0) return 0;
	int data;

	// We should just be able to read the key value register (0x04) but it seems to work more reliably if we read the key count register (0x03) first.
	data = i2c_smbus_read_byte_data(keyboard->fd, 0x03);
	if (data < 0 || data & 0x05 == 0) return 0;

	data = i2c_smbus_read_byte_data(keyboard->fd, 0x04);
	if (data <= 0) return 0;

#ifdef SWAP_ROWS_AND_COLUMNS
	int row = 4 - (((data & 0x7F) % 10) - 1);
	int column = (data & 0x7F) / 10;
#else
	int row = 4 - ((data & 0x7F) / 10);
	int column = ((data & 0x7F) % 10) - 1;
#endif

	*key_p = (row * 10) + column + keyboard->key_offset;
	*down_p = ((data & 0x80) != 0);
	return 1;
}

struct SqueezeSensor
{
	Smoother_t smoother;
	NAU7802_t nau7802;
	int zero_offset_saved;
	int zero_offset;
	int last_reading;
};

static void SqueezeSensor_disconnect(SqueezeSensor_t squeeze_sensor)
{
	if (squeeze_sensor->nau7802 == NULL) return;
	free(squeeze_sensor->nau7802);
	squeeze_sensor->nau7802 = NULL;
}

static void SqueezeSensor_connect(SqueezeSensor_t squeeze_sensor)
{
	squeeze_sensor->nau7802 = NAU7802_new(LEFT_I2C_BUS_NUMBER, SQUEEZE_SENSOR_I2C_DEVICE_NUMBER);

	if (!NAU7802_begin(squeeze_sensor->nau7802, 1))
	{
		SqueezeSensor_disconnect(squeeze_sensor);
		return;
	}

   NAU7802_setSampleRate(squeeze_sensor->nau7802, NAU7802_SPS_320);
   NAU7802_calibrateAFE(squeeze_sensor->nau7802);

	if (squeeze_sensor->zero_offset_saved)
	{
		NAU7802_setZeroOffset(squeeze_sensor->nau7802, squeeze_sensor->zero_offset);
	}
	else
	{
		SqueezeSensor_tare(squeeze_sensor);
	}

	NAU7802_setCalibrationFactor(squeeze_sensor->nau7802, SQUEEZE_SENSOR_CALIBRATION_FACTOR);
}

SqueezeSensor_t SqueezeSensor_open(void)
{
	SqueezeSensor_t squeeze_sensor = (SqueezeSensor_t)(malloc(sizeof (struct SqueezeSensor)));
	squeeze_sensor->smoother = Smoother_new(SMOOTHER_SAMPLES);
	squeeze_sensor->zero_offset_saved = 0;
	SqueezeSensor_connect(squeeze_sensor);
	return squeeze_sensor;
}

void SqueezeSensor_close(SqueezeSensor_t squeeze_sensor)
{
	Smoother_free(squeeze_sensor->smoother);
	SqueezeSensor_disconnect(squeeze_sensor);
	free(squeeze_sensor);
}

void SqueezeSensor_reconnect(SqueezeSensor_t squeeze_sensor)
{
	SqueezeSensor_disconnect(squeeze_sensor);
	SqueezeSensor_connect(squeeze_sensor);
}

int SqueezeSensor_read(SqueezeSensor_t squeeze_sensor, int *value_p)
{
	if (squeeze_sensor->nau7802 == NULL) return 0;
	Smoother_addSample(squeeze_sensor->smoother, fmax(fmin(NAU7802_getWeight(squeeze_sensor->nau7802, 0, 1), 1.0), 0.0));
	*value_p = (int)(roundf(Smoother_getAverage(squeeze_sensor->smoother) * 127.0));
	if (*value_p == squeeze_sensor->last_reading) return 0;
	squeeze_sensor->last_reading = *value_p;
	return 1;
}

void SqueezeSensor_tare(SqueezeSensor_t squeeze_sensor)
{
	if (squeeze_sensor->nau7802 == NULL) return;
	NAU7802_calculateZeroOffset(squeeze_sensor->nau7802, SMOOTHER_SAMPLES);
	squeeze_sensor->zero_offset = NAU7802_getZeroOffset(squeeze_sensor->nau7802);
	squeeze_sensor->zero_offset_saved = 1;
}

struct TiltSensor
{
	Smoother_t smoother;
	int fd;
};

static void TiltSensor_disconnect(TiltSensor_t tilt_sensor)
{
	if (tilt_sensor->fd < 0) return;
	close(tilt_sensor->fd);
	tilt_sensor->fd = -1;
}

static void TiltSensor_connect(TiltSensor_t tilt_sensor)
{
	char i2c_device_filename[128];
	sprintf(i2c_device_filename, "/dev/i2c-%d", RIGHT_I2C_BUS_NUMBER);
	tilt_sensor->fd = open(i2c_device_filename, O_RDWR);

	if (tilt_sensor->fd < 0)
	{
		fprintf(stderr, "Warning: cannot connect to tilt sensor on %s (bus not found)\n", i2c_device_filename);
		return;
	}

	ioctl(tilt_sensor->fd, I2C_SLAVE, TILT_SENSOR_I2C_DEVICE_NUMBER);

	fprintf(stderr, "Info: connected tilt sensor on %s\n", i2c_device_filename);
}

TiltSensor_t TiltSensor_open(void)
{
	TiltSensor_t tilt_sensor = (TiltSensor_t)(malloc(sizeof (struct TiltSensor)));
	tilt_sensor->smoother = Smoother_new(SMOOTHER_SAMPLES);
	TiltSensor_connect(tilt_sensor);
	return tilt_sensor;
}

void TiltSensor_close(TiltSensor_t tilt_sensor)
{
	Smoother_free(tilt_sensor->smoother);
	TiltSensor_disconnect(tilt_sensor);
	free(tilt_sensor);
}

void TiltSensor_reconnect(TiltSensor_t tilt_sensor)
{
	TiltSensor_disconnect(tilt_sensor);
	TiltSensor_connect(tilt_sensor);
}

int TiltSensor_read(TiltSensor_t tilt_sensor, int *value_p)
{
	if (tilt_sensor->fd < 0) return 0;
	// TODO
	return 0;
}

void TiltSensor_tare(TiltSensor_t tilt_sensor)
{
	if (tilt_sensor->fd < 0) return;
	// TODO
}

