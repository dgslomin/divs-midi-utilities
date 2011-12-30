
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple-collections.h"

typedef struct IntIntMapNode *IntIntMapNode_t;
typedef struct IntPointerMapNode *IntPointerMapNode_t;
typedef struct LongIntMapNode *LongIntMapNode_t;
typedef struct LongPointerMapNode *LongPointerMapNode_t;
typedef struct PointerIntMapNode *PointerIntMapNode_t;
typedef struct PointerPointerMapNode *PointerPointerMapNode_t;
typedef struct BlobIntMapNode *BlobIntMapNode_t;
typedef struct BlobPointerMapNode *BlobPointerMapNode_t;
typedef struct StringIntMapNode *StringIntMapNode_t;
typedef struct StringPointerMapNode *StringPointerMapNode_t;

struct ByteArray
{
	int capacity;
	int size;
	unsigned char *buffer;
};

struct IntArray
{
	ByteArray_t byte_array;
};

struct LongArray
{
	ByteArray_t byte_array;
};

struct FloatArray
{
	ByteArray_t byte_array;
};

struct DoubleArray
{
	ByteArray_t byte_array;
};

struct PointerArray
{
	ByteArray_t byte_array;
};

struct String
{
	ByteArray_t byte_array;
};

struct BlobArray
{
	IntArray_t offset_array;
	IntArray_t size_array;
	ByteArray_t data;
};

struct StringArray
{
	BlobArray_t blob_array;
};

struct IntIntMap
{
	int number_of_buckets;
	IntIntMapNode_t *bucket_node_lists;
};

struct IntIntMapNode
{
	int key;
	int value;
	IntIntMapNode_t next;
};

struct IntPointerMap
{
	int number_of_buckets;
	IntPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct IntPointerMapNode
{
	int key;
	void *value;
	IntPointerMapNode_t next;
};

struct LongIntMap
{
	int number_of_buckets;
	LongIntMapNode_t *bucket_node_lists;
};

struct LongIntMapNode
{
	int key;
	int value;
	LongIntMapNode_t next;
};

struct LongPointerMap
{
	int number_of_buckets;
	LongPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct LongPointerMapNode
{
	int key;
	void *value;
	LongPointerMapNode_t next;
};

struct PointerIntMap
{
	int number_of_buckets;
	PointerIntMapNode_t *bucket_node_lists;
};

struct PointerIntMapNode
{
	void *key;
	int value;
	PointerIntMapNode_t next;
};

struct PointerPointerMap
{
	int number_of_buckets;
	PointerPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct PointerPointerMapNode
{
	void *key;
	void *value;
	PointerPointerMapNode_t next;
};

struct BlobIntMap
{
	int number_of_buckets;
	BlobIntMapNode_t *bucket_node_lists;
};

struct BlobIntMapNode
{
	unsigned char *key;
	int key_size;
	int value;
	BlobIntMapNode_t next;
};

struct BlobPointerMap
{
	int number_of_buckets;
	BlobPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct BlobPointerMapNode
{
	unsigned char *key;
	int key_size;
	void *value;
	BlobPointerMapNode_t next;
};

struct StringIntMap
{
	int number_of_buckets;
	StringIntMapNode_t *bucket_node_lists;
};

struct StringIntMapNode
{
	unsigned char *key;
	int value;
	StringIntMapNode_t next;
};

struct StringPointerMap
{
	int number_of_buckets;
	StringPointerMapNode_t *bucket_node_lists;
	void (*free_callback)(void *value, void *user_data);
	void *free_callback_user_data;
};

struct StringPointerMapNode
{
	unsigned char *key;
	void *value;
	StringPointerMapNode_t next;
};

ByteArray_t ByteArray_new(int initial_capacity)
{
	ByteArray_t array = (ByteArray_t)(malloc(sizeof (struct ByteArray)));
	array->capacity = initial_capacity;
	array->size = 0;
	array->buffer = (unsigned char *)(malloc(initial_capacity));
	memset(array->buffer, 0, initial_capacity);
	return array;
}

void ByteArray_free(ByteArray_t array)
{
	free(array->buffer);
	free(array);
}

void ByteArray_clear(ByteArray_t array)
{
	ByteArray_setSize(array, 0);
}

int ByteArray_getCapacity(ByteArray_t array)
{
	return array->capacity;
}

void ByteArray_setCapacity(ByteArray_t array, int capacity)
{
	if (array->size > capacity) array->size = capacity;
	array->capacity = capacity;
	array->buffer = realloc(array->buffer, capacity);
	memset(array->buffer + array->size, 0, capacity - array->size);
}

int ByteArray_getSize(ByteArray_t array)
{
	return array->size;
}

void ByteArray_setSize(ByteArray_t array, int size)
{
	if (size < array->size)
	{
		memset(array->buffer + size, 0, array->size - size);
	}
	else if (size > array->capacity)
	{
		/* TODO: consider growth algorithm from <http://www.python.org/~jeremy/weblog/040115.html> */
		ByteArray_setCapacity(array, size - (size % 4096) + 4096);
	}

	array->size = size;
}

unsigned char *ByteArray_getBuffer(ByteArray_t array)
{
	return array->buffer;
}

unsigned char ByteArray_get(ByteArray_t array, int element_number)
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

void ByteArray_set(ByteArray_t array, int element_number, unsigned char value)
{
	if (element_number >= array->size) ByteArray_setSize(array, element_number + 1);
	array->buffer[element_number] = value;
}

void ByteArray_add(ByteArray_t array, unsigned char value)
{
	ByteArray_set(array, array->size, value);
}

void ByteArray_insert(ByteArray_t array, int element_number, unsigned char value)
{
	ByteArray_setSize(array, array->size + 1);
	memmove(array->buffer + element_number + 1, array->buffer + element_number, array->size - element_number + 1);
	array->buffer[element_number] = value;
}

void ByteArray_remove(ByteArray_t array, int element_number)
{
	memmove(array->buffer + element_number, array->buffer + element_number + 1, array->size - element_number - 1);
	ByteArray_setSize(array, array->size - 1);
}

void ByteArray_setValues(ByteArray_t array, unsigned char *values, int number_of_values)
{
	ByteArray_setSize(array, number_of_values);
	memcpy(array->buffer, values, number_of_values);
}

void ByteArray_addValues(ByteArray_t array, unsigned char *values, int number_of_values)
{
	int offset = array->size;
	ByteArray_setSize(array, offset + number_of_values);
	memcpy(array->buffer + offset, values, number_of_values);
}

void ByteArray_replaceValues(ByteArray_t array, int element_number, unsigned char *values, int number_of_values)
{
	if (element_number + number_of_values > array->size) ByteArray_setSize(array, element_number + number_of_values);
	memcpy(array->buffer + element_number, values, number_of_values);
}

void ByteArray_insertValues(ByteArray_t array, int element_number, unsigned char *values, int number_of_values)
{
	ByteArray_setSize(array, array->size + number_of_values);
	memmove(array->buffer + element_number + number_of_values, array->buffer + element_number, array->size - element_number + number_of_values);
	memcpy(array->buffer + element_number, values, number_of_values);
}

void ByteArray_removeValues(ByteArray_t array, int element_number, int number_of_values)
{
	memmove(array->buffer + element_number, array->buffer + element_number + number_of_values, array->size - element_number - number_of_values);
	ByteArray_setSize(array, array->size - number_of_values);
}

IntArray_t IntArray_new(int initial_capacity)
{
	IntArray_t array = (IntArray_t)(malloc(sizeof (struct IntArray)));
	array->byte_array = ByteArray_new(initial_capacity * sizeof (int));
	return array;
}

void IntArray_free(IntArray_t array)
{
	ByteArray_free(array->byte_array);
	free(array);
}

void IntArray_clear(IntArray_t array)
{
	ByteArray_clear(array->byte_array);
}

int IntArray_getCapacity(IntArray_t array)
{
	return ByteArray_getCapacity(array->byte_array) / sizeof (int);
}

void IntArray_setCapacity(IntArray_t array, int capacity)
{
	ByteArray_setCapacity(array->byte_array, capacity * sizeof (int));
}

int IntArray_getSize(IntArray_t array)
{
	return ByteArray_getSize(array->byte_array) / sizeof (int);
}

void IntArray_setSize(IntArray_t array, int size)
{
	ByteArray_setSize(array->byte_array, size * sizeof (int));
}

int *IntArray_getBuffer(IntArray_t array)
{
	return (int *)(ByteArray_getBuffer(array->byte_array));
}

int IntArray_get(IntArray_t array, int element_number)
{
	return IntArray_getBuffer(array)[element_number];
}

void IntArray_set(IntArray_t array, int element_number, int value)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (int), (unsigned char *)(&value), sizeof (int));
}

void IntArray_add(IntArray_t array, int value)
{
	IntArray_set(array, IntArray_getSize(array), value);
}

void IntArray_insert(IntArray_t array, int element_number, int value)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (int), (unsigned char *)(&value), sizeof (int));
}

void IntArray_remove(IntArray_t array, int element_number)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (int), sizeof (int));
}

void IntArray_setValues(IntArray_t array, int *values, int number_of_values)
{
	ByteArray_setValues(array->byte_array, (char *)(values), number_of_values * sizeof (int));
}

void IntArray_addValues(IntArray_t array, int *values, int number_of_values)
{
	ByteArray_addValues(array->byte_array, (char *)(values), number_of_values * sizeof (int));
}

void IntArray_replaceValues(IntArray_t array, int element_number, int *values, int number_of_values)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (int), (char *)(values), number_of_values * sizeof (int));
}

void IntArray_insertValues(IntArray_t array, int element_number, int *values, int number_of_values)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (int), (char *)(values), number_of_values * sizeof (int));
}

void IntArray_removeValues(IntArray_t array, int element_number, int number_of_values)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (int), number_of_values * sizeof (int));
}

LongArray_t LongArray_new(int initial_capacity)
{
	LongArray_t array = (LongArray_t)(malloc(sizeof (struct LongArray)));
	array->byte_array = ByteArray_new(initial_capacity * sizeof (long));
	return array;
}

void LongArray_free(LongArray_t array)
{
	ByteArray_free(array->byte_array);
	free(array);
}

void LongArray_clear(LongArray_t array)
{
	ByteArray_clear(array->byte_array);
}

int LongArray_getCapacity(LongArray_t array)
{
	return ByteArray_getCapacity(array->byte_array) / sizeof (long);
}

void LongArray_setCapacity(LongArray_t array, int capacity)
{
	ByteArray_setCapacity(array->byte_array, capacity * sizeof (long));
}

int LongArray_getSize(LongArray_t array)
{
	return ByteArray_getSize(array->byte_array) / sizeof (long);
}

void LongArray_setSize(LongArray_t array, int size)
{
	ByteArray_setSize(array->byte_array, size * sizeof (long));
}

long *LongArray_getBuffer(LongArray_t array)
{
	return (long *)(ByteArray_getBuffer(array->byte_array));
}

long LongArray_get(LongArray_t array, int element_number)
{
	return LongArray_getBuffer(array)[element_number];
}

void LongArray_set(LongArray_t array, int element_number, long value)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (long), (unsigned char *)(&value), sizeof (long));
}

void LongArray_add(LongArray_t array, long value)
{
	LongArray_set(array, LongArray_getSize(array), value);
}

void LongArray_insert(LongArray_t array, int element_number, long value)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (long), (unsigned char *)(&value), sizeof (long));
}

void LongArray_remove(LongArray_t array, int element_number)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (long), sizeof (long));
}

void LongArray_setValues(LongArray_t array, long *values, int number_of_values)
{
	ByteArray_setValues(array->byte_array, (char *)(values), number_of_values * sizeof (long));
}

void LongArray_addValues(LongArray_t array, long *values, int number_of_values)
{
	ByteArray_addValues(array->byte_array, (char *)(values), number_of_values * sizeof (long));
}

void LongArray_replaceValues(LongArray_t array, int element_number, long *values, int number_of_values)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (long), (char *)(values), number_of_values * sizeof (long));
}

void LongArray_insertValues(LongArray_t array, int element_number, long *values, int number_of_values)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (long), (char *)(values), number_of_values * sizeof (long));
}

void LongArray_removeValues(LongArray_t array, int element_number, int number_of_values)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (long), number_of_values * sizeof (long));
}

FloatArray_t FloatArray_new(int initial_capacity)
{
	FloatArray_t array = (FloatArray_t)(malloc(sizeof (struct FloatArray)));
	array->byte_array = ByteArray_new(initial_capacity * sizeof (float));
	return array;
}

void FloatArray_free(FloatArray_t array)
{
	ByteArray_free(array->byte_array);
	free(array);
}

void FloatArray_clear(FloatArray_t array)
{
	ByteArray_clear(array->byte_array);
}

int FloatArray_getCapacity(FloatArray_t array)
{
	return ByteArray_getCapacity(array->byte_array) / sizeof (float);
}

void FloatArray_setCapacity(FloatArray_t array, int capacity)
{
	ByteArray_setCapacity(array->byte_array, capacity * sizeof (float));
}

int FloatArray_getSize(FloatArray_t array)
{
	return ByteArray_getSize(array->byte_array) / sizeof (float);
}

void FloatArray_setSize(FloatArray_t array, int size)
{
	ByteArray_setSize(array->byte_array, size * sizeof (float));
}

float *FloatArray_getBuffer(FloatArray_t array)
{
	return (float *)(ByteArray_getBuffer(array->byte_array));
}

float FloatArray_get(FloatArray_t array, int element_number)
{
	return FloatArray_getBuffer(array)[element_number];
}

void FloatArray_set(FloatArray_t array, int element_number, float value)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (float), (unsigned char *)(&value), sizeof (float));
}

void FloatArray_add(FloatArray_t array, float value)
{
	FloatArray_set(array, FloatArray_getSize(array), value);
}

void FloatArray_insert(FloatArray_t array, int element_number, float value)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (float), (unsigned char *)(&value), sizeof (float));
}

void FloatArray_remove(FloatArray_t array, int element_number)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (float), sizeof (float));
}

void FloatArray_setValues(FloatArray_t array, float *values, int number_of_values)
{
	ByteArray_setValues(array->byte_array, (char *)(values), number_of_values * sizeof (float));
}

void FloatArray_addValues(FloatArray_t array, float *values, int number_of_values)
{
	ByteArray_addValues(array->byte_array, (char *)(values), number_of_values * sizeof (float));
}

void FloatArray_replaceValues(FloatArray_t array, int element_number, float *values, int number_of_values)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (float), (char *)(values), number_of_values * sizeof (float));
}

void FloatArray_insertValues(FloatArray_t array, int element_number, float *values, int number_of_values)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (float), (char *)(values), number_of_values * sizeof (float));
}

void FloatArray_removeValues(FloatArray_t array, int element_number, int number_of_values)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (float), number_of_values * sizeof (float));
}

DoubleArray_t DoubleArray_new(int initial_capacity)
{
	DoubleArray_t array = (DoubleArray_t)(malloc(sizeof (struct DoubleArray)));
	array->byte_array = ByteArray_new(initial_capacity * sizeof (double));
	return array;
}

void DoubleArray_free(DoubleArray_t array)
{
	ByteArray_free(array->byte_array);
	free(array);
}

void DoubleArray_clear(DoubleArray_t array)
{
	ByteArray_clear(array->byte_array);
}

int DoubleArray_getCapacity(DoubleArray_t array)
{
	return ByteArray_getCapacity(array->byte_array) / sizeof (double);
}

void DoubleArray_setCapacity(DoubleArray_t array, int capacity)
{
	ByteArray_setCapacity(array->byte_array, capacity * sizeof (double));
}

int DoubleArray_getSize(DoubleArray_t array)
{
	return ByteArray_getSize(array->byte_array) / sizeof (double);
}

void DoubleArray_setSize(DoubleArray_t array, int size)
{
	ByteArray_setSize(array->byte_array, size * sizeof (double));
}

double *DoubleArray_getBuffer(DoubleArray_t array)
{
	return (double *)(ByteArray_getBuffer(array->byte_array));
}

double DoubleArray_get(DoubleArray_t array, int element_number)
{
	return DoubleArray_getBuffer(array)[element_number];
}

void DoubleArray_set(DoubleArray_t array, int element_number, double value)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (double), (unsigned char *)(&value), sizeof (double));
}

void DoubleArray_add(DoubleArray_t array, double value)
{
	DoubleArray_set(array, DoubleArray_getSize(array), value);
}

void DoubleArray_insert(DoubleArray_t array, int element_number, double value)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (double), (unsigned char *)(&value), sizeof (double));
}

void DoubleArray_remove(DoubleArray_t array, int element_number)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (double), sizeof (double));
}

void DoubleArray_setValues(DoubleArray_t array, double *values, int number_of_values)
{
	ByteArray_setValues(array->byte_array, (char *)(values), number_of_values * sizeof (double));
}

void DoubleArray_addValues(DoubleArray_t array, double *values, int number_of_values)
{
	ByteArray_addValues(array->byte_array, (char *)(values), number_of_values * sizeof (double));
}

void DoubleArray_replaceValues(DoubleArray_t array, int element_number, double *values, int number_of_values)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (double), (char *)(values), number_of_values * sizeof (double));
}

void DoubleArray_insertValues(DoubleArray_t array, int element_number, double *values, int number_of_values)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (double), (char *)(values), number_of_values * sizeof (double));
}

void DoubleArray_removeValues(DoubleArray_t array, int element_number, int number_of_values)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (double), number_of_values * sizeof (double));
}

PointerArray_t PointerArray_new(int initial_capacity)
{
	PointerArray_t array = (PointerArray_t)(malloc(sizeof (struct PointerArray)));
	array->byte_array = ByteArray_new(initial_capacity * sizeof (void *));
	return array;
}

void PointerArray_free(PointerArray_t array)
{
	ByteArray_free(array->byte_array);
	free(array);
}

void PointerArray_clear(PointerArray_t array)
{
	ByteArray_clear(array->byte_array);
}

int PointerArray_getCapacity(PointerArray_t array)
{
	return ByteArray_getCapacity(array->byte_array) / sizeof (void *);
}

void PointerArray_setCapacity(PointerArray_t array, int capacity)
{
	ByteArray_setCapacity(array->byte_array, capacity * sizeof (void *));
}

int PointerArray_getSize(PointerArray_t array)
{
	return ByteArray_getSize(array->byte_array) / sizeof (void *);
}

void PointerArray_setSize(PointerArray_t array, int size)
{
	ByteArray_setSize(array->byte_array, size * sizeof (void *));
}

void **PointerArray_getBuffer(PointerArray_t array)
{
	return (void **)(ByteArray_getBuffer(array->byte_array));
}

void *PointerArray_get(PointerArray_t array, int element_number)
{
	return PointerArray_getBuffer(array)[element_number];
}

void PointerArray_set(PointerArray_t array, int element_number, void *value)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (void *), (unsigned char *)(&value), sizeof (void *));
}

void PointerArray_add(PointerArray_t array, void *value)
{
	PointerArray_set(array, PointerArray_getSize(array), value);
}

void PointerArray_insert(PointerArray_t array, int element_number, void *value)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (void *), (unsigned char *)(&value), sizeof (void *));
}

void PointerArray_remove(PointerArray_t array, int element_number)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (void *), sizeof (void *));
}

void PointerArray_setValues(PointerArray_t array, void **values, int number_of_values)
{
	ByteArray_setValues(array->byte_array, (char *)(values), number_of_values * sizeof (void *));
}

void PointerArray_addValues(PointerArray_t array, void **values, int number_of_values)
{
	ByteArray_addValues(array->byte_array, (char *)(values), number_of_values * sizeof (void *));
}

void PointerArray_replaceValues(PointerArray_t array, int element_number, void **values, int number_of_values)
{
	ByteArray_replaceValues(array->byte_array, element_number * sizeof (void *), (char *)(values), number_of_values * sizeof (void *));
}

void PointerArray_insertValues(PointerArray_t array, int element_number, void **values, int number_of_values)
{
	ByteArray_insertValues(array->byte_array, element_number * sizeof (void *), (char *)(values), number_of_values * sizeof (void *));
}

void PointerArray_removeValues(PointerArray_t array, int element_number, int number_of_values)
{
	ByteArray_removeValues(array->byte_array, element_number * sizeof (void *), number_of_values * sizeof (void *));
}

String_t String_new(int initial_capacity)
{
	String_t string = (String_t)(malloc(sizeof (struct String)));
	string->byte_array = ByteArray_new(initial_capacity);
	ByteArray_set(string->byte_array, 0, '\0');
	return string;
}

void String_free(String_t string)
{
	ByteArray_free(string->byte_array);
	free(string);
}

void String_clear(String_t string)
{
	ByteArray_clear(string->byte_array);
	ByteArray_set(string->byte_array, 0, '\0');
}

int String_getCapacity(String_t string)
{
	return ByteArray_getCapacity(string->byte_array);
}

void String_setCapacity(String_t string, int capacity)
{
	ByteArray_setCapacity(string->byte_array, capacity);
}

int String_getSize(String_t string)
{
	return ByteArray_getSize(string->byte_array) - 1;
}

unsigned char *String_get(String_t string)
{
	return ByteArray_getBuffer(string->byte_array);
}

void String_set(String_t string, unsigned char *value)
{
	ByteArray_setValues(string->byte_array, value, strlen((char *)(value)) + 1);
}

void String_addChar(String_t string, unsigned char value)
{
	int size = String_getSize(string);
	ByteArray_set(string->byte_array, size, value);
	ByteArray_set(string->byte_array, size + 1, '\0');
}

void String_addString(String_t string, unsigned char *value)
{
	ByteArray_replaceValues(string->byte_array, String_getSize(string), value, strlen((char *)(value)) + 1);
}

BlobArray_t BlobArray_new(int initial_element_capacity, int initial_byte_capacity)
{
	BlobArray_t array = (BlobArray_t)(malloc(sizeof (struct BlobArray)));
	array->offset_array = IntArray_new(initial_element_capacity);
	array->size_array = IntArray_new(initial_element_capacity);
	array->data = ByteArray_new(initial_byte_capacity);
	return array;
}

void BlobArray_free(BlobArray_t array)
{
	ByteArray_free(array->data);
	IntArray_free(array->size_array);
	IntArray_free(array->offset_array);
	free(array);
}

void BlobArray_clear(BlobArray_t array)
{
	IntArray_clear(array->offset_array);
	IntArray_clear(array->size_array);
	ByteArray_clear(array->data);
}

int BlobArray_getSize(BlobArray_t array)
{
	return IntArray_getSize(array->offset_array);
}

unsigned char *BlobArray_getValue(BlobArray_t array, int element_number)
{
	return ByteArray_getBuffer(array->data) + IntArray_get(array->offset_array, element_number);
}

int BlobArray_getValueSize(BlobArray_t array, int element_number)
{
	return IntArray_get(array->size_array, element_number);
}

void BlobArray_set(BlobArray_t array, int element_number, unsigned char *value, int value_size)
{
	int size = BlobArray_getSize(array);
	int offset;

	if ((element_number < size) && (value_size <= IntArray_get(array->size_array, element_number)))
	{
		offset = IntArray_get(array->offset_array, element_number);
	}
	else
	{
		if (size == 0)
		{
			offset = 0;
		}
		else
		{
			offset = IntArray_get(array->offset_array, size - 1) + IntArray_get(array->size_array, size - 1);
		}

		IntArray_set(array->offset_array, element_number, offset);
		ByteArray_setSize(array->data, offset + value_size);
	}

	IntArray_set(array->size_array, element_number, value_size);
	if (value_size > 0) memcpy(ByteArray_getBuffer(array->data) + offset, value, value_size);
}

void BlobArray_add(BlobArray_t array, unsigned char *value, int value_size)
{
	BlobArray_set(array, BlobArray_getSize(array), value, value_size);
}

void BlobArray_insert(BlobArray_t array, int element_number, unsigned char *value, int value_size)
{
	int size = BlobArray_getSize(array);
	int offset = IntArray_get(array->offset_array, size - 1) + IntArray_get(array->size_array, size - 1);

	if (value_size > 0)
	{
		ByteArray_setSize(array->data, offset + value_size);
		memcpy(ByteArray_getBuffer(array->data) + offset, value, value_size);
	}

	IntArray_insert(array->offset_array, element_number, offset);
	IntArray_insert(array->size_array, element_number, value_size);
}

void BlobArray_remove(BlobArray_t array, int element_number)
{
	BlobArray_set(array, element_number, NULL, 0);
}

StringArray_t StringArray_new(int initial_element_capacity, int initial_byte_capacity)
{
	StringArray_t array = (StringArray_t)(malloc(sizeof (struct StringArray)));
	array->blob_array = BlobArray_new(initial_element_capacity, initial_byte_capacity);
	return array;
}

void StringArray_free(StringArray_t array)
{
	BlobArray_free(array->blob_array);
	free(array);
}

void StringArray_clear(StringArray_t array)
{
	BlobArray_clear(array->blob_array);
}

int StringArray_getSize(StringArray_t array)
{
	return BlobArray_getSize(array->blob_array);
}

unsigned char *StringArray_get(StringArray_t array, int element_number)
{
	return BlobArray_getValue(array->blob_array, element_number);
}

void StringArray_set(StringArray_t array, int element_number, unsigned char *value)
{
	BlobArray_set(array->blob_array, element_number, value, strlen((char *)(value)) + 1);
}

void StringArray_add(StringArray_t array, unsigned char *value)
{
	BlobArray_add(array->blob_array, value, strlen((char *)(value)) + 1);
}

void StringArray_insert(StringArray_t array, int element_number, unsigned char *value)
{
	BlobArray_insert(array->blob_array, element_number, value, strlen((char *)(value)) + 1);
}

void StringArray_remove(StringArray_t array, int element_number)
{
	BlobArray_remove(array->blob_array, element_number);
}

static int _hashInt(int key, int number_of_buckets)
{
	return (key * 62342347) % number_of_buckets;
}

IntIntMap_t IntIntMap_new(int number_of_buckets)
{
	IntIntMap_t map = (IntIntMap_t)(malloc(sizeof (struct IntIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (IntIntMapNode_t *)(malloc(sizeof (IntIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void IntIntMap_free(IntIntMap_t map)
{
	IntIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void IntIntMap_clear(IntIntMap_t map)
{
	int bucket_number;
	IntIntMapNode_t node;
	IntIntMapNode_t next_node = NULL;

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

int IntIntMap_hasKey(IntIntMap_t map, int key)
{
	IntIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

int IntIntMap_get(IntIntMap_t map, int key, int default_value)
{
	IntIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return default_value;
}

void IntIntMap_set(IntIntMap_t map, int key, int value)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	IntIntMapNode_t node;
	IntIntMapNode_t previous_node = NULL;

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

	node = (IntIntMapNode_t)(malloc(sizeof (struct IntIntMapNode)));
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

void IntIntMap_remove(IntIntMap_t map, int key)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	IntIntMapNode_t node;
	IntIntMapNode_t previous_node = NULL;

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

void IntIntMap_enumerate(IntIntMap_t map, int (*callback)(int key, int value, void *user_data), void *user_data)
{
	int bucket_number;
	IntIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

IntPointerMap_t IntPointerMap_new(int number_of_buckets)
{
	IntPointerMap_t map = (IntPointerMap_t)(malloc(sizeof (struct IntPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (IntPointerMapNode_t *)(malloc(sizeof (IntPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void IntPointerMap_setFreeCallback(IntPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void IntPointerMap_free(IntPointerMap_t map)
{
	IntPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void IntPointerMap_clear(IntPointerMap_t map)
{
	int bucket_number;
	IntPointerMapNode_t node;
	IntPointerMapNode_t next_node = NULL;

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

int IntPointerMap_hasKey(IntPointerMap_t map, int key)
{
	IntPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

void *IntPointerMap_get(IntPointerMap_t map, int key)
{
	IntPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return NULL;
}

void IntPointerMap_set(IntPointerMap_t map, int key, void *value)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	IntPointerMapNode_t node;
	IntPointerMapNode_t previous_node = NULL;

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

	node = (IntPointerMapNode_t)(malloc(sizeof (struct IntPointerMapNode)));
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

void IntPointerMap_remove(IntPointerMap_t map, int key)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	IntPointerMapNode_t node;
	IntPointerMapNode_t previous_node = NULL;

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

void IntPointerMap_enumerate(IntPointerMap_t map, int (*callback)(int key, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	IntPointerMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

LongIntMap_t LongIntMap_new(int number_of_buckets)
{
	LongIntMap_t map = (LongIntMap_t)(malloc(sizeof (struct LongIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (LongIntMapNode_t *)(malloc(sizeof (LongIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void LongIntMap_free(LongIntMap_t map)
{
	LongIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void LongIntMap_clear(LongIntMap_t map)
{
	int bucket_number;
	LongIntMapNode_t node;
	LongIntMapNode_t next_node = NULL;

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

int LongIntMap_hasKey(LongIntMap_t map, long key)
{
	LongIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

int LongIntMap_get(LongIntMap_t map, long key, int default_value)
{
	LongIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return default_value;
}

void LongIntMap_set(LongIntMap_t map, long key, int value)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	LongIntMapNode_t node;
	LongIntMapNode_t previous_node = NULL;

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

	node = (LongIntMapNode_t)(malloc(sizeof (struct LongIntMapNode)));
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

void LongIntMap_remove(LongIntMap_t map, long key)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	LongIntMapNode_t node;
	LongIntMapNode_t previous_node = NULL;

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

void LongIntMap_enumerate(LongIntMap_t map, int (*callback)(long key, int value, void *user_data), void *user_data)
{
	int bucket_number;
	LongIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

LongPointerMap_t LongPointerMap_new(int number_of_buckets)
{
	LongPointerMap_t map = (LongPointerMap_t)(malloc(sizeof (struct LongPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (LongPointerMapNode_t *)(malloc(sizeof (LongPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void LongPointerMap_setFreeCallback(LongPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void LongPointerMap_free(LongPointerMap_t map)
{
	LongPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void LongPointerMap_clear(LongPointerMap_t map)
{
	int bucket_number;
	LongPointerMapNode_t node;
	LongPointerMapNode_t next_node = NULL;

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

int LongPointerMap_hasKey(LongPointerMap_t map, long key)
{
	LongPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

void *LongPointerMap_get(LongPointerMap_t map, long key)
{
	LongPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashInt(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return NULL;
}

void LongPointerMap_set(LongPointerMap_t map, long key, void *value)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	LongPointerMapNode_t node;
	LongPointerMapNode_t previous_node = NULL;

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

	node = (LongPointerMapNode_t)(malloc(sizeof (struct LongPointerMapNode)));
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

void LongPointerMap_remove(LongPointerMap_t map, long key)
{
	int bucket_number = _hashInt(key, map->number_of_buckets);
	LongPointerMapNode_t node;
	LongPointerMapNode_t previous_node = NULL;

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

void LongPointerMap_enumerate(LongPointerMap_t map, int (*callback)(long key, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	LongPointerMapNode_t node;

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

PointerIntMap_t PointerIntMap_new(int number_of_buckets)
{
	PointerIntMap_t map = (PointerIntMap_t)(malloc(sizeof (struct PointerIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (PointerIntMapNode_t *)(malloc(sizeof (PointerIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void PointerIntMap_free(PointerIntMap_t map)
{
	PointerIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void PointerIntMap_clear(PointerIntMap_t map)
{
	int bucket_number;
	PointerIntMapNode_t node;
	PointerIntMapNode_t next_node = NULL;

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

int PointerIntMap_hasKey(PointerIntMap_t map, void *key)
{
	PointerIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashPointer(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

int PointerIntMap_get(PointerIntMap_t map, void *key, int default_value)
{
	PointerIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashPointer(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return default_value;
}

void PointerIntMap_set(PointerIntMap_t map, void *key, int value)
{
	int bucket_number = _hashPointer(key, map->number_of_buckets);
	PointerIntMapNode_t node;
	PointerIntMapNode_t previous_node = NULL;

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

	node = (PointerIntMapNode_t)(malloc(sizeof (struct PointerIntMapNode)));
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

void PointerIntMap_remove(PointerIntMap_t map, void *key)
{
	int bucket_number = _hashPointer(key, map->number_of_buckets);
	PointerIntMapNode_t node;
	PointerIntMapNode_t previous_node = NULL;

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

void PointerIntMap_enumerate(PointerIntMap_t map, int (*callback)(void *key, int value, void *user_data), void *user_data)
{
	int bucket_number;
	PointerIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

PointerPointerMap_t PointerPointerMap_new(int number_of_buckets)
{
	PointerPointerMap_t map = (PointerPointerMap_t)(malloc(sizeof (struct PointerPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (PointerPointerMapNode_t *)(malloc(sizeof (PointerPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void PointerPointerMap_setFreeCallback(PointerPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void PointerPointerMap_free(PointerPointerMap_t map)
{
	PointerPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void PointerPointerMap_clear(PointerPointerMap_t map)
{
	int bucket_number;
	PointerPointerMapNode_t node;
	PointerPointerMapNode_t next_node = NULL;

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

int PointerPointerMap_hasKey(PointerPointerMap_t map, void *key)
{
	PointerPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashPointer(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return 1;
	}

	return 0;
}

void *PointerPointerMap_get(PointerPointerMap_t map, void *key)
{
	PointerPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashPointer(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (node->key == key) return node->value;
	}

	return NULL;
}

void PointerPointerMap_set(PointerPointerMap_t map, void *key, void *value)
{
	int bucket_number = _hashPointer(key, map->number_of_buckets);
	PointerPointerMapNode_t node;
	PointerPointerMapNode_t previous_node = NULL;

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

	node = (PointerPointerMapNode_t)(malloc(sizeof (struct PointerPointerMapNode)));
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

void PointerPointerMap_remove(PointerPointerMap_t map, void *key)
{
	int bucket_number = _hashPointer(key, map->number_of_buckets);
	PointerPointerMapNode_t node;
	PointerPointerMapNode_t previous_node = NULL;

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

void PointerPointerMap_enumerate(PointerPointerMap_t map, int (*callback)(void *key, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	PointerPointerMapNode_t node;

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

BlobIntMap_t BlobIntMap_new(int number_of_buckets)
{
	BlobIntMap_t map = (BlobIntMap_t)(malloc(sizeof (struct BlobIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (BlobIntMapNode_t *)(malloc(sizeof (BlobIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void BlobIntMap_free(BlobIntMap_t map)
{
	BlobIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void BlobIntMap_clear(BlobIntMap_t map)
{
	int bucket_number;
	BlobIntMapNode_t node;
	BlobIntMapNode_t next_node = NULL;

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

int BlobIntMap_hasKey(BlobIntMap_t map, unsigned char *key, int key_size)
{
	BlobIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashBlob(key, key_size, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0)) return 1;
	}

	return 0;
}

int BlobIntMap_get(BlobIntMap_t map, unsigned char *key, int key_size, int default_value)
{
	BlobIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashBlob(key, key_size, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0)) return node->value;
	}

	return default_value;
}

void BlobIntMap_set(BlobIntMap_t map, unsigned char *key, int key_size, int value)
{
	int bucket_number = _hashBlob(key, key_size, map->number_of_buckets);
	BlobIntMapNode_t node;
	BlobIntMapNode_t previous_node = NULL;

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

	node = (BlobIntMapNode_t)(malloc(sizeof (struct BlobIntMapNode)));
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

void BlobIntMap_remove(BlobIntMap_t map, unsigned char *key, int key_size)
{
	int bucket_number = _hashBlob(key, key_size, map->number_of_buckets);
	BlobIntMapNode_t node;
	BlobIntMapNode_t previous_node = NULL;

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

void BlobIntMap_enumerate(BlobIntMap_t map, int (*callback)(unsigned char *key, int key_size, int value, void *user_data), void *user_data)
{
	int bucket_number;
	BlobIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->key_size, node->value, user_data)) break;
		}
	}
}

BlobPointerMap_t BlobPointerMap_new(int number_of_buckets)
{
	BlobPointerMap_t map = (BlobPointerMap_t)(malloc(sizeof (struct BlobPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (BlobPointerMapNode_t *)(malloc(sizeof (BlobPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void BlobPointerMap_setFreeCallback(BlobPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void BlobPointerMap_free(BlobPointerMap_t map)
{
	BlobPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void BlobPointerMap_clear(BlobPointerMap_t map)
{
	int bucket_number;
	BlobPointerMapNode_t node;
	BlobPointerMapNode_t next_node = NULL;

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

int BlobPointerMap_hasKey(BlobPointerMap_t map, unsigned char *key, int key_size)
{
	BlobPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashBlob(key, key_size, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0)) return 1;
	}

	return 0;
}

void *BlobPointerMap_get(BlobPointerMap_t map, unsigned char *key, int key_size)
{
	BlobPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashBlob(key, key_size, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if ((node->key_size == key_size) && (memcmp(node->key, key, key_size) == 0)) return node->value;
	}

	return NULL;
}

void BlobPointerMap_set(BlobPointerMap_t map, unsigned char *key, int key_size, void *value)
{
	int bucket_number = _hashBlob(key, key_size, map->number_of_buckets);
	BlobPointerMapNode_t node;
	BlobPointerMapNode_t previous_node = NULL;

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

	node = (BlobPointerMapNode_t)(malloc(sizeof (struct BlobPointerMapNode)));
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

void BlobPointerMap_remove(BlobPointerMap_t map, unsigned char *key, int key_size)
{
	int bucket_number = _hashBlob(key, key_size, map->number_of_buckets);
	BlobPointerMapNode_t node;
	BlobPointerMapNode_t previous_node = NULL;

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

void BlobPointerMap_enumerate(BlobPointerMap_t map, int (*callback)(unsigned char *key, int key_size, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	BlobPointerMapNode_t node;

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

StringIntMap_t StringIntMap_new(int number_of_buckets)
{
	StringIntMap_t map = (StringIntMap_t)(malloc(sizeof (struct StringIntMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (StringIntMapNode_t *)(malloc(sizeof (StringIntMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	return map;
}

void StringIntMap_free(StringIntMap_t map)
{
	StringIntMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void StringIntMap_clear(StringIntMap_t map)
{
	int bucket_number;
	StringIntMapNode_t node;
	StringIntMapNode_t next_node = NULL;

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

int StringIntMap_hasKey(StringIntMap_t map, unsigned char *key)
{
	StringIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashString(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0) return 1;
	}

	return 0;
}

int StringIntMap_get(StringIntMap_t map, unsigned char *key, int default_value)
{
	StringIntMapNode_t node;

	for (node = map->bucket_node_lists[_hashString(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0) return node->value;
	}

	return default_value;
}

void StringIntMap_set(StringIntMap_t map, unsigned char *key, int value)
{
	int bucket_number = _hashString(key, map->number_of_buckets);
	StringIntMapNode_t node;
	StringIntMapNode_t previous_node = NULL;

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

	node = (StringIntMapNode_t)(malloc(sizeof (struct StringIntMapNode)));
	node->key = strdup(key);
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

void StringIntMap_remove(StringIntMap_t map, unsigned char *key)
{
	int bucket_number = _hashString(key, map->number_of_buckets);
	StringIntMapNode_t node;
	StringIntMapNode_t previous_node = NULL;

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

void StringIntMap_enumerate(StringIntMap_t map, int (*callback)(unsigned char *key, int value, void *user_data), void *user_data)
{
	int bucket_number;
	StringIntMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

StringPointerMap_t StringPointerMap_new(int number_of_buckets)
{
	StringPointerMap_t map = (StringPointerMap_t)(malloc(sizeof (struct StringPointerMap)));
	int bucket_number;
	map->number_of_buckets = number_of_buckets;
	map->bucket_node_lists = (StringPointerMapNode_t *)(malloc(sizeof (StringPointerMapNode_t) * number_of_buckets));
	for (bucket_number = 0; bucket_number < number_of_buckets; bucket_number++) map->bucket_node_lists[bucket_number] = NULL;
	map->free_callback = NULL;
	map->free_callback_user_data = NULL;
	return map;
}

void StringPointerMap_setFreeCallback(StringPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data)
{
	map->free_callback = callback;
	map->free_callback_user_data = user_data;
}

void StringPointerMap_free(StringPointerMap_t map)
{
	StringPointerMap_clear(map);
	free(map->bucket_node_lists);
	free(map);
}

void StringPointerMap_clear(StringPointerMap_t map)
{
	int bucket_number;
	StringPointerMapNode_t node;
	StringPointerMapNode_t next_node = NULL;

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

int StringPointerMap_hasKey(StringPointerMap_t map, unsigned char *key)
{
	StringPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashString(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0) return 1;
	}

	return 0;
}

void *StringPointerMap_get(StringPointerMap_t map, unsigned char *key)
{
	StringPointerMapNode_t node;

	for (node = map->bucket_node_lists[_hashString(key, map->number_of_buckets)]; node != NULL; node = node->next)
	{
		if (strcmp((char *)(node->key), (char *)(key)) == 0) return node->value;
	}

	return NULL;
}

void StringPointerMap_set(StringPointerMap_t map, unsigned char *key, void *value)
{
	int bucket_number = _hashString(key, map->number_of_buckets);
	StringPointerMapNode_t node;
	StringPointerMapNode_t previous_node = NULL;

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

	node = (StringPointerMapNode_t)(malloc(sizeof (struct StringPointerMapNode)));
	node->key = strdup(key);
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

void StringPointerMap_remove(StringPointerMap_t map, unsigned char *key)
{
	int bucket_number = _hashString(key, map->number_of_buckets);
	StringPointerMapNode_t node;
	StringPointerMapNode_t previous_node = NULL;

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

void StringPointerMap_enumerate(StringPointerMap_t map, int (*callback)(unsigned char *key, void *value, void *user_data), void *user_data)
{
	int bucket_number;
	StringPointerMapNode_t node;

	for (bucket_number = 0; bucket_number < map->number_of_buckets; bucket_number++)
	{
		for (node = map->bucket_node_lists[bucket_number]; node != NULL; node = node->next)
		{
			if ((*callback)(node->key, node->value, user_data)) break;
		}
	}
}

static void _quicksort(int begin, int end, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data)
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
		_quicksort(begin, left, compare_callback, exchange_callback, user_data);
		_quicksort(right, end, compare_callback, exchange_callback, user_data);
	}
}

void quicksort(int number_of_elements, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data)
{
	_quicksort(0, number_of_elements, compare_callback, exchange_callback, user_data);
}

static void _heapsortSiftDown(int start, int end, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data)
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

void heapsort(int number_of_elements, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data)
{
	if (number_of_elements > 1)
	{
		int end;
		int start = (number_of_elements - 1) / 2;

		while (1)
		{
			_heapsortSiftDown(start, number_of_elements - 1, compare_callback, exchange_callback, user_data);
			if (start == 0) break;
			start--;
		}

		end = number_of_elements - 1;

		while (end > 0)
		{
			(*exchange_callback)(end, 0, user_data);
			end--;
			_heapsortSiftDown(0, end, compare_callback, exchange_callback, user_data);
		}
	}
}

