#ifndef SYNTINA_DRIVER_HARDWARE_INCLUDED
#define SYNTINA_DRIVER_HARDWARE_INCLUDED

typedef struct Keyboard *Keyboard_t;
Keyboard_t Keyboard_openLeft(void);
Keyboard_t Keyboard_openRight(void);
void Keyboard_close(Keyboard_t keyboard);
void Keyboard_reconnect(Keyboard_t keyboard);
int Keyboard_read(Keyboard_t keyboard, int *key_p, int *down_p);

typedef struct SqueezeSensor *SqueezeSensor_t;
SqueezeSensor_t SqueezeSensor_open(void);
void SqueezeSensor_close(SqueezeSensor_t squeeze_sensor);
void SqueezeSensor_reconnect(SqueezeSensor_t squeeze_sensor);
int SqueezeSensor_read(SqueezeSensor_t squeeze_sensor, int *value_p);
void SqueezeSensor_tare(SqueezeSensor_t squeeze_sensor);

typedef struct TiltSensor *TiltSensor_t;
TiltSensor_t TiltSensor_open(void);
void TiltSensor_close(TiltSensor_t tilt_sensor);
void TiltSensor_reconnect(TiltSensor_t tilt_sensor);
int TiltSensor_read(TiltSensor_t tilt_sensor, int *value_p);
void TiltSensor_tare(TiltSensor_t tilt_sensor);

#endif
