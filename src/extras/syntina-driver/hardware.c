
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
#define TILT_SENSOR_I2C_DEVICE_NUMBER 0x18
#define SWAP_ROWS_AND_COLUMNS
#define SQUEEZE_SENSOR_SMOOTHER_SAMPLES 8
#define SQUEEZE_SENSOR_CALIBRATION_FACTOR 800000
#define TILT_SENSOR_SMOOTHER_SAMPLES 32

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

	if (i2c_smbus_read_byte_data(keyboard->fd, 0x03) & 0x80 != 0) /* reserved bit of the status register is always zero on this device */
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
	int data = i2c_smbus_read_byte_data(keyboard->fd, 0x04);
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
	squeeze_sensor->smoother = Smoother_new(SQUEEZE_SENSOR_SMOOTHER_SAMPLES);
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
	NAU7802_calculateZeroOffset(squeeze_sensor->nau7802, SQUEEZE_SENSOR_SMOOTHER_SAMPLES);
	squeeze_sensor->zero_offset = NAU7802_getZeroOffset(squeeze_sensor->nau7802);
	squeeze_sensor->zero_offset_saved = 1;
}

struct TiltSensor
{
	Smoother_t raw_tilt_x_smoother;
	Smoother_t raw_tilt_y_smoother;
	float raw_tilt_x_zero;
	float raw_tilt_y_zero;
	int last_tilt_x;
	int last_tilt_y;
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

	if (i2c_smbus_read_byte_data(tilt_sensor->fd, 0x0F) != 0x33) /* "who am I" identification for this device */
	{
		TiltSensor_disconnect(tilt_sensor);
		fprintf(stderr, "Warning: cannot connect to tilt sensor on %s (device not found)\n", i2c_device_filename);
		return;
	}

	i2c_smbus_write_byte_data(tilt_sensor->fd, 0x20, 0x5F); /* 100 Hz, 8 bit, 3 axis */

	fprintf(stderr, "Info: connected tilt sensor on %s\n", i2c_device_filename);
}

TiltSensor_t TiltSensor_open(void)
{
	TiltSensor_t tilt_sensor = (TiltSensor_t)(malloc(sizeof (struct TiltSensor)));
	tilt_sensor->raw_tilt_x_smoother = Smoother_new(TILT_SENSOR_SMOOTHER_SAMPLES);
	tilt_sensor->raw_tilt_y_smoother = Smoother_new(TILT_SENSOR_SMOOTHER_SAMPLES);
	tilt_sensor->raw_tilt_x_zero = 0;
	tilt_sensor->raw_tilt_y_zero = 0;
	tilt_sensor->last_tilt_x = -1;
	tilt_sensor->last_tilt_y = -1;
	TiltSensor_connect(tilt_sensor);
	TiltSensor_tare(tilt_sensor);
	return tilt_sensor;
}

void TiltSensor_close(TiltSensor_t tilt_sensor)
{
	Smoother_free(tilt_sensor->raw_tilt_x_smoother);
	Smoother_free(tilt_sensor->raw_tilt_y_smoother);
	TiltSensor_disconnect(tilt_sensor);
	free(tilt_sensor);
}

void TiltSensor_reconnect(TiltSensor_t tilt_sensor)
{
	TiltSensor_disconnect(tilt_sensor);
	TiltSensor_connect(tilt_sensor);
}

static int TiltSensor_readRaw(TiltSensor_t tilt_sensor, float *raw_tilt_x_p, float *raw_tilt_y_p)
{
	if (tilt_sensor->fd < 0) return 0;
	int raw_x = i2c_smbus_read_byte_data(tilt_sensor->fd, 0x29);
	int raw_y = i2c_smbus_read_byte_data(tilt_sensor->fd, 0x2B);
	int raw_z = i2c_smbus_read_byte_data(tilt_sensor->fd, 0x2D);
	if (raw_x < 0 || raw_y < 0 || raw_z < 0) return 0;
	/* The syntina has the accelerometer mounted so that +raw_x is down, +raw_y is back, and +raw_z is left; convert to standard terms while fixing up the sign bit. */
	float x = (int8_t)(raw_y);
	float y = (int8_t)(raw_z);
	float z = -(int8_t)(raw_x);
	*raw_tilt_x_p = atan2(y, z); /* sometimes called "roll" */
	*raw_tilt_y_p = atan2(-x, sqrt((y * y) + (z * z))); /* sometimes called "pitch" */
	return 1;
}

int TiltSensor_read(TiltSensor_t tilt_sensor, int *tilt_x_p, int *tilt_y_p)
{
	if (tilt_sensor->fd < 0) return 0;
	float raw_tilt_x, raw_tilt_y;
	if (!TiltSensor_readRaw(tilt_sensor, &raw_tilt_x, &raw_tilt_y)) return 0;
	Smoother_addSample(tilt_sensor->raw_tilt_x_smoother, raw_tilt_x - tilt_sensor->raw_tilt_x_zero);
	Smoother_addSample(tilt_sensor->raw_tilt_y_smoother, raw_tilt_y - tilt_sensor->raw_tilt_y_zero);

	/* range -45 to -15 and 15 to 45 degrees */
	float fractional_tilt_x = fmin(fmax(Smoother_getAverage(tilt_sensor->raw_tilt_x_smoother) / (M_PI / 4.0), -1.0), 1.0);

	if (fractional_tilt_x > (15.0 / 45.0))
	{
		*tilt_x_p = (int)(roundf((fractional_tilt_x - (15.0 / 45.0)) * (45.0 / 30.0) * 63.5 + 63.5));
	}
	else if (fractional_tilt_x < (-15.0 / 45.0))
	{
		*tilt_x_p = (int)(roundf((fractional_tilt_x + (15.0 / 45.0)) * (45.0 / 30.0) * 63.5 + 63.5));
	}
	else
	{
		*tilt_x_p = 64;
	}

	/* range 0 to 45 degrees */
	*tilt_y_p = (int)(roundf(fmin(fmax(Smoother_getAverage(tilt_sensor->raw_tilt_y_smoother) / (M_PI / 4.0), 0.0), 1.0) * 127.0));

	if ((*tilt_x_p == tilt_sensor->last_tilt_x) && (*tilt_y_p == tilt_sensor->last_tilt_y)) return 0;
	tilt_sensor->last_tilt_x = *tilt_x_p;
	tilt_sensor->last_tilt_y = *tilt_y_p;
	return 1;
}

void TiltSensor_tare(TiltSensor_t tilt_sensor)
{
	if (tilt_sensor->fd < 0) return;
	float raw_tilt_x_total = 0, raw_tilt_y_total = 0;

	for (int sample_number = 0; sample_number < TILT_SENSOR_SMOOTHER_SAMPLES; sample_number++)
	{
		float raw_tilt_x, raw_tilt_y;
		if (!TiltSensor_readRaw(tilt_sensor, &raw_tilt_x, &raw_tilt_y)) return;
		raw_tilt_x_total += raw_tilt_x;
		raw_tilt_y_total += raw_tilt_y;
	}

	tilt_sensor->raw_tilt_x_zero = raw_tilt_x_total / TILT_SENSOR_SMOOTHER_SAMPLES;
	tilt_sensor->raw_tilt_y_zero = raw_tilt_y_total / TILT_SENSOR_SMOOTHER_SAMPLES;
}

