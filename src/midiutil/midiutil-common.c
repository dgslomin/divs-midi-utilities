
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midiutil-common.h>

typedef struct MidiUtilIntIntMapNode *MidiUtilIntIntMapNode_t;
typedef struct MidiUtilIntPointerMapNode *MidiUtilIntPointerMapNode_t;
typedef struct MidiUtilLongIntMapNode *MidiUtilLongIntMapNode_t;
typedef struct MidiUtilLongPointerMapNode *MidiUtilLongPointerMapNode_t;
typedef struct MidiUtilPointerIntMapNode *MidiUtilPointerIntMapNode_t;
typedef struct MidiUtilPointerPointerMapNode *MidiUtilPointerPointerMapNode_t;
typedef struct MidiUtilBlobIntMapNode *MidiUtilBlobIntMapNode_t;
typedef struct MidiUtilBlobPointerMapNode *MidiUtilBlobPointerMapNode_t;
typedef struct MidiUtilStringIntMapNode *MidiUtilStringIntMapNode_t;
typedef struct MidiUtilStringPointerMapNode *MidiUtilStringPointerMapNode_t;

struct MidiUtilByteArray
{
	int capacity;
	int size;
	unsigned char *buffer;
};

struct MidiUtilIntArray
{
	MidiUtilByteArray_t byte_array;
};

struct MidiUtilLongArray
{
	MidiUtilByteArray_t byte_array;
};

struct MidiUtilFloatArray
{
	MidiUtilByteArray_t byte_array;
};

struct MidiUtilDoubleArray
{
	MidiUtilByteArray_t byte_array;
};

struct MidiUtilPointerArray
{
	MidiUtilByteArray_t byte_array;
};

struct MidiUtilString
{
	MidiUtilByteArray_t byte_array;
};

struct MidiUtilBlobArray
{
	MidiUtilIntArray_t offset_array;
	MidiUtilIntArray_t size_array;
	MidiUtilByteArray_t data;
};

struct MidiUtilStringArray
{
	MidiUtilBlobArray_t blob_array;
};

struct MidiUtilIntIntMap
{
	int number_of_buckets;
	MidiUtilIntIntMapNode_t *bucket_node_lists;
};

struct MidiUtilIntIntMapNode
{
	int key;
	int value;
	MidiUtilIntIntMapNode_t next;
};

struct MidiUtilIntPointerMap
{
	int number_of_buckets;
	MidiUtilIntPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct MidiUtilIntPointerMapNode
{
	int key;
	void *value;
	MidiUtilIntPointerMapNode_t next;
};

struct MidiUtilLongIntMap
{
	int number_of_buckets;
	MidiUtilLongIntMapNode_t *bucket_node_lists;
};

struct MidiUtilLongIntMapNode
{
	int key;
	int value;
	MidiUtilLongIntMapNode_t next;
};

struct MidiUtilLongPointerMap
{
	int number_of_buckets;
	MidiUtilLongPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct MidiUtilLongPointerMapNode
{
	int key;
	void *value;
	MidiUtilLongPointerMapNode_t next;
};

struct MidiUtilPointerIntMap
{
	int number_of_buckets;
	MidiUtilPointerIntMapNode_t *bucket_node_lists;
};

struct MidiUtilPointerIntMapNode
{
	void *key;
	int value;
	MidiUtilPointerIntMapNode_t next;
};

struct MidiUtilPointerPointerMap
{
	int number_of_buckets;
	MidiUtilPointerPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct MidiUtilPointerPointerMapNode
{
	void *key;
	void *value;
	MidiUtilPointerPointerMapNode_t next;
};

struct MidiUtilBlobIntMap
{
	int number_of_buckets;
	MidiUtilBlobIntMapNode_t *bucket_node_lists;
};

struct MidiUtilBlobIntMapNode
{
	unsigned char *key;
	int key_size;
	int value;
	MidiUtilBlobIntMapNode_t next;
};

struct MidiUtilBlobPointerMap
{
	int number_of_buckets;
	MidiUtilBlobPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct MidiUtilBlobPointerMapNode
{
	unsigned char *key;
	int key_size;
	void *value;
	MidiUtilBlobPointerMapNode_t next;
};

struct MidiUtilStringIntMap
{
	int number_of_buckets;
	MidiUtilStringIntMapNode_t *bucket_node_lists;
};

struct MidiUtilStringIntMapNode
{
	unsigned char *key;
	int value;
	MidiUtilStringIntMapNode_t next;
};

struct MidiUtilStringPointerMap
{
	int number_of_buckets;
	MidiUtilStringPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct MidiUtilStringPointerMapNode
{
	unsigned char *key;
	void *value;
	MidiUtilStringPointerMapNode_t next;
};

MidiUtilByteArray_t MidiUtilByteArray_new(int initial_capacity)
{
	MidiUtilByteArray_t array = (MidiUtilByteArray_t)(malloc(sizeof (struct MidiUtilByteArray)));
	array->capacity = initial_capacity;
	array->size = 0;
	array->buffer = (unsigned char *)(malloc(initial_capacity));
	memset(array->buffer, 0, initial_capacity);
	return array;
}

void MidiUtilByteArray_free(MidiUtilByteArray_t array)
{
	free(array->buffer);
	free(array);
}

void MidiUtilByteArray_clear(MidiUtilByteArray_t array)
{
	MidiUtilByteArray_setSize(array, 0);
}

int MidiUtilByteArray_getCapacity(MidiUtilByteArray_t array)
{
	return array->capacity;
}

void MidiUtilByteArray_setCapacity(MidiUtilByteArray_t array, int capacity)
{
	if (array->size > capacity) array->size = capacity;
	array->capacity = capacity;
	array->buffer = realloc(array->buffer, capacity);
	memset(array->buffer + array->size, 0, capacity - array->size);
}

int MidiUtilByteArray_getSize(MidiUtilByteArray_t array)
{
	return array->size;
}

void MidiUtilByteArray_setSize(MidiUtilByteArray_t array, int size)
{
	if (size < array->size)
	{
		memset(array->buffer + size, 0, array->size - size);
	}
	else if (size > array->capacity)
	{
		/* TODO: consider growth algorithm from <http://www.python.org/~jeremy/weblog/040115.html> */
		MidiUtilByteArray_setCapacity(array, size - (size % 4096) + 4096);
	}

	array->size = size;
}

unsigned char *MidiUtilByteArray_getBuffer(MidiUtilByteArray_t array)
{
	return array->buffer;
}

unsigned char MidiUtilByteArray_get(MidiUtilByteArray_t array, int element_number)
{
	if (element_number < array->size)
	{
		return array->buffer[element_number];
	}
	else
	{
		return 0;
	}
}

void MidiUtilByteArray_set(MidiUtilByteArray_t array, int element_number, unsigned char value)
{
	if (element_number >= array->size) MidiUtilByteArray_setSize(array, element_number + 1);
	array->buffer[element_number] = value;
}

void MidiUtilByteArray_add(MidiUtilByteArray_t array, unsigned char value)
{
	MidiUtilByteArray_set(array, array->size, value);
}

void MidiUtilByteArray_insert(MidiUtilByteArray_t array, int element_number, unsigned char value)
{
	MidiUtilByteArray_setSize(array, array->size + 1);
	memmove(array->buffer + element_number + 1, array->buffer + element_number, array->size - element_number + 1);
	array->buffer[element_number] = value;
}

void MidiUtilByteArray_remove(MidiUtilByteArray_t array, int element_number)
{
	memmove(array->buffer + element_number, array->buffer + element_number + 1, array->size - element_number - 1);
	MidiUtilByteArray_setSize(array, array->size - 1);
}

void MidiUtilByteArray_setValues(MidiUtilByteArray_t array, unsigned char *values, int number_of_values)
{
	MidiUtilByteArray_setSize(array, number_of_values);
	memcpy(array->buffer, values, number_of_values);
}

void MidiUtilByteArray_addValues(MidiUtilByteArray_t array, unsigned char *values, int number_of_values)
{
	int offset = array->size;
	MidiUtilByteArray_setSize(array, offset + number_of_values);
	memcpy(array->buffer + offset, values, number_of_values);
}

void MidiUtilByteArray_replaceValues(MidiUtilByteArray_t array, int element_number, unsigned char *values, int number_of_values)
{
	if (element_number + number_of_values > array->size) MidiUtilByteArray_setSize(array, element_number + number_of_values);
	memcpy(array->buffer + element_number, values, number_of_values);
}

void MidiUtilByteArray_insertValues(MidiUtilByteArray_t array, int element_number, unsigned char *values, int number_of_values)
{
	MidiUtilByteArray_setSize(array, array->size + number_of_values);
	memmove(array->buffer + element_number + number_of_values, array->buffer + element_number, array->size - element_number + number_of_values);
	memcpy(array->buffer + element_number, values, number_of_values);
}

void MidiUtilByteArray_removeValues(MidiUtilByteArray_t array, int element_number, int number_of_values)
{
	memmove(array->buffer + element_number, array->buffer + element_number + number_of_values, array->size - element_number - number_of_values);
	MidiUtilByteArray_setSize(array, array->size - number_of_values);
}

MidiUtilIntArray_t MidiUtilIntArray_new(int initial_capacity)
{
	MidiUtilIntArray_t array = (MidiUtilIntArray_t)(malloc(sizeof (struct MidiUtilIntArray)));
	array->byte_array = MidiUtilByteArray_new(initial_capacity * sizeof (int));
	return array;
}

void MidiUtilIntArray_free(MidiUtilIntArray_t array)
{
	MidiUtilByteArray_free(array->byte_array);
	free(array);
}

void MidiUtilIntArray_clear(MidiUtilIntArray_t array)
{
	MidiUtilByteArray_clear(array->byte_array);
}

int MidiUtilIntArray_getCapacity(MidiUtilIntArray_t array)
{
	return MidiUtilByteArray_getCapacity(array->byte_array) / sizeof (int);
}

void MidiUtilIntArray_setCapacity(MidiUtilIntArray_t array, int capacity)
{
	MidiUtilByteArray_setCapacity(array->byte_array, capacity * sizeof (int));
}

int MidiUtilIntArray_getSize(MidiUtilIntArray_t array)
{
	return MidiUtilByteArray_getSize(array->byte_array) / sizeof (int);
}

void MidiUtilIntArray_setSize(MidiUtilIntArray_t array, int size)
{
	MidiUtilByteArray_setSize(array->byte_array, size * sizeof (int));
}

int *MidiUtilIntArray_getBuffer(MidiUtilIntArray_t array)
{
	return (int *)(MidiUtilByteArray_getBuffer(array->byte_array));
}

int MidiUtilIntArray_get(MidiUtilIntArray_t array, int element_number)
{
	return MidiUtilIntArray_getBuffer(array)[element_number];
}

void MidiUtilIntArray_set(MidiUtilIntArray_t array, int element_number, int value)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (int), (unsigned char *)(&value), sizeof (int));
}

void MidiUtilIntArray_add(MidiUtilIntArray_t array, int value)
{
	MidiUtilIntArray_set(array, MidiUtilIntArray_getSize(array), value);
}

void MidiUtilIntArray_insert(MidiUtilIntArray_t array, int element_number, int value)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (int), (unsigned char *)(&value), sizeof (int));
}

void MidiUtilIntArray_remove(MidiUtilIntArray_t array, int element_number)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (int), sizeof (int));
}

void MidiUtilIntArray_setValues(MidiUtilIntArray_t array, int *values, int number_of_values)
{
	MidiUtilByteArray_setValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (int));
}

void MidiUtilIntArray_addValues(MidiUtilIntArray_t array, int *values, int number_of_values)
{
	MidiUtilByteArray_addValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (int));
}

void MidiUtilIntArray_replaceValues(MidiUtilIntArray_t array, int element_number, int *values, int number_of_values)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (int), (unsigned char *)(values), number_of_values * sizeof (int));
}

void MidiUtilIntArray_insertValues(MidiUtilIntArray_t array, int element_number, int *values, int number_of_values)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (int), (unsigned char *)(values), number_of_values * sizeof (int));
}

void MidiUtilIntArray_removeValues(MidiUtilIntArray_t array, int element_number, int number_of_values)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (int), number_of_values * sizeof (int));
}

MidiUtilLongArray_t MidiUtilLongArray_new(int initial_capacity)
{
	MidiUtilLongArray_t array = (MidiUtilLongArray_t)(malloc(sizeof (struct MidiUtilLongArray)));
	array->byte_array = MidiUtilByteArray_new(initial_capacity * sizeof (long));
	return array;
}

void MidiUtilLongArray_free(MidiUtilLongArray_t array)
{
	MidiUtilByteArray_free(array->byte_array);
	free(array);
}

void MidiUtilLongArray_clear(MidiUtilLongArray_t array)
{
	MidiUtilByteArray_clear(array->byte_array);
}

int MidiUtilLongArray_getCapacity(MidiUtilLongArray_t array)
{
	return MidiUtilByteArray_getCapacity(array->byte_array) / sizeof (long);
}

void MidiUtilLongArray_setCapacity(MidiUtilLongArray_t array, int capacity)
{
	MidiUtilByteArray_setCapacity(array->byte_array, capacity * sizeof (long));
}

int MidiUtilLongArray_getSize(MidiUtilLongArray_t array)
{
	return MidiUtilByteArray_getSize(array->byte_array) / sizeof (long);
}

void MidiUtilLongArray_setSize(MidiUtilLongArray_t array, int size)
{
	MidiUtilByteArray_setSize(array->byte_array, size * sizeof (long));
}

long *MidiUtilLongArray_getBuffer(MidiUtilLongArray_t array)
{
	return (long *)(MidiUtilByteArray_getBuffer(array->byte_array));
}

long MidiUtilLongArray_get(MidiUtilLongArray_t array, int element_number)
{
	return MidiUtilLongArray_getBuffer(array)[element_number];
}

void MidiUtilLongArray_set(MidiUtilLongArray_t array, int element_number, long value)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (long), (unsigned char *)(&value), sizeof (long));
}

void MidiUtilLongArray_add(MidiUtilLongArray_t array, long value)
{
	MidiUtilLongArray_set(array, MidiUtilLongArray_getSize(array), value);
}

void MidiUtilLongArray_insert(MidiUtilLongArray_t array, int element_number, long value)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (long), (unsigned char *)(&value), sizeof (long));
}

void MidiUtilLongArray_remove(MidiUtilLongArray_t array, int element_number)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (long), sizeof (long));
}

void MidiUtilLongArray_setValues(MidiUtilLongArray_t array, long *values, int number_of_values)
{
	MidiUtilByteArray_setValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (long));
}

void MidiUtilLongArray_addValues(MidiUtilLongArray_t array, long *values, int number_of_values)
{
	MidiUtilByteArray_addValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (long));
}

void MidiUtilLongArray_replaceValues(MidiUtilLongArray_t array, int element_number, long *values, int number_of_values)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (long), (unsigned char *)(values), number_of_values * sizeof (long));
}

void MidiUtilLongArray_insertValues(MidiUtilLongArray_t array, int element_number, long *values, int number_of_values)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (long), (unsigned char *)(values), number_of_values * sizeof (long));
}

void MidiUtilLongArray_removeValues(MidiUtilLongArray_t array, int element_number, int number_of_values)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (long), number_of_values * sizeof (long));
}

MidiUtilFloatArray_t MidiUtilFloatArray_new(int initial_capacity)
{
	MidiUtilFloatArray_t array = (MidiUtilFloatArray_t)(malloc(sizeof (struct MidiUtilFloatArray)));
	array->byte_array = MidiUtilByteArray_new(initial_capacity * sizeof (float));
	return array;
}

void MidiUtilFloatArray_free(MidiUtilFloatArray_t array)
{
	MidiUtilByteArray_free(array->byte_array);
	free(array);
}

void MidiUtilFloatArray_clear(MidiUtilFloatArray_t array)
{
	MidiUtilByteArray_clear(array->byte_array);
}

int MidiUtilFloatArray_getCapacity(MidiUtilFloatArray_t array)
{
	return MidiUtilByteArray_getCapacity(array->byte_array) / sizeof (float);
}

void MidiUtilFloatArray_setCapacity(MidiUtilFloatArray_t array, int capacity)
{
	MidiUtilByteArray_setCapacity(array->byte_array, capacity * sizeof (float));
}

int MidiUtilFloatArray_getSize(MidiUtilFloatArray_t array)
{
	return MidiUtilByteArray_getSize(array->byte_array) / sizeof (float);
}

void MidiUtilFloatArray_setSize(MidiUtilFloatArray_t array, int size)
{
	MidiUtilByteArray_setSize(array->byte_array, size * sizeof (float));
}

float *MidiUtilFloatArray_getBuffer(MidiUtilFloatArray_t array)
{
	return (float *)(MidiUtilByteArray_getBuffer(array->byte_array));
}

float MidiUtilFloatArray_get(MidiUtilFloatArray_t array, int element_number)
{
	return MidiUtilFloatArray_getBuffer(array)[element_number];
}

void MidiUtilFloatArray_set(MidiUtilFloatArray_t array, int element_number, float value)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (float), (unsigned char *)(&value), sizeof (float));
}

void MidiUtilFloatArray_add(MidiUtilFloatArray_t array, float value)
{
	MidiUtilFloatArray_set(array, MidiUtilFloatArray_getSize(array), value);
}

void MidiUtilFloatArray_insert(MidiUtilFloatArray_t array, int element_number, float value)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (float), (unsigned char *)(&value), sizeof (float));
}

void MidiUtilFloatArray_remove(MidiUtilFloatArray_t array, int element_number)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (float), sizeof (float));
}

void MidiUtilFloatArray_setValues(MidiUtilFloatArray_t array, float *values, int number_of_values)
{
	MidiUtilByteArray_setValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (float));
}

void MidiUtilFloatArray_addValues(MidiUtilFloatArray_t array, float *values, int number_of_values)
{
	MidiUtilByteArray_addValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (float));
}

void MidiUtilFloatArray_replaceValues(MidiUtilFloatArray_t array, int element_number, float *values, int number_of_values)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (float), (unsigned char *)(values), number_of_values * sizeof (float));
}

void MidiUtilFloatArray_insertValues(MidiUtilFloatArray_t array, int element_number, float *values, int number_of_values)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (float), (unsigned char *)(values), number_of_values * sizeof (float));
}

void MidiUtilFloatArray_removeValues(MidiUtilFloatArray_t array, int element_number, int number_of_values)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (float), number_of_values * sizeof (float));
}

MidiUtilDoubleArray_t MidiUtilDoubleArray_new(int initial_capacity)
{
	MidiUtilDoubleArray_t array = (MidiUtilDoubleArray_t)(malloc(sizeof (struct MidiUtilDoubleArray)));
	array->byte_array = MidiUtilByteArray_new(initial_capacity * sizeof (double));
	return array;
}

void MidiUtilDoubleArray_free(MidiUtilDoubleArray_t array)
{
	MidiUtilByteArray_free(array->byte_array);
	free(array);
}

void MidiUtilDoubleArray_clear(MidiUtilDoubleArray_t array)
{
	MidiUtilByteArray_clear(array->byte_array);
}

int MidiUtilDoubleArray_getCapacity(MidiUtilDoubleArray_t array)
{
	return MidiUtilByteArray_getCapacity(array->byte_array) / sizeof (double);
}

void MidiUtilDoubleArray_setCapacity(MidiUtilDoubleArray_t array, int capacity)
{
	MidiUtilByteArray_setCapacity(array->byte_array, capacity * sizeof (double));
}

int MidiUtilDoubleArray_getSize(MidiUtilDoubleArray_t array)
{
	return MidiUtilByteArray_getSize(array->byte_array) / sizeof (double);
}

void MidiUtilDoubleArray_setSize(MidiUtilDoubleArray_t array, int size)
{
	MidiUtilByteArray_setSize(array->byte_array, size * sizeof (double));
}

double *MidiUtilDoubleArray_getBuffer(MidiUtilDoubleArray_t array)
{
	return (double *)(MidiUtilByteArray_getBuffer(array->byte_array));
}

double MidiUtilDoubleArray_get(MidiUtilDoubleArray_t array, int element_number)
{
	return MidiUtilDoubleArray_getBuffer(array)[element_number];
}

void MidiUtilDoubleArray_set(MidiUtilDoubleArray_t array, int element_number, double value)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (double), (unsigned char *)(&value), sizeof (double));
}

void MidiUtilDoubleArray_add(MidiUtilDoubleArray_t array, double value)
{
	MidiUtilDoubleArray_set(array, MidiUtilDoubleArray_getSize(array), value);
}

void MidiUtilDoubleArray_insert(MidiUtilDoubleArray_t array, int element_number, double value)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (double), (unsigned char *)(&value), sizeof (double));
}

void MidiUtilDoubleArray_remove(MidiUtilDoubleArray_t array, int element_number)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (double), sizeof (double));
}

void MidiUtilDoubleArray_setValues(MidiUtilDoubleArray_t array, double *values, int number_of_values)
{
	MidiUtilByteArray_setValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (double));
}

void MidiUtilDoubleArray_addValues(MidiUtilDoubleArray_t array, double *values, int number_of_values)
{
	MidiUtilByteArray_addValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (double));
}

void MidiUtilDoubleArray_replaceValues(MidiUtilDoubleArray_t array, int element_number, double *values, int number_of_values)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (double), (unsigned char *)(values), number_of_values * sizeof (double));
}

void MidiUtilDoubleArray_insertValues(MidiUtilDoubleArray_t array, int element_number, double *values, int number_of_values)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (double), (unsigned char *)(values), number_of_values * sizeof (double));
}

void MidiUtilDoubleArray_removeValues(MidiUtilDoubleArray_t array, int element_number, int number_of_values)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (double), number_of_values * sizeof (double));
}

MidiUtilPointerArray_t MidiUtilPointerArray_new(int initial_capacity)
{
	MidiUtilPointerArray_t array = (MidiUtilPointerArray_t)(malloc(sizeof (struct MidiUtilPointerArray)));
	array->byte_array = MidiUtilByteArray_new(initial_capacity * sizeof (void *));
	return array;
}

void MidiUtilPointerArray_free(MidiUtilPointerArray_t array)
{
	MidiUtilByteArray_free(array->byte_array);
	free(array);
}

void MidiUtilPointerArray_clear(MidiUtilPointerArray_t array)
{
	MidiUtilByteArray_clear(array->byte_array);
}

int MidiUtilPointerArray_getCapacity(MidiUtilPointerArray_t array)
{
	return MidiUtilByteArray_getCapacity(array->byte_array) / sizeof (void *);
}

void MidiUtilPointerArray_setCapacity(MidiUtilPointerArray_t array, int capacity)
{
	MidiUtilByteArray_setCapacity(array->byte_array, capacity * sizeof (void *));
}

int MidiUtilPointerArray_getSize(MidiUtilPointerArray_t array)
{
	return MidiUtilByteArray_getSize(array->byte_array) / sizeof (void *);
}

void MidiUtilPointerArray_setSize(MidiUtilPointerArray_t array, int size)
{
	MidiUtilByteArray_setSize(array->byte_array, size * sizeof (void *));
}

void **MidiUtilPointerArray_getBuffer(MidiUtilPointerArray_t array)
{
	return (void **)(MidiUtilByteArray_getBuffer(array->byte_array));
}

void *MidiUtilPointerArray_get(MidiUtilPointerArray_t array, int element_number)
{
	return MidiUtilPointerArray_getBuffer(array)[element_number];
}

void MidiUtilPointerArray_set(MidiUtilPointerArray_t array, int element_number, void *value)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (void *), (unsigned char *)(&value), sizeof (void *));
}

void MidiUtilPointerArray_add(MidiUtilPointerArray_t array, void *value)
{
	MidiUtilPointerArray_set(array, MidiUtilPointerArray_getSize(array), value);
}

void MidiUtilPointerArray_insert(MidiUtilPointerArray_t array, int element_number, void *value)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (void *), (unsigned char *)(&value), sizeof (void *));
}

void MidiUtilPointerArray_remove(MidiUtilPointerArray_t array, int element_number)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (void *), sizeof (void *));
}

void MidiUtilPointerArray_setValues(MidiUtilPointerArray_t array, void **values, int number_of_values)
{
	MidiUtilByteArray_setValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (void *));
}

void MidiUtilPointerArray_addValues(MidiUtilPointerArray_t array, void **values, int number_of_values)
{
	MidiUtilByteArray_addValues(array->byte_array, (unsigned char *)(values), number_of_values * sizeof (void *));
}

void MidiUtilPointerArray_replaceValues(MidiUtilPointerArray_t array, int element_number, void **values, int number_of_values)
{
	MidiUtilByteArray_replaceValues(array->byte_array, element_number * sizeof (void *), (unsigned char *)(values), number_of_values * sizeof (void *));
}

void MidiUtilPointerArray_insertValues(MidiUtilPointerArray_t array, int element_number, void **values, int number_of_values)
{
	MidiUtilByteArray_insertValues(array->byte_array, element_number * sizeof (void *), (unsigned char *)(values), number_of_values * sizeof (void *));
}

void MidiUtilPointerArray_removeValues(MidiUtilPointerArray_t array, int element_number, int number_of_values)
{
	MidiUtilByteArray_removeValues(array->byte_array, element_number * sizeof (void *), number_of_values * sizeof (void *));
}

MidiUtilString_t MidiUtilString_new(int initial_capacity)
{
	MidiUtilString_t string = (MidiUtilString_t)(malloc(sizeof (struct MidiUtilString)));
	string->byte_array = MidiUtilByteArray_new(initial_capacity);
	MidiUtilByteArray_set(string->byte_array, 0, '\0');
	return string;
}

void MidiUtilString_free(MidiUtilString_t string)
{
	MidiUtilByteArray_free(string->byte_array);
	free(string);
}

void MidiUtilString_clear(MidiUtilString_t string)
{
	MidiUtilByteArray_clear(string->byte_array);
	MidiUtilByteArray_set(string->byte_array, 0, '\0');
}

int MidiUtilString_getCapacity(MidiUtilString_t string)
{
	return MidiUtilByteArray_getCapacity(string->byte_array);
}

void MidiUtilString_setCapacity(MidiUtilString_t string, int capacity)
{
	MidiUtilByteArray_setCapacity(string->byte_array, capacity);
}

int MidiUtilString_getSize(MidiUtilString_t string)
{
	return MidiUtilByteArray_getSize(string->byte_array) - 1;
}

unsigned char *MidiUtilString_get(MidiUtilString_t string)
{
	return MidiUtilByteArray_getBuffer(string->byte_array);
}

void MidiUtilString_set(MidiUtilString_t string, unsigned char *value)
{
	MidiUtilByteArray_setValues(string->byte_array, value, strlen((char *)(value)) + 1);
}

void MidiUtilString_addChar(MidiUtilString_t string, unsigned char value)
{
	int size = MidiUtilString_getSize(string);
	MidiUtilByteArray_set(string->byte_array, size, value);
	MidiUtilByteArray_set(string->byte_array, size + 1, '\0');
}

void MidiUtilString_addString(MidiUtilString_t string, unsigned char *value)
{
	MidiUtilByteArray_replaceValues(string->byte_array, MidiUtilString_getSize(string), value, strlen((char *)(value)) + 1);
}

MidiUtilBlobArray_t MidiUtilBlobArray_new(int initial_element_capacity, int initial_byte_capacity)
{
	MidiUtilBlobArray_t array = (MidiUtilBlobArray_t)(malloc(sizeof (struct MidiUtilBlobArray)));
	array->offset_array = MidiUtilIntArray_new(initial_element_capacity);
	array->size_array = MidiUtilIntArray_new(initial_element_capacity);
	array->data = MidiUtilByteArray_new(initial_byte_capacity);
	return array;
}

void MidiUtilBlobArray_free(MidiUtilBlobArray_t array)
{
	MidiUtilByteArray_free(array->data);
	MidiUtilIntArray_free(array->size_array);
	MidiUtilIntArray_free(array->offset_array);
	free(array);
}

void MidiUtilBlobArray_clear(MidiUtilBlobArray_t array)
{
	MidiUtilIntArray_clear(array->offset_array);
	MidiUtilIntArray_clear(array->size_array);
	MidiUtilByteArray_clear(array->data);
}

int MidiUtilBlobArray_getSize(MidiUtilBlobArray_t array)
{
	return MidiUtilIntArray_getSize(array->offset_array);
}

unsigned char *MidiUtilBlobArray_getValue(MidiUtilBlobArray_t array, int element_number)
{
	return MidiUtilByteArray_getBuffer(array->data) + MidiUtilIntArray_get(array->offset_array, element_number);
}

int MidiUtilBlobArray_getValueSize(MidiUtilBlobArray_t array, int element_number)
{
	return MidiUtilIntArray_get(array->size_array, element_number);
}

void MidiUtilBlobArray_set(MidiUtilBlobArray_t array, int element_number, unsigned char *value, int value_size)
{
	int size = MidiUtilBlobArray_getSize(array);
	int offset;

	if ((element_number < size) && (value_size <= MidiUtilIntArray_get(array->size_array, element_number)))
	{
		offset = MidiUtilIntArray_get(array->offset_array, element_number);
	}
	else
	{
		if (size == 0)
		{
			offset = 0;
		}
		else
		{
			offset = MidiUtilIntArray_get(array->offset_array, size - 1) + MidiUtilIntArray_get(array->size_array, size - 1);
		}

		MidiUtilIntArray_set(array->offset_array, element_number, offset);
		MidiUtilByteArray_setSize(array->data, offset + value_size);
	}

	MidiUtilIntArray_set(array->size_array, element_number, value_size);
	if (value_size > 0) memcpy(MidiUtilByteArray_getBuffer(array->data) + offset, value, value_size);
}

void MidiUtilBlobArray_add(MidiUtilBlobArray_t array, unsigned char *value, int value_size)
{
	MidiUtilBlobArray_set(array, MidiUtilBlobArray_getSize(array), value, value_size);
}

void MidiUtilBlobArray_insert(MidiUtilBlobArray_t array, int element_number, unsigned char *value, int value_size)
{
	int size = MidiUtilBlobArray_getSize(array);
	int offset = MidiUtilIntArray_get(array->offset_array, size - 1) + MidiUtilIntArray_get(array->size_array, size - 1);

	if (value_size > 0)
	{
		MidiUtilByteArray_setSize(array->data, offset + value_size);
		memcpy(MidiUtilByteArray_getBuffer(array->data) + offset, value, value_size);
	}

	MidiUtilIntArray_insert(array->offset_array, element_number, offset);
	MidiUtilIntArray_insert(array->size_array, element_number, value_size);
}

void MidiUtilBlobArray_remove(MidiUtilBlobArray_t array, int element_number)
{
	MidiUtilBlobArray_set(array, element_number, NULL, 0);
}

MidiUtilStringArray_t MidiUtilStringArray_new(int initial_element_capacity, int initial_byte_capacity)
{
	MidiUtilStringArray_t array = (MidiUtilStringArray_t)(malloc(sizeof (struct MidiUtilStringArray)));
	array->blob_array = MidiUtilBlobArray_new(initial_element_capacity, initial_byte_capacity);
	return array;
}

void MidiUtilStringArray_free(MidiUtilStringArray_t array)
{
	MidiUtilBlobArray_free(array->blob_array);
	free(array);
}

void MidiUtilStringArray_clear(MidiUtilStringArray_t array)
{
	MidiUtilBlobArray_clear(array->blob_array);
}

int MidiUtilStringArray_getSize(MidiUtilStringArray_t array)
{
	return MidiUtilBlobArray_getSize(array->blob_array);
}

unsigned char *MidiUtilStringArray_get(MidiUtilStringArray_t array, int element_number)
{
	return MidiUtilBlobArray_getValue(array->blob_array, element_number);
}

void MidiUtilStringArray_set(MidiUtilStringArray_t array, int element_number, unsigned char *value)
{
	MidiUtilBlobArray_set(array->blob_array, element_number, value, strlen((char *)(value)) + 1);
}

void MidiUtilStringArray_add(MidiUtilStringArray_t array, unsigned char *value)
{
	MidiUtilBlobArray_add(array->blob_array, value, strlen((char *)(value)) + 1);
}

void MidiUtilStringArray_insert(MidiUtilStringArray_t array, int element_number, unsigned char *value)
{
	MidiUtilBlobArray_insert(array->blob_array, element_number, value, strlen((char *)(value)) + 1);
}

void MidiUtilStringArray_remove(MidiUtilStringArray_t array, int element_number)
{
	MidiUtilBlobArray_remove(array->blob_array, element_number);
}

static int _hashInt(int key, int number_of_buckets)
{
	return (key * 62342347) % number_of_buckets;
}

MidiUtilIntIntMap_t MidiUtilIntIntMap_new(int number_of_buckets)
{
	MidiUtilIntIntMap_t map = (MidiUtilIntIntMap_t)(malloc(sizeof (struct MidiUtilIntIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilIntIntMapNode_t *)(malloc(sizeof (MidiUtilIntIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void MidiUtilIntIntMap_free(MidiUtilIntIntMap_t map)
{
	MidiUtilIntIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilIntIntMap_clear(MidiUtilIntIntMap_t map)
{
	int bucket_number;
	MidiUtilIntIntMapNode_t node;
	MidiUtilIntIntMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilIntIntMap_hasKey(MidiUtilIntIntMap_t map, int key)
{
	MidiUtilIntIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

int MidiUtilIntIntMap_get(MidiUtilIntIntMap_t map, int key, int default_value)
{
	MidiUtilIntIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return default_value;
}

void MidiUtilIntIntMap_set(MidiUtilIntIntMap_t map, int key, int value)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	MidiUtilIntIntMapNode_t node;
	MidiUtilIntIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilIntIntMapNode_t)(malloc(sizeof (struct MidiUtilIntIntMapNode)));
	node->key = key;
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilIntIntMap_remove(MidiUtilIntIntMap_t map, int key)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	MidiUtilIntIntMapNode_t node;
	MidiUtilIntIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilIntIntMap_enumerate(MidiUtilIntIntMap_t map, int (*callback)(int key, int value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilIntIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

MidiUtilIntPointerMap_t MidiUtilIntPointerMap_new(int number_of_buckets)
{
	MidiUtilIntPointerMap_t map = (MidiUtilIntPointerMap_t)(malloc(sizeof (struct MidiUtilIntPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilIntPointerMapNode_t *)(malloc(sizeof (MidiUtilIntPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void MidiUtilIntPointerMap_setFreeCallback(MidiUtilIntPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void MidiUtilIntPointerMap_free(MidiUtilIntPointerMap_t map)
{
	MidiUtilIntPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilIntPointerMap_clear(MidiUtilIntPointerMap_t map)
{
	int bucket_number;
	MidiUtilIntPointerMapNode_t node;
	MidiUtilIntPointerMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilIntPointerMap_hasKey(MidiUtilIntPointerMap_t map, int key)
{
	MidiUtilIntPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

void *MidiUtilIntPointerMap_get(MidiUtilIntPointerMap_t map, int key)
{
	MidiUtilIntPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return NULL;
}

void MidiUtilIntPointerMap_set(MidiUtilIntPointerMap_t map, int key, void *value)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	MidiUtilIntPointerMapNode_t node;
	MidiUtilIntPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilIntPointerMapNode_t)(malloc(sizeof (struct MidiUtilIntPointerMapNode)));
	node->key = key;
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilIntPointerMap_remove(MidiUtilIntPointerMap_t map, int key)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	MidiUtilIntPointerMapNode_t node;
	MidiUtilIntPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilIntPointerMap_enumerate(MidiUtilIntPointerMap_t map, int (*callback)(int key, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilIntPointerMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

MidiUtilLongIntMap_t MidiUtilLongIntMap_new(int number_of_buckets)
{
	MidiUtilLongIntMap_t map = (MidiUtilLongIntMap_t)(malloc(sizeof (struct MidiUtilLongIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilLongIntMapNode_t *)(malloc(sizeof (MidiUtilLongIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void MidiUtilLongIntMap_free(MidiUtilLongIntMap_t map)
{
	MidiUtilLongIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilLongIntMap_clear(MidiUtilLongIntMap_t map)
{
	int bucket_number;
	MidiUtilLongIntMapNode_t node;
	MidiUtilLongIntMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilLongIntMap_hasKey(MidiUtilLongIntMap_t map, long key)
{
	MidiUtilLongIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

int MidiUtilLongIntMap_get(MidiUtilLongIntMap_t map, long key, int default_value)
{
	MidiUtilLongIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return default_value;
}

void MidiUtilLongIntMap_set(MidiUtilLongIntMap_t map, long key, int value)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	MidiUtilLongIntMapNode_t node;
	MidiUtilLongIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilLongIntMapNode_t)(malloc(sizeof (struct MidiUtilLongIntMapNode)));
	node->key = key;
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilLongIntMap_remove(MidiUtilLongIntMap_t map, long key)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	MidiUtilLongIntMapNode_t node;
	MidiUtilLongIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilLongIntMap_enumerate(MidiUtilLongIntMap_t map, int (*callback)(long key, int value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilLongIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

MidiUtilLongPointerMap_t MidiUtilLongPointerMap_new(int number_of_buckets)
{
	MidiUtilLongPointerMap_t map = (MidiUtilLongPointerMap_t)(malloc(sizeof (struct MidiUtilLongPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilLongPointerMapNode_t *)(malloc(sizeof (MidiUtilLongPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void MidiUtilLongPointerMap_setFreeCallback(MidiUtilLongPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void MidiUtilLongPointerMap_free(MidiUtilLongPointerMap_t map)
{
	MidiUtilLongPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilLongPointerMap_clear(MidiUtilLongPointerMap_t map)
{
	int bucket_number;
	MidiUtilLongPointerMapNode_t node;
	MidiUtilLongPointerMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilLongPointerMap_hasKey(MidiUtilLongPointerMap_t map, long key)
{
	MidiUtilLongPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

void *MidiUtilLongPointerMap_get(MidiUtilLongPointerMap_t map, long key)
{
	MidiUtilLongPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return NULL;
}

void MidiUtilLongPointerMap_set(MidiUtilLongPointerMap_t map, long key, void *value)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	MidiUtilLongPointerMapNode_t node;
	MidiUtilLongPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilLongPointerMapNode_t)(malloc(sizeof (struct MidiUtilLongPointerMapNode)));
	node->key = key;
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilLongPointerMap_remove(MidiUtilLongPointerMap_t map, long key)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	MidiUtilLongPointerMapNode_t node;
	MidiUtilLongPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilLongPointerMap_enumerate(MidiUtilLongPointerMap_t map, int (*callback)(long key, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilLongPointerMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

static int _hashPointer(void *key, int number_of_buckets)
{
	/* TODO: check this for correctness */
	return (int)(((long)(key) * 62342347) % number_of_buckets);
}

MidiUtilPointerIntMap_t MidiUtilPointerIntMap_new(int number_of_buckets)
{
	MidiUtilPointerIntMap_t map = (MidiUtilPointerIntMap_t)(malloc(sizeof (struct MidiUtilPointerIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilPointerIntMapNode_t *)(malloc(sizeof (MidiUtilPointerIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void MidiUtilPointerIntMap_free(MidiUtilPointerIntMap_t map)
{
	MidiUtilPointerIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilPointerIntMap_clear(MidiUtilPointerIntMap_t map)
{
	int bucket_number;
	MidiUtilPointerIntMapNode_t node;
	MidiUtilPointerIntMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilPointerIntMap_hasKey(MidiUtilPointerIntMap_t map, void *key)
{
	MidiUtilPointerIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashPointer(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

int MidiUtilPointerIntMap_get(MidiUtilPointerIntMap_t map, void *key, int default_value)
{
	MidiUtilPointerIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashPointer(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return default_value;
}

void MidiUtilPointerIntMap_set(MidiUtilPointerIntMap_t map, void *key, int value)
{
	int bucket_number = _hashPointer(key, map->number_of_buckets);
	MidiUtilPointerIntMapNode_t node;
	MidiUtilPointerIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilPointerIntMapNode_t)(malloc(sizeof (struct MidiUtilPointerIntMapNode)));
	node->key = key;
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilPointerIntMap_remove(MidiUtilPointerIntMap_t map, void *key)
{
	int bucket_number = _hashPointer(key, map->number_of_buckets);
	MidiUtilPointerIntMapNode_t node;
	MidiUtilPointerIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilPointerIntMap_enumerate(MidiUtilPointerIntMap_t map, int (*callback)(void *key, int value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilPointerIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

MidiUtilPointerPointerMap_t MidiUtilPointerPointerMap_new(int number_of_buckets)
{
	MidiUtilPointerPointerMap_t map = (MidiUtilPointerPointerMap_t)(malloc(sizeof (struct MidiUtilPointerPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilPointerPointerMapNode_t *)(malloc(sizeof (MidiUtilPointerPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void MidiUtilPointerPointerMap_setFreeCallback(MidiUtilPointerPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void MidiUtilPointerPointerMap_free(MidiUtilPointerPointerMap_t map)
{
	MidiUtilPointerPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilPointerPointerMap_clear(MidiUtilPointerPointerMap_t map)
{
	int bucket_number;
	MidiUtilPointerPointerMapNode_t node;
	MidiUtilPointerPointerMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilPointerPointerMap_hasKey(MidiUtilPointerPointerMap_t map, void *key)
{
	MidiUtilPointerPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashPointer(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

void *MidiUtilPointerPointerMap_get(MidiUtilPointerPointerMap_t map, void *key)
{
	MidiUtilPointerPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashPointer(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return NULL;
}

void MidiUtilPointerPointerMap_set(MidiUtilPointerPointerMap_t map, void *key, void *value)
{
	int bucket_number = _hashPointer(key, map->number_of_buckets);
	MidiUtilPointerPointerMapNode_t node;
	MidiUtilPointerPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilPointerPointerMapNode_t)(malloc(sizeof (struct MidiUtilPointerPointerMapNode)));
	node->key = key;
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilPointerPointerMap_remove(MidiUtilPointerPointerMap_t map, void *key)
{
	int bucket_number = _hashPointer(key, map->number_of_buckets);
	MidiUtilPointerPointerMapNode_t node;
	MidiUtilPointerPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (node->key == key)
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilPointerPointerMap_enumerate(MidiUtilPointerPointerMap_t map, int (*callback)(void *key, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilPointerPointerMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

static int _hashBlob(unsigned char *key, int key_size, int number_of_buckets)
{
	int i;
	int bucket_number = 0;
	for (i = 0; i < key_size; i++) bucket_number = ((bucket_number * 128) + (int)(key[i])) % number_of_buckets;
	return bucket_number;
}

MidiUtilBlobIntMap_t MidiUtilBlobIntMap_new(int number_of_buckets)
{
	MidiUtilBlobIntMap_t map = (MidiUtilBlobIntMap_t)(malloc(sizeof (struct MidiUtilBlobIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilBlobIntMapNode_t *)(malloc(sizeof (MidiUtilBlobIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void MidiUtilBlobIntMap_free(MidiUtilBlobIntMap_t map)
{
	MidiUtilBlobIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilBlobIntMap_clear(MidiUtilBlobIntMap_t map)
{
	int bucket_number;
	MidiUtilBlobIntMapNode_t node;
	MidiUtilBlobIntMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			next_node = node->next;
			free(node->key);
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilBlobIntMap_hasKey(MidiUtilBlobIntMap_t map, unsigned char *key, int key_size)
{
	MidiUtilBlobIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashBlob(key, key_size, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0)) return 1;
	}

	return 0;
}

int MidiUtilBlobIntMap_get(MidiUtilBlobIntMap_t map, unsigned char *key, int key_size, int default_value)
{
	MidiUtilBlobIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashBlob(key, key_size, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0)) return node->value;
	}

	return default_value;
}

void MidiUtilBlobIntMap_set(MidiUtilBlobIntMap_t map, unsigned char *key, int key_size, int value)
{
	int bucket_number = _hashBlob(key, key_size, map->number_of_buckets);
	MidiUtilBlobIntMapNode_t node;
	MidiUtilBlobIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0))
		{
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilBlobIntMapNode_t)(malloc(sizeof (struct MidiUtilBlobIntMapNode)));
	node->key = malloc(key_size);
	memcpy(node->key, key, key_size);
	node->key_size = key_size;
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilBlobIntMap_remove(MidiUtilBlobIntMap_t map, unsigned char *key, int key_size)
{
	int bucket_number = _hashBlob(key, key_size, map->number_of_buckets);
	MidiUtilBlobIntMapNode_t node;
	MidiUtilBlobIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0))
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			free(node->key);
			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilBlobIntMap_enumerate(MidiUtilBlobIntMap_t map, int (*callback)(unsigned char *key, int key_size, int value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilBlobIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->key_size, node->value, user_data)) break;
		}
	}
}

MidiUtilBlobPointerMap_t MidiUtilBlobPointerMap_new(int number_of_buckets)
{
	MidiUtilBlobPointerMap_t map = (MidiUtilBlobPointerMap_t)(malloc(sizeof (struct MidiUtilBlobPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilBlobPointerMapNode_t *)(malloc(sizeof (MidiUtilBlobPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void MidiUtilBlobPointerMap_setFreeCallback(MidiUtilBlobPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void MidiUtilBlobPointerMap_free(MidiUtilBlobPointerMap_t map)
{
	MidiUtilBlobPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilBlobPointerMap_clear(MidiUtilBlobPointerMap_t map)
{
	int bucket_number;
	MidiUtilBlobPointerMapNode_t node;
	MidiUtilBlobPointerMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			free(node->key);
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilBlobPointerMap_hasKey(MidiUtilBlobPointerMap_t map, unsigned char *key, int key_size)
{
	MidiUtilBlobPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashBlob(key, key_size, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0)) return 1;
	}

	return 0;
}

void *MidiUtilBlobPointerMap_get(MidiUtilBlobPointerMap_t map, unsigned char *key, int key_size)
{
	MidiUtilBlobPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashBlob(key, key_size, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0)) return node->value;
	}

	return NULL;
}

void MidiUtilBlobPointerMap_set(MidiUtilBlobPointerMap_t map, unsigned char *key, int key_size, void *value)
{
	int bucket_number = _hashBlob(key, key_size, map->number_of_buckets);
	MidiUtilBlobPointerMapNode_t node;
	MidiUtilBlobPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0))
		{
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilBlobPointerMapNode_t)(malloc(sizeof (struct MidiUtilBlobPointerMapNode)));
	node->key = malloc(key_size);
	memcpy(node->key, key, key_size);
	node->key_size = key_size;
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilBlobPointerMap_remove(MidiUtilBlobPointerMap_t map, unsigned char *key, int key_size)
{
	int bucket_number = _hashBlob(key, key_size, map->number_of_buckets);
	MidiUtilBlobPointerMapNode_t node;
	MidiUtilBlobPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0))
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			free(node->key);
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilBlobPointerMap_enumerate(MidiUtilBlobPointerMap_t map, int (*callback)(unsigned char *key, int key_size, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilBlobPointerMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->key_size, node->value, user_data)) break;
		}
	}
}

static int _hashString(unsigned char *key, int number_of_buckets)
{
	int i;
	int bucket_number = 0;
	for (i = 0; key[i] != '\0'; i++) bucket_number = ((bucket_number * 128) + (int)(key[i])) % number_of_buckets;
	return bucket_number;
}

MidiUtilStringIntMap_t MidiUtilStringIntMap_new(int number_of_buckets)
{
	MidiUtilStringIntMap_t map = (MidiUtilStringIntMap_t)(malloc(sizeof (struct MidiUtilStringIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilStringIntMapNode_t *)(malloc(sizeof (MidiUtilStringIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void MidiUtilStringIntMap_free(MidiUtilStringIntMap_t map)
{
	MidiUtilStringIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilStringIntMap_clear(MidiUtilStringIntMap_t map)
{
	int bucket_number;
	MidiUtilStringIntMapNode_t node;
	MidiUtilStringIntMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			free(node->key);
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilStringIntMap_hasKey(MidiUtilStringIntMap_t map, unsigned char *key)
{
	MidiUtilStringIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashString(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0) return 1;
	}

	return 0;
}

int MidiUtilStringIntMap_get(MidiUtilStringIntMap_t map, unsigned char *key, int default_value)
{
	MidiUtilStringIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashString(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0) return node->value;
	}

	return default_value;
}

void MidiUtilStringIntMap_set(MidiUtilStringIntMap_t map, unsigned char *key, int value)
{
	int bucket_number = _hashString(key, map->number_of_buckets);
	MidiUtilStringIntMapNode_t node;
	MidiUtilStringIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0)
		{
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilStringIntMapNode_t)(malloc(sizeof (struct MidiUtilStringIntMapNode)));
	node->key = (unsigned char *)(strdup((char *)(key)));
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilStringIntMap_remove(MidiUtilStringIntMap_t map, unsigned char *key)
{
	int bucket_number = _hashString(key, map->number_of_buckets);
	MidiUtilStringIntMapNode_t node;
	MidiUtilStringIntMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0)
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			free(node->key);
			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilStringIntMap_enumerate(MidiUtilStringIntMap_t map, int (*callback)(unsigned char *key, int value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilStringIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

MidiUtilStringPointerMap_t MidiUtilStringPointerMap_new(int number_of_buckets)
{
	MidiUtilStringPointerMap_t map = (MidiUtilStringPointerMap_t)(malloc(sizeof (struct MidiUtilStringPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (MidiUtilStringPointerMapNode_t *)(malloc(sizeof (MidiUtilStringPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void MidiUtilStringPointerMap_setFreeCallback(MidiUtilStringPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void MidiUtilStringPointerMap_free(MidiUtilStringPointerMap_t map)
{
	MidiUtilStringPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void MidiUtilStringPointerMap_clear(MidiUtilStringPointerMap_t map)
{
	int bucket_number;
	MidiUtilStringPointerMapNode_t node;
	MidiUtilStringPointerMapNode_t next_node = NULL;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = next_node)
		{
			free(node->key);
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			next_node = node->next;
			free(node);
		}

		map->bucket_node_lists[bucket_number] = NULL;
	}
}

int MidiUtilStringPointerMap_hasKey(MidiUtilStringPointerMap_t map, unsigned char *key)
{
	MidiUtilStringPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashString(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0) return 1;
	}

	return 0;
}

void *MidiUtilStringPointerMap_get(MidiUtilStringPointerMap_t map, unsigned char *key)
{
	MidiUtilStringPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashString(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0) return node->value;
	}

	return NULL;
}

void MidiUtilStringPointerMap_set(MidiUtilStringPointerMap_t map, unsigned char *key, void *value)
{
	int bucket_number = _hashString(key, map->number_of_buckets);
	MidiUtilStringPointerMapNode_t node;
	MidiUtilStringPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0)
		{
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			node->value = value;
			return;
		}
		else
		{
			previous_node = node;
		}
	}

	node = (MidiUtilStringPointerMapNode_t)(malloc(sizeof (struct MidiUtilStringPointerMapNode)));
	node->key = (unsigned char *)(strdup((char *)(key)));
	node->value = value;
	node->next = NULL;

	if (previous_node == NULL)
	{
		map->bucket_node_lists[bucket_number] = node;
	}
	else
	{
		previous_node->next = node;
	}
}

void MidiUtilStringPointerMap_remove(MidiUtilStringPointerMap_t map, unsigned char *key)
{
	int bucket_number = _hashString(key, map->number_of_buckets);
	MidiUtilStringPointerMapNode_t node;
	MidiUtilStringPointerMapNode_t previous_node = NULL;

	for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0)
		{
			if (previous_node == NULL)
			{
				map->bucket_node_lists[bucket_number] = node->next;
			}
			else
			{
				previous_node->next = node->next;
			}

			free(node->key);
			if (map->free_callback != NULL) (*(map->free_callback))(node->value, map->free_callback_user_data);
			free(node);
			return;
		}
		else
		{
			previous_node = node;
		}
	}
}

void MidiUtilStringPointerMap_enumerate(MidiUtilStringPointerMap_t map, int (*callback)(unsigned char *key, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	MidiUtilStringPointerMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

static void quicksort_helper(int begin, int end, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data)
{
	if (end > begin + 1)
	{
		int left = begin + 1;
		int right = end;
		int pivot = begin;
    
		while (left < right)
		{
			if ((*compare_callback)(left, pivot, user_data) <= 0)
			{
				left++;
			}
			else
			{
				if (pivot == left)
				{
					pivot = right - 1;
				}
				else if (pivot == right - 1)
				{
					pivot = left;
				}

				(*exchange_callback)(left, --right, user_data);
			}
		}
    
		(*exchange_callback)(--left, begin, user_data);
		quicksort_helper(begin, left, compare_callback, exchange_callback, user_data);
		quicksort_helper(right, end, compare_callback, exchange_callback, user_data);
	}
}

void MidiUtil_quicksort(int number_of_elements, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data)
{
	quicksort_helper(0, number_of_elements, compare_callback, exchange_callback, user_data);
}

static void heapsort_sift_down(int start, int end, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data)
{
	int root = start;

	while (root * 2 + 1 <= end)
	{
		int child = root * 2 + 1;

		if ((child < end) && ((*compare_callback)(child, child + 1, user_data) < 0)) child++;

		if ((*compare_callback)(root, child, user_data) < 0)
		{
			(*exchange_callback)(root, child, user_data);
			root = child;
		}
		else
		{
			return;
		}
	}
}

void MidiUtil_heapsort(int number_of_elements, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data)
{
	if (number_of_elements > 1)
	{
		int end;
		int start = (number_of_elements - 1) / 2;

		while (1)
		{
			heapsort_sift_down(start, number_of_elements - 1, compare_callback, exchange_callback, user_data);
			if (start == 0) break;
			start--;
		}

		end = number_of_elements - 1;

		while (end > 0)
		{
			(*exchange_callback)(end, 0, user_data);
			end--;
			heapsort_sift_down(0, end, compare_callback, exchange_callback, user_data);
		}
	}
}

int MidiUtil_clamp(int i, int low, int high)
{
	return i < low ? low : i > high ? high : i;
}

void MidiUtilMessage_setNoteOff(unsigned char *message, int channel, int note, int velocity)
{
	message[0] = MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF | channel;
	message[1] = note;
	message[2] = velocity;
}

void MidiUtilMessage_setNoteOn(unsigned char *message, int channel, int note, int velocity)
{
	message[0] = MIDI_UTIL_MESSAGE_TYPE_NOTE_ON | channel;
	message[1] = note;
	message[2] = velocity;
}

void MidiUtilMessage_setKeyPressure(unsigned char *message, int channel, int note, int amount)
{
	message[0] = MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE | channel;
	message[1] = note;
	message[2] = amount;
}

void MidiUtilMessage_setControlChange(unsigned char *message, int channel, int number, int value)
{
	message[0] = MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE | channel;
	message[1] = number;
	message[2] = value;
}

void MidiUtilMessage_setProgramChange(unsigned char *message, int channel, int number)
{
	message[0] = MIDI_UTIL_MESSAGE_TYPE_PROGRAM_CHANGE | channel;
	message[1] = number;
}

void MidiUtilMessage_setChannelPressure(unsigned char *message, int channel, int amount)
{
	message[0] = MIDI_UTIL_MESSAGE_TYPE_CHANNEL_PRESSURE | channel;
	message[1] = amount;
}

void MidiUtilMessage_setPitchWheel(unsigned char *message, int channel, int value)
{
	message[0] = MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL | channel;
	message[1] = value & 0x7F;
	message[2] = (value >> 7) & 0x7F;
}

MidiUtilMessageType_t MidiUtilMessage_getType(const unsigned char *message)
{
	return (message[0] & 0xF0);
}

int MidiUtilMessage_getSize(const unsigned char *message)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF: return MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF;
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON: return MIDI_UTIL_MESSAGE_SIZE_NOTE_ON;
		case MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE: return MIDI_UTIL_MESSAGE_SIZE_KEY_PRESSURE;
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE: return MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE;
		case MIDI_UTIL_MESSAGE_TYPE_PROGRAM_CHANGE: return MIDI_UTIL_MESSAGE_SIZE_PROGRAM_CHANGE;
		case MIDI_UTIL_MESSAGE_TYPE_CHANNEL_PRESSURE: return MIDI_UTIL_MESSAGE_SIZE_CHANNEL_PRESSURE;
		case MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL: return MIDI_UTIL_MESSAGE_SIZE_PITCH_WHEEL;
		default: return -1;
	}
}

int MidiUtilMessage_getChannel(const unsigned char *message)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		case MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE:
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE:
		case MIDI_UTIL_MESSAGE_TYPE_PROGRAM_CHANGE:
		case MIDI_UTIL_MESSAGE_TYPE_CHANNEL_PRESSURE:
		case MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL:
		{
			return (message[0] & 0x0F);
		}
		default:
		{
			return -1;
		}
	}
}

void MidiUtilMessage_setChannel(unsigned char *message, int channel)
{
	message[0] = (message[0] & ~0x0F) | channel;
}

int MidiUtilNoteOffMessage_getChannel(const unsigned char *message)
{
	return (message[0] & 0x0F);
}

void MidiUtilNoteOffMessage_setChannel(unsigned char *message, int channel)
{
	message[0] = (message[0] & ~0x0F) | channel;
}

int MidiUtilNoteOffMessage_getNote(const unsigned char *message)
{
	return message[1];
}

void MidiUtilNoteOffMessage_setNote(unsigned char *message, int note)
{
	message[1] = note;
}

int MidiUtilNoteOffMessage_getVelocity(const unsigned char *message)
{
	return message[2];
}

void MidiUtilNoteOffMessage_setVelocity(unsigned char *message, int velocity)
{
	message[2] = velocity;
}

int MidiUtilNoteOnMessage_getChannel(const unsigned char *message)
{
	return (message[0] & 0x0F);
}

void MidiUtilNoteOnMessage_setChannel(unsigned char *message, int channel)
{
	message[0] = (message[0] & ~0x0F) | channel;
}

int MidiUtilNoteOnMessage_getNote(const unsigned char *message)
{
	return message[1];
}

void MidiUtilNoteOnMessage_setNote(unsigned char *message, int note)
{
	message[1] = note;
}

int MidiUtilNoteOnMessage_getVelocity(const unsigned char *message)
{
	return message[2];
}

void MidiUtilNoteOnMessage_setVelocity(unsigned char *message, int velocity)
{
	message[2] = velocity;
}

int MidiUtilKeyPressureMessage_getChannel(const unsigned char *message)
{
	return (message[0] & 0x0F);
}

void MidiUtilKeyPressureMessage_setChannel(unsigned char *message, int channel)
{
	message[0] = (message[0] & ~0x0F) | channel;
}

int MidiUtilKeyPressureMessage_getNote(const unsigned char *message)
{
	return message[1];
}

void MidiUtilKeyPressureMessage_setNote(unsigned char *message, int note)
{
	message[1] = note;
}

int MidiUtilKeyPressureMessage_getAmount(const unsigned char *message)
{
	return message[2];
}

void MidiUtilKeyPressureMessage_setAmount(unsigned char *message, int amount)
{
	message[2] = amount;
}

int MidiUtilControlChangeMessage_getChannel(const unsigned char *message)
{
	return (message[0] & 0x0F);
}

void MidiUtilControlChangeMessage_setChannel(unsigned char *message, int channel)
{
	message[0] = (message[0] & ~0x0F) | channel;
}

int MidiUtilControlChangeMessage_getNumber(const unsigned char *message)
{
	return message[1];
}

void MidiUtilControlChangeMessage_setNumber(unsigned char *message, int number)
{
	message[1] = number;
}

int MidiUtilControlChangeMessage_getValue(const unsigned char *message)
{
	return message[2];
}

void MidiUtilControlChangeMessage_setValue(unsigned char *message, int value)
{
	message[2] = value;
}

int MidiUtilProgramChangeMessage_getChannel(const unsigned char *message)
{
	return (message[0] & 0x0F);
}

void MidiUtilProgramChangeMessage_setChannel(unsigned char *message, int channel)
{
	message[0] = (message[0] & ~0x0F) | channel;
}

int MidiUtilProgramChangeMessage_getNumber(const unsigned char *message)
{
	return message[1];
}

void MidiUtilProgramChangeMessage_setNumber(unsigned char *message, int number)
{
	message[1] = number;
}

int MidiUtilChannelPressureMessage_getChannel(const unsigned char *message)
{
	return (message[0] & 0x0F);
}

void MidiUtilChannelPressureMessage_setChannel(unsigned char *message, int channel)
{
	message[0] = (message[0] & ~0x0F) | channel;
}

int MidiUtilChannelPressureMessage_getAmount(const unsigned char *message)
{
	return message[1];
}

void MidiUtilChannelPressureMessage_setAmount(unsigned char *message, int amount)
{
	message[1] = amount;
}

int MidiUtilPitchWheelMessage_getChannel(const unsigned char *message)
{
	return (message[0] & 0x0F);
}

void MidiUtilPitchWheelMessage_setChannel(unsigned char *message, int channel)
{
	message[0] = (message[0] & ~0x0F) | channel;
}

int MidiUtilPitchWheelMessage_getValue(const unsigned char *message)
{
	return (int)(message[2]) << 7 | (int)(message[1]);
}

void MidiUtilPitchWheelMessage_setValue(unsigned char *message, int value)
{
	message[1] = value & 0x7F;
	message[2] = (value >> 7) & 0x7F;
}

int MidiUtil_getNoteNumberFromName(char *note_name)
{
	const char *note_names[] = {"C#", "C", "Db", "D#", "D", "Eb", "E", "F#", "F", "Gb", "G#", "G", "Ab", "A#", "A", "Bb", "B"};
	const int chromatics[] = {1, 0, 1, 3, 2, 3, 4, 6, 5, 6, 8, 7, 8, 10, 9, 10, 11};
	int chromatic = -1;
	int octave = -1;
	int length;
	int note_number;
	int i;

	for (i = 0; i < (sizeof (note_names) / sizeof (char *)); i++)
	{
		length = strlen(note_names[i]);

		if (strncmp(note_names[i], note_name, length) == 0)
		{
			chromatic = chromatics[i];
			break;
		}
	}

	if (chromatic >= 0)
	{
		if (sscanf(note_name + length, "%d", &octave) == 1)
		{
			note_number = ((octave + 1) * 12) + chromatic;
			if ((note_number >= 0) && (note_number < 128)) return note_number;
		}
	}

	if (sscanf(note_name, "%d", &note_number) == 1)
	{
		if ((note_number >= 0) && (note_number < 128)) return note_number;
	}

	return -1;
}

int MidiUtil_setNoteNameFromNumber(int note_number, char *note_name)
{
	const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

	if ((note_number >= 0) && (note_number < 128))
	{
		sprintf(note_name, "%s%d", note_names[note_number % 12], (note_number / 12) - 1);
		return 0;
	}

	return -1;
}

