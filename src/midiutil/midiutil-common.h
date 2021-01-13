#ifndef MIDIUTIL_COMMON_INCLUDED
#define MIDIUTIL_COMMON_INCLUDED

/* Common helpers that have no dependencies beyond the standard C library. */

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MidiUtilByteArray *MidiUtilByteArray_t;
typedef struct MidiUtilIntArray *MidiUtilIntArray_t;
typedef struct MidiUtilLongArray *MidiUtilLongArray_t;
typedef struct MidiUtilFloatArray *MidiUtilFloatArray_t;
typedef struct MidiUtilDoubleArray *MidiUtilDoubleArray_t;
typedef struct MidiUtilPointerArray *MidiUtilPointerArray_t;
typedef struct MidiUtilString *MidiUtilString_t;
typedef struct MidiUtilBlobArray *MidiUtilBlobArray_t;
typedef struct MidiUtilStringArray *MidiUtilStringArray_t;
typedef struct MidiUtilIntIntMap *MidiUtilIntIntMap_t;
typedef struct MidiUtilIntPointerMap *MidiUtilIntPointerMap_t;
typedef struct MidiUtilLongIntMap *MidiUtilLongIntMap_t;
typedef struct MidiUtilLongPointerMap *MidiUtilLongPointerMap_t;
typedef struct MidiUtilPointerIntMap *MidiUtilPointerIntMap_t;
typedef struct MidiUtilPointerPointerMap *MidiUtilPointerPointerMap_t;
typedef struct MidiUtilBlobIntMap *MidiUtilBlobIntMap_t;
typedef struct MidiUtilBlobPointerMap *MidiUtilBlobPointerMap_t;
typedef struct MidiUtilStringIntMap *MidiUtilStringIntMap_t;
typedef struct MidiUtilStringPointerMap *MidiUtilStringPointerMap_t;

typedef enum
{
	MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF = 0x80,
	MIDI_UTIL_MESSAGE_TYPE_NOTE_ON = 0x90,
	MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE = 0xA0,
	MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE = 0xB0,
	MIDI_UTIL_MESSAGE_TYPE_PROGRAM_CHANGE = 0xC0,
	MIDI_UTIL_MESSAGE_TYPE_CHANNEL_PRESSURE = 0xD0,
	MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL = 0xE0,
	MIDI_UTIL_MESSAGE_TYPE_SYSEX = 0xF0
}
MidiUtilMessageType_t;

typedef enum
{
	MIDI_UTIL_MESSAGE_SIZE_SHORT_MESSAGE = 4,
	MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF = 3,
	MIDI_UTIL_MESSAGE_SIZE_NOTE_ON = 3,
	MIDI_UTIL_MESSAGE_SIZE_KEY_PRESSURE = 3,
	MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE = 2,
	MIDI_UTIL_MESSAGE_SIZE_PROGRAM_CHANGE = 2,
	MIDI_UTIL_MESSAGE_SIZE_CHANNEL_PRESSURE = 2,
	MIDI_UTIL_MESSAGE_SIZE_PITCH_WHEEL = 3
}
MidiUtilMessageSize_t;

MidiUtilByteArray_t MidiUtilByteArray_new(int initial_capacity);
void MidiUtilByteArray_free(MidiUtilByteArray_t array);
void MidiUtilByteArray_clear(MidiUtilByteArray_t array);
int MidiUtilByteArray_getCapacity(MidiUtilByteArray_t array);
void MidiUtilByteArray_setCapacity(MidiUtilByteArray_t array, int capacity);
int MidiUtilByteArray_getSize(MidiUtilByteArray_t array);
void MidiUtilByteArray_setSize(MidiUtilByteArray_t array, int size);
unsigned char *MidiUtilByteArray_getBuffer(MidiUtilByteArray_t array);
unsigned char MidiUtilByteArray_get(MidiUtilByteArray_t array, int element_number);
void MidiUtilByteArray_set(MidiUtilByteArray_t array, int element_number, unsigned char value);
void MidiUtilByteArray_add(MidiUtilByteArray_t array, unsigned char value);
void MidiUtilByteArray_insert(MidiUtilByteArray_t array, int element_number, unsigned char value);
void MidiUtilByteArray_remove(MidiUtilByteArray_t array, int element_number);
void MidiUtilByteArray_setValues(MidiUtilByteArray_t array, unsigned char *values, int number_of_values);
void MidiUtilByteArray_addValues(MidiUtilByteArray_t array, unsigned char *values, int number_of_values);
void MidiUtilByteArray_replaceValues(MidiUtilByteArray_t array, int element_number, unsigned char *values, int number_of_values);
void MidiUtilByteArray_insertValues(MidiUtilByteArray_t array, int element_number, unsigned char *values, int number_of_values);
void MidiUtilByteArray_removeValues(MidiUtilByteArray_t array, int element_number, int number_of_values);

MidiUtilIntArray_t MidiUtilIntArray_new(int initial_capacity);
void MidiUtilIntArray_free(MidiUtilIntArray_t array);
void MidiUtilIntArray_clear(MidiUtilIntArray_t array);
int MidiUtilIntArray_getCapacity(MidiUtilIntArray_t array);
void MidiUtilIntArray_setCapacity(MidiUtilIntArray_t array, int capacity);
int MidiUtilIntArray_getSize(MidiUtilIntArray_t array);
void MidiUtilIntArray_setSize(MidiUtilIntArray_t array, int size);
int *MidiUtilIntArray_getBuffer(MidiUtilIntArray_t array);
int MidiUtilIntArray_get(MidiUtilIntArray_t array, int element_number);
void MidiUtilIntArray_set(MidiUtilIntArray_t array, int element_number, int value);
void MidiUtilIntArray_add(MidiUtilIntArray_t array, int value);
void MidiUtilIntArray_insert(MidiUtilIntArray_t array, int element_number, int value);
void MidiUtilIntArray_remove(MidiUtilIntArray_t array, int element_number);
void MidiUtilIntArray_setValues(MidiUtilIntArray_t array, int *values, int number_of_values);
void MidiUtilIntArray_addValues(MidiUtilIntArray_t array, int *values, int number_of_values);
void MidiUtilIntArray_replaceValues(MidiUtilIntArray_t array, int element_number, int *values, int number_of_values);
void MidiUtilIntArray_insertValues(MidiUtilIntArray_t array, int element_number, int *values, int number_of_values);
void MidiUtilIntArray_removeValues(MidiUtilIntArray_t array, int element_number, int number_of_values);

MidiUtilLongArray_t MidiUtilLongArray_new(int initial_capacity);
void MidiUtilLongArray_free(MidiUtilLongArray_t array);
void MidiUtilLongArray_clear(MidiUtilLongArray_t array);
int MidiUtilLongArray_getCapacity(MidiUtilLongArray_t array);
void MidiUtilLongArray_setCapacity(MidiUtilLongArray_t array, int capacity);
int MidiUtilLongArray_getSize(MidiUtilLongArray_t array);
void MidiUtilLongArray_setSize(MidiUtilLongArray_t array, int size);
long *MidiUtilLongArray_getBuffer(MidiUtilLongArray_t array);
long MidiUtilLongArray_get(MidiUtilLongArray_t array, int element_number);
void MidiUtilLongArray_set(MidiUtilLongArray_t array, int element_number, long value);
void MidiUtilLongArray_add(MidiUtilLongArray_t array, long value);
void MidiUtilLongArray_insert(MidiUtilLongArray_t array, int element_number, long value);
void MidiUtilLongArray_remove(MidiUtilLongArray_t array, int element_number);
void MidiUtilLongArray_setValues(MidiUtilLongArray_t array, long *values, int number_of_values);
void MidiUtilLongArray_addValues(MidiUtilLongArray_t array, long *values, int number_of_values);
void MidiUtilLongArray_replaceValues(MidiUtilLongArray_t array, int element_number, long *values, int number_of_values);
void MidiUtilLongArray_insertValues(MidiUtilLongArray_t array, int element_number, long *values, int number_of_values);
void MidiUtilLongArray_removeValues(MidiUtilLongArray_t array, int element_number, int number_of_values);

MidiUtilFloatArray_t MidiUtilFloatArray_new(int initial_capacity);
void MidiUtilFloatArray_free(MidiUtilFloatArray_t array);
void MidiUtilFloatArray_clear(MidiUtilFloatArray_t array);
int MidiUtilFloatArray_getCapacity(MidiUtilFloatArray_t array);
void MidiUtilFloatArray_setCapacity(MidiUtilFloatArray_t array, int capacity);
int MidiUtilFloatArray_getSize(MidiUtilFloatArray_t array);
void MidiUtilFloatArray_setSize(MidiUtilFloatArray_t array, int size);
float *MidiUtilFloatArray_getBuffer(MidiUtilFloatArray_t array);
float MidiUtilFloatArray_get(MidiUtilFloatArray_t array, int element_number);
void MidiUtilFloatArray_set(MidiUtilFloatArray_t array, int element_number, float value);
void MidiUtilFloatArray_add(MidiUtilFloatArray_t array, float value);
void MidiUtilFloatArray_insert(MidiUtilFloatArray_t array, int element_number, float value);
void MidiUtilFloatArray_remove(MidiUtilFloatArray_t array, int element_number);
void MidiUtilFloatArray_setValues(MidiUtilFloatArray_t array, float *values, int number_of_values);
void MidiUtilFloatArray_addValues(MidiUtilFloatArray_t array, float *values, int number_of_values);
void MidiUtilFloatArray_replaceValues(MidiUtilFloatArray_t array, int element_number, float *values, int number_of_values);
void MidiUtilFloatArray_insertValues(MidiUtilFloatArray_t array, int element_number, float *values, int number_of_values);
void MidiUtilFloatArray_removeValues(MidiUtilFloatArray_t array, int element_number, int number_of_values);

MidiUtilDoubleArray_t MidiUtilDoubleArray_new(int initial_capacity);
void MidiUtilDoubleArray_free(MidiUtilDoubleArray_t array);
void MidiUtilDoubleArray_clear(MidiUtilDoubleArray_t array);
int MidiUtilDoubleArray_getCapacity(MidiUtilDoubleArray_t array);
void MidiUtilDoubleArray_setCapacity(MidiUtilDoubleArray_t array, int capacity);
int MidiUtilDoubleArray_getSize(MidiUtilDoubleArray_t array);
void MidiUtilDoubleArray_setSize(MidiUtilDoubleArray_t array, int size);
double *MidiUtilDoubleArray_getBuffer(MidiUtilDoubleArray_t array);
double MidiUtilDoubleArray_get(MidiUtilDoubleArray_t array, int element_number);
void MidiUtilDoubleArray_set(MidiUtilDoubleArray_t array, int element_number, double value);
void MidiUtilDoubleArray_add(MidiUtilDoubleArray_t array, double value);
void MidiUtilDoubleArray_insert(MidiUtilDoubleArray_t array, int element_number, double value);
void MidiUtilDoubleArray_remove(MidiUtilDoubleArray_t array, int element_number);
void MidiUtilDoubleArray_setValues(MidiUtilDoubleArray_t array, double *values, int number_of_values);
void MidiUtilDoubleArray_addValues(MidiUtilDoubleArray_t array, double *values, int number_of_values);
void MidiUtilDoubleArray_replaceValues(MidiUtilDoubleArray_t array, int element_number, double *values, int number_of_values);
void MidiUtilDoubleArray_insertValues(MidiUtilDoubleArray_t array, int element_number, double *values, int number_of_values);
void MidiUtilDoubleArray_removeValues(MidiUtilDoubleArray_t array, int element_number, int number_of_values);

MidiUtilPointerArray_t MidiUtilPointerArray_new(int initial_capacity);
void MidiUtilPointerArray_free(MidiUtilPointerArray_t array);
void MidiUtilPointerArray_clear(MidiUtilPointerArray_t array);
int MidiUtilPointerArray_getCapacity(MidiUtilPointerArray_t array);
void MidiUtilPointerArray_setCapacity(MidiUtilPointerArray_t array, int capacity);
int MidiUtilPointerArray_getSize(MidiUtilPointerArray_t array);
void MidiUtilPointerArray_setSize(MidiUtilPointerArray_t array, int size);
void **MidiUtilPointerArray_getBuffer(MidiUtilPointerArray_t array);
void *MidiUtilPointerArray_get(MidiUtilPointerArray_t array, int element_number);
void MidiUtilPointerArray_set(MidiUtilPointerArray_t array, int element_number, void *value);
void MidiUtilPointerArray_add(MidiUtilPointerArray_t array, void *value);
void MidiUtilPointerArray_insert(MidiUtilPointerArray_t array, int element_number, void *value);
void MidiUtilPointerArray_remove(MidiUtilPointerArray_t array, int element_number);
void MidiUtilPointerArray_setValues(MidiUtilPointerArray_t array, void **values, int number_of_values);
void MidiUtilPointerArray_addValues(MidiUtilPointerArray_t array, void **values, int number_of_values);
void MidiUtilPointerArray_replaceValues(MidiUtilPointerArray_t array, int element_number, void **values, int number_of_values);
void MidiUtilPointerArray_insertValues(MidiUtilPointerArray_t array, int element_number, void **values, int number_of_values);
void MidiUtilPointerArray_removeValues(MidiUtilPointerArray_t array, int element_number, int number_of_values);

MidiUtilString_t MidiUtilString_new(int initial_capacity);
void MidiUtilString_free(MidiUtilString_t string);
void MidiUtilString_clear(MidiUtilString_t string);
int MidiUtilString_getCapacity(MidiUtilString_t string);
void MidiUtilString_setCapacity(MidiUtilString_t string, int capacity);
int MidiUtilString_getSize(MidiUtilString_t string);
unsigned char *MidiUtilString_get(MidiUtilString_t string);
void MidiUtilString_set(MidiUtilString_t string, unsigned char *value);
void MidiUtilString_addChar(MidiUtilString_t string, unsigned char value);
void MidiUtilString_addString(MidiUtilString_t string, unsigned char *value);

MidiUtilBlobArray_t MidiUtilBlobArray_new(int initial_element_capacity, int initial_byte_capacity);
void MidiUtilBlobArray_free(MidiUtilBlobArray_t array);
void MidiUtilBlobArray_clear(MidiUtilBlobArray_t array);
int MidiUtilBlobArray_getSize(MidiUtilBlobArray_t array);
unsigned char *MidiUtilBlobArray_getValue(MidiUtilBlobArray_t array, int element_number);
int MidiUtilBlobArray_getValueSize(MidiUtilBlobArray_t array, int element_number);
void MidiUtilBlobArray_set(MidiUtilBlobArray_t array, int element_number, unsigned char *value, int value_size);
void MidiUtilBlobArray_add(MidiUtilBlobArray_t array, unsigned char *value, int value_size);
void MidiUtilBlobArray_insert(MidiUtilBlobArray_t array, int element_number, unsigned char *value, int value_size);
void MidiUtilBlobArray_remove(MidiUtilBlobArray_t array, int element_number);

MidiUtilStringArray_t MidiUtilStringArray_new(int initial_element_capacity, int initial_byte_capacity);
void MidiUtilStringArray_free(MidiUtilStringArray_t array);
void MidiUtilStringArray_clear(MidiUtilStringArray_t array);
int MidiUtilStringArray_getSize(MidiUtilStringArray_t array);
unsigned char *MidiUtilStringArray_get(MidiUtilStringArray_t array, int element_number);
void MidiUtilStringArray_set(MidiUtilStringArray_t array, int element_number, unsigned char *value);
void MidiUtilStringArray_add(MidiUtilStringArray_t array, unsigned char *value);
void MidiUtilStringArray_insert(MidiUtilStringArray_t array, int element_number, unsigned char *value);
void MidiUtilStringArray_remove(MidiUtilStringArray_t array, int element_number);

MidiUtilIntIntMap_t MidiUtilIntIntMap_new(int number_of_buckets);
void MidiUtilIntIntMap_free(MidiUtilIntIntMap_t map);
void MidiUtilIntIntMap_clear(MidiUtilIntIntMap_t map);
int MidiUtilIntIntMap_hasKey(MidiUtilIntIntMap_t map, int key);
int MidiUtilIntIntMap_get(MidiUtilIntIntMap_t map, int key, int default_value);
void MidiUtilIntIntMap_set(MidiUtilIntIntMap_t map, int key, int value);
void MidiUtilIntIntMap_remove(MidiUtilIntIntMap_t map, int key);
void MidiUtilIntIntMap_enumerate(MidiUtilIntIntMap_t map, int (*callback)(int key, int value, void *user_data), void *user_data);

MidiUtilIntPointerMap_t MidiUtilIntPointerMap_new(int number_of_buckets);
void MidiUtilIntPointerMap_setFreeCallback(MidiUtilIntPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void MidiUtilIntPointerMap_free(MidiUtilIntPointerMap_t map);
void MidiUtilIntPointerMap_clear(MidiUtilIntPointerMap_t map);
int MidiUtilIntPointerMap_hasKey(MidiUtilIntPointerMap_t map, int key);
void *MidiUtilIntPointerMap_get(MidiUtilIntPointerMap_t map, int key);
void MidiUtilIntPointerMap_set(MidiUtilIntPointerMap_t map, int key, void *value);
void MidiUtilIntPointerMap_remove(MidiUtilIntPointerMap_t map, int key);
void MidiUtilIntPointerMap_enumerate(MidiUtilIntPointerMap_t map, int (*callback)(int key, void *value, void *user_data), void *user_data);

MidiUtilLongIntMap_t MidiUtilLongIntMap_new(int number_of_buckets);
void MidiUtilLongIntMap_free(MidiUtilLongIntMap_t map);
void MidiUtilLongIntMap_clear(MidiUtilLongIntMap_t map);
int MidiUtilLongIntMap_hasKey(MidiUtilLongIntMap_t map, long key);
int MidiUtilLongIntMap_get(MidiUtilLongIntMap_t map, long key, int default_value);
void MidiUtilLongIntMap_set(MidiUtilLongIntMap_t map, long key, int value);
void MidiUtilLongIntMap_remove(MidiUtilLongIntMap_t map, long key);
void MidiUtilLongIntMap_enumerate(MidiUtilLongIntMap_t map, int (*callback)(long key, int value, void *user_data), void *user_data);

MidiUtilLongPointerMap_t MidiUtilLongPointerMap_new(int number_of_buckets);
void MidiUtilLongPointerMap_setFreeCallback(MidiUtilLongPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void MidiUtilLongPointerMap_free(MidiUtilLongPointerMap_t map);
void MidiUtilLongPointerMap_clear(MidiUtilLongPointerMap_t map);
int MidiUtilLongPointerMap_hasKey(MidiUtilLongPointerMap_t map, long key);
void *MidiUtilLongPointerMap_get(MidiUtilLongPointerMap_t map, long key);
void MidiUtilLongPointerMap_set(MidiUtilLongPointerMap_t map, long key, void *value);
void MidiUtilLongPointerMap_remove(MidiUtilLongPointerMap_t map, long key);
void MidiUtilLongPointerMap_enumerate(MidiUtilLongPointerMap_t map, int (*callback)(long key, void *value, void *user_data), void *user_data);

MidiUtilPointerIntMap_t MidiUtilPointerIntMap_new(int number_of_buckets);
void MidiUtilPointerIntMap_free(MidiUtilPointerIntMap_t map);
void MidiUtilPointerIntMap_clear(MidiUtilPointerIntMap_t map);
int MidiUtilPointerIntMap_hasKey(MidiUtilPointerIntMap_t map, void *key);
int MidiUtilPointerIntMap_get(MidiUtilPointerIntMap_t map, void *key, int default_value);
void MidiUtilPointerIntMap_set(MidiUtilPointerIntMap_t map, void *key, int value);
void MidiUtilPointerIntMap_remove(MidiUtilPointerIntMap_t map, void *key);
void MidiUtilPointerIntMap_enumerate(MidiUtilPointerIntMap_t map, int (*callback)(void *key, int value, void *user_data), void *user_data);

MidiUtilPointerPointerMap_t MidiUtilPointerPointerMap_new(int number_of_buckets);
void MidiUtilPointerPointerMap_setFreeCallback(MidiUtilPointerPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void MidiUtilPointerPointerMap_free(MidiUtilPointerPointerMap_t map);
void MidiUtilPointerPointerMap_clear(MidiUtilPointerPointerMap_t map);
int MidiUtilPointerPointerMap_hasKey(MidiUtilPointerPointerMap_t map, void *key);
void *MidiUtilPointerPointerMap_get(MidiUtilPointerPointerMap_t map, void *key);
void MidiUtilPointerPointerMap_set(MidiUtilPointerPointerMap_t map, void *key, void *value);
void MidiUtilPointerPointerMap_remove(MidiUtilPointerPointerMap_t map, void *key);
void MidiUtilPointerPointerMap_enumerate(MidiUtilPointerPointerMap_t map, int (*callback)(void *key, void *value, void *user_data), void *user_data);

MidiUtilBlobIntMap_t MidiUtilBlobIntMap_new(int number_of_buckets);
void MidiUtilBlobIntMap_free(MidiUtilBlobIntMap_t map);
void MidiUtilBlobIntMap_clear(MidiUtilBlobIntMap_t map);
int MidiUtilBlobIntMap_hasKey(MidiUtilBlobIntMap_t map, unsigned char *key, int key_size);
int MidiUtilBlobIntMap_get(MidiUtilBlobIntMap_t map, unsigned char *key, int key_size, int default_value);
void MidiUtilBlobIntMap_set(MidiUtilBlobIntMap_t map, unsigned char *key, int key_size, int value);
void MidiUtilBlobIntMap_remove(MidiUtilBlobIntMap_t map, unsigned char *key, int key_size);
void MidiUtilBlobIntMap_enumerate(MidiUtilBlobIntMap_t map, int (*callback)(unsigned char *key, int key_size, int value, void *user_data), void *user_data);

MidiUtilBlobPointerMap_t MidiUtilBlobPointerMap_new(int number_of_buckets);
void MidiUtilBlobPointerMap_setFreeCallback(MidiUtilBlobPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void MidiUtilBlobPointerMap_free(MidiUtilBlobPointerMap_t map);
void MidiUtilBlobPointerMap_clear(MidiUtilBlobPointerMap_t map);
int MidiUtilBlobPointerMap_hasKey(MidiUtilBlobPointerMap_t map, unsigned char *key, int key_size);
void *MidiUtilBlobPointerMap_get(MidiUtilBlobPointerMap_t map, unsigned char *key, int key_size);
void MidiUtilBlobPointerMap_set(MidiUtilBlobPointerMap_t map, unsigned char *key, int key_size, void *value);
void MidiUtilBlobPointerMap_remove(MidiUtilBlobPointerMap_t map, unsigned char *key, int key_size);
void MidiUtilBlobPointerMap_enumerate(MidiUtilBlobPointerMap_t map, int (*callback)(unsigned char *key, int key_size, void *value, void *user_data), void *user_data);

MidiUtilStringIntMap_t MidiUtilStringIntMap_new(int number_of_buckets);
void MidiUtilStringIntMap_free(MidiUtilStringIntMap_t map);
void MidiUtilStringIntMap_clear(MidiUtilStringIntMap_t map);
int MidiUtilStringIntMap_hasKey(MidiUtilStringIntMap_t map, unsigned char *key);
int MidiUtilStringIntMap_get(MidiUtilStringIntMap_t map, unsigned char *key, int default_value);
void MidiUtilStringIntMap_set(MidiUtilStringIntMap_t map, unsigned char *key, int value);
void MidiUtilStringIntMap_remove(MidiUtilStringIntMap_t map, unsigned char *key);
void MidiUtilStringIntMap_enumerate(MidiUtilStringIntMap_t map, int (*callback)(unsigned char *key, int value, void *user_data), void *user_data);

MidiUtilStringPointerMap_t MidiUtilStringPointerMap_new(int number_of_buckets);
void MidiUtilStringPointerMap_setFreeCallback(MidiUtilStringPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void MidiUtilStringPointerMap_free(MidiUtilStringPointerMap_t map);
void MidiUtilStringPointerMap_clear(MidiUtilStringPointerMap_t map);
int MidiUtilStringPointerMap_hasKey(MidiUtilStringPointerMap_t map, unsigned char *key);
void *MidiUtilStringPointerMap_get(MidiUtilStringPointerMap_t map, unsigned char *key);
void MidiUtilStringPointerMap_set(MidiUtilStringPointerMap_t map, unsigned char *key, void *value);
void MidiUtilStringPointerMap_remove(MidiUtilStringPointerMap_t map, unsigned char *key);
void MidiUtilStringPointerMap_enumerate(MidiUtilStringPointerMap_t map, int (*callback)(unsigned char *key, void *value, void *user_data), void *user_data);

void MidiUtil_quicksort(int number_of_elements, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data);
void MidiUtil_heapsort(int number_of_elements, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data);

int MidiUtil_clamp(int i, int low, int high);

void MidiUtilMessage_setNoteOff(unsigned char *message, int channel, int note, int velocity);
void MidiUtilMessage_setNoteOn(unsigned char *message, int channel, int note, int velocity);
void MidiUtilMessage_setKeyPressure(unsigned char *message, int channel, int note, int amount);
void MidiUtilMessage_setControlChange(unsigned char *message, int channel, int number, int value);
void MidiUtilMessage_setProgramChange(unsigned char *message, int channel, int number);
void MidiUtilMessage_setChannelPressure(unsigned char *message, int channel, int amount);
void MidiUtilMessage_setPitchWheel(unsigned char *message, int channel, int value);

MidiUtilMessageType_t MidiUtilMessage_getType(const unsigned char *message);
int MidiUtilMessage_getSize(unsigned char *message);

int MidiUtilNoteOffMessage_getChannel(const unsigned char *message);
void MidiUtilNoteOffMessage_setChannel(unsigned char *message, int channel);
int MidiUtilNoteOffMessage_getNote(const unsigned char *message);
void MidiUtilNoteOffMessage_setNote(unsigned char *message, int note);
int MidiUtilNoteOffMessage_getVelocity(const unsigned char *message);
void MidiUtilNoteOffMessage_setVelocity(unsigned char *message, int velocity);

int MidiUtilNoteOnMessage_getChannel(const unsigned char *message);
void MidiUtilNoteOnMessage_setChannel(unsigned char *message, int channel);
int MidiUtilNoteOnMessage_getNote(const unsigned char *message);
void MidiUtilNoteOnMessage_setNote(unsigned char *message, int note);
int MidiUtilNoteOnMessage_getVelocity(const unsigned char *message);
void MidiUtilNoteOnMessage_setVelocity(unsigned char *message, int velocity);

int MidiUtilKeyPressureMessage_getChannel(const unsigned char *message);
void MidiUtilKeyPressureMessage_setChannel(unsigned char *message, int channel);
int MidiUtilKeyPressureMessage_getNote(const unsigned char *message);
void MidiUtilKeyPressureMessage_setNote(unsigned char *message, int note);
int MidiUtilKeyPressureMessage_getAmount(const unsigned char *message);
void MidiUtilKeyPressureMessage_setAmount(unsigned char *message, int amount);

int MidiUtilControlChangeMessage_getChannel(const unsigned char *message);
void MidiUtilControlChangeMessage_setChannel(unsigned char *message, int channel);
int MidiUtilControlChangeMessage_getNumber(const unsigned char *message);
void MidiUtilControlChangeMessage_setNumber(unsigned char *message, int number);
int MidiUtilControlChangeMessage_getValue(const unsigned char *message);
void MidiUtilControlChangeMessage_setValue(unsigned char *message, int value);

int MidiUtilProgramChangeMessage_getChannel(const unsigned char *message);
void MidiUtilProgramChangeMessage_setChannel(unsigned char *message, int channel);
int MidiUtilProgramChangeMessage_getNumber(const unsigned char *message);
void MidiUtilProgramChangeMessage_setNumber(unsigned char *message, int number);

int MidiUtilChannelPressureMessage_getChannel(const unsigned char *message);
void MidiUtilChannelPressureMessage_setChannel(unsigned char *message, int channel);
int MidiUtilChannelPressureMessage_getAmount(const unsigned char *message);
void MidiUtilChannelPressureMessage_setAmount(unsigned char *message, int amount);

int MidiUtilPitchWheelMessage_getChannel(const unsigned char *message);
void MidiUtilPitchWheelMessage_setChannel(unsigned char *message, int channel);
int MidiUtilPitchWheelMessage_getValue(const unsigned char *message);
void MidiUtilPitchWheelMessage_setValue(unsigned char *message, int value);

int MidiUtil_getNoteNumberFromName(char *note_name);
int MidiUtil_setNoteNameFromNumber(int note_number, char *note_name);

#ifdef __cplusplus
}
#endif

#endif
