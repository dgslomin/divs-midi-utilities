#ifndef SIMPLE_COLLECTIONS_INCLUDED
#define SIMPLE_COLLECTIONS_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ByteArray *ByteArray_t;
typedef struct IntArray *IntArray_t;
typedef struct LongArray *LongArray_t;
typedef struct FloatArray *FloatArray_t;
typedef struct DoubleArray *DoubleArray_t;
typedef struct PointerArray *PointerArray_t;
typedef struct String *String_t;
typedef struct BlobArray *BlobArray_t;
typedef struct StringArray *StringArray_t;
typedef struct IntIntMap *IntIntMap_t;
typedef struct IntPointerMap *IntPointerMap_t;
typedef struct LongIntMap *LongIntMap_t;
typedef struct LongPointerMap *LongPointerMap_t;
typedef struct PointerIntMap *PointerIntMap_t;
typedef struct PointerPointerMap *PointerPointerMap_t;
typedef struct BlobIntMap *BlobIntMap_t;
typedef struct BlobPointerMap *BlobPointerMap_t;
typedef struct StringIntMap *StringIntMap_t;
typedef struct StringPointerMap *StringPointerMap_t;

ByteArray_t ByteArray_new(int initial_capacity);
void ByteArray_free(ByteArray_t array);
void ByteArray_clear(ByteArray_t array);
int ByteArray_getCapacity(ByteArray_t array);
void ByteArray_setCapacity(ByteArray_t array, int capacity);
int ByteArray_getSize(ByteArray_t array);
void ByteArray_setSize(ByteArray_t array, int size);
unsigned char *ByteArray_getBuffer(ByteArray_t array);
unsigned char ByteArray_get(ByteArray_t array, int element_number);
void ByteArray_set(ByteArray_t array, int element_number, unsigned char value);
void ByteArray_add(ByteArray_t array, unsigned char value);
void ByteArray_insert(ByteArray_t array, int element_number, unsigned char value);
void ByteArray_remove(ByteArray_t array, int element_number);
void ByteArray_setValues(ByteArray_t array, unsigned char *values, int number_of_values);
void ByteArray_addValues(ByteArray_t array, unsigned char *values, int number_of_values);
void ByteArray_replaceValues(ByteArray_t array, int element_number, unsigned char *values, int number_of_values);
void ByteArray_insertValues(ByteArray_t array, int element_number, unsigned char *values, int number_of_values);
void ByteArray_removeValues(ByteArray_t array, int element_number, int number_of_values);

IntArray_t IntArray_new(int initial_capacity);
void IntArray_free(IntArray_t array);
void IntArray_clear(IntArray_t array);
int IntArray_getCapacity(IntArray_t array);
void IntArray_setCapacity(IntArray_t array, int capacity);
int IntArray_getSize(IntArray_t array);
void IntArray_setSize(IntArray_t array, int size);
int *IntArray_getBuffer(IntArray_t array);
int IntArray_get(IntArray_t array, int element_number);
void IntArray_set(IntArray_t array, int element_number, int value);
void IntArray_add(IntArray_t array, int value);
void IntArray_insert(IntArray_t array, int element_number, int value);
void IntArray_remove(IntArray_t array, int element_number);
void IntArray_setValues(IntArray_t array, int *values, int number_of_values);
void IntArray_addValues(IntArray_t array, int *values, int number_of_values);
void IntArray_replaceValues(IntArray_t array, int element_number, int *values, int number_of_values);
void IntArray_insertValues(IntArray_t array, int element_number, int *values, int number_of_values);
void IntArray_removeValues(IntArray_t array, int element_number, int number_of_values);

LongArray_t LongArray_new(int initial_capacity);
void LongArray_free(LongArray_t array);
void LongArray_clear(LongArray_t array);
int LongArray_getCapacity(LongArray_t array);
void LongArray_setCapacity(LongArray_t array, int capacity);
int LongArray_getSize(LongArray_t array);
void LongArray_setSize(LongArray_t array, int size);
long *LongArray_getBuffer(LongArray_t array);
long LongArray_get(LongArray_t array, int element_number);
void LongArray_set(LongArray_t array, int element_number, long value);
void LongArray_add(LongArray_t array, long value);
void LongArray_insert(LongArray_t array, int element_number, long value);
void LongArray_remove(LongArray_t array, int element_number);
void LongArray_setValues(LongArray_t array, long *values, int number_of_values);
void LongArray_addValues(LongArray_t array, long *values, int number_of_values);
void LongArray_replaceValues(LongArray_t array, int element_number, long *values, int number_of_values);
void LongArray_insertValues(LongArray_t array, int element_number, long *values, int number_of_values);
void LongArray_removeValues(LongArray_t array, int element_number, int number_of_values);

FloatArray_t FloatArray_new(int initial_capacity);
void FloatArray_free(FloatArray_t array);
void FloatArray_clear(FloatArray_t array);
int FloatArray_getCapacity(FloatArray_t array);
void FloatArray_setCapacity(FloatArray_t array, int capacity);
int FloatArray_getSize(FloatArray_t array);
void FloatArray_setSize(FloatArray_t array, int size);
float *FloatArray_getBuffer(FloatArray_t array);
float FloatArray_get(FloatArray_t array, int element_number);
void FloatArray_set(FloatArray_t array, int element_number, float value);
void FloatArray_add(FloatArray_t array, float value);
void FloatArray_insert(FloatArray_t array, int element_number, float value);
void FloatArray_remove(FloatArray_t array, int element_number);
void FloatArray_setValues(FloatArray_t array, float *values, int number_of_values);
void FloatArray_addValues(FloatArray_t array, float *values, int number_of_values);
void FloatArray_replaceValues(FloatArray_t array, int element_number, float *values, int number_of_values);
void FloatArray_insertValues(FloatArray_t array, int element_number, float *values, int number_of_values);
void FloatArray_removeValues(FloatArray_t array, int element_number, int number_of_values);

DoubleArray_t DoubleArray_new(int initial_capacity);
void DoubleArray_free(DoubleArray_t array);
void DoubleArray_clear(DoubleArray_t array);
int DoubleArray_getCapacity(DoubleArray_t array);
void DoubleArray_setCapacity(DoubleArray_t array, int capacity);
int DoubleArray_getSize(DoubleArray_t array);
void DoubleArray_setSize(DoubleArray_t array, int size);
double *DoubleArray_getBuffer(DoubleArray_t array);
double DoubleArray_get(DoubleArray_t array, int element_number);
void DoubleArray_set(DoubleArray_t array, int element_number, double value);
void DoubleArray_add(DoubleArray_t array, double value);
void DoubleArray_insert(DoubleArray_t array, int element_number, double value);
void DoubleArray_remove(DoubleArray_t array, int element_number);
void DoubleArray_setValues(DoubleArray_t array, double *values, int number_of_values);
void DoubleArray_addValues(DoubleArray_t array, double *values, int number_of_values);
void DoubleArray_replaceValues(DoubleArray_t array, int element_number, double *values, int number_of_values);
void DoubleArray_insertValues(DoubleArray_t array, int element_number, double *values, int number_of_values);
void DoubleArray_removeValues(DoubleArray_t array, int element_number, int number_of_values);

PointerArray_t PointerArray_new(int initial_capacity);
void PointerArray_free(PointerArray_t array);
void PointerArray_clear(PointerArray_t array);
int PointerArray_getCapacity(PointerArray_t array);
void PointerArray_setCapacity(PointerArray_t array, int capacity);
int PointerArray_getSize(PointerArray_t array);
void PointerArray_setSize(PointerArray_t array, int size);
void **PointerArray_getBuffer(PointerArray_t array);
void *PointerArray_get(PointerArray_t array, int element_number);
void PointerArray_set(PointerArray_t array, int element_number, void *value);
void PointerArray_add(PointerArray_t array, void *value);
void PointerArray_insert(PointerArray_t array, int element_number, void *value);
void PointerArray_remove(PointerArray_t array, int element_number);
void PointerArray_setValues(PointerArray_t array, void **values, int number_of_values);
void PointerArray_addValues(PointerArray_t array, void **values, int number_of_values);
void PointerArray_replaceValues(PointerArray_t array, int element_number, void **values, int number_of_values);
void PointerArray_insertValues(PointerArray_t array, int element_number, void **values, int number_of_values);
void PointerArray_removeValues(PointerArray_t array, int element_number, int number_of_values);

String_t String_new(int initial_capacity);
void String_free(String_t string);
void String_clear(String_t string);
int String_getCapacity(String_t string);
void String_setCapacity(String_t string, int capacity);
int String_getSize(String_t string);
unsigned char *String_get(String_t string);
void String_set(String_t string, unsigned char *value);
void String_addChar(String_t string, unsigned char value);
void String_addString(String_t string, unsigned char *value);

BlobArray_t BlobArray_new(int initial_element_capacity, int initial_byte_capacity);
void BlobArray_free(BlobArray_t array);
void BlobArray_clear(BlobArray_t array);
int BlobArray_getSize(BlobArray_t array);
unsigned char *BlobArray_getValue(BlobArray_t array, int element_number);
int BlobArray_getValueSize(BlobArray_t array, int element_number);
void BlobArray_set(BlobArray_t array, int element_number, unsigned char *value, int value_size);
void BlobArray_add(BlobArray_t array, unsigned char *value, int value_size);
void BlobArray_insert(BlobArray_t array, int element_number, unsigned char *value, int value_size);
void BlobArray_remove(BlobArray_t array, int element_number);

StringArray_t StringArray_new(int initial_element_capacity, int initial_byte_capacity);
void StringArray_free(StringArray_t array);
void StringArray_clear(StringArray_t array);
int StringArray_getSize(StringArray_t array);
unsigned char *StringArray_get(StringArray_t array, int element_number);
void StringArray_set(StringArray_t array, int element_number, unsigned char *value);
void StringArray_add(StringArray_t array, unsigned char *value);
void StringArray_insert(StringArray_t array, int element_number, unsigned char *value);
void StringArray_remove(StringArray_t array, int element_number);

IntIntMap_t IntIntMap_new(int number_of_buckets);
void IntIntMap_free(IntIntMap_t map);
void IntIntMap_clear(IntIntMap_t map);
int IntIntMap_hasKey(IntIntMap_t map, int key);
int IntIntMap_get(IntIntMap_t map, int key, int default_value);
void IntIntMap_set(IntIntMap_t map, int key, int value);
void IntIntMap_remove(IntIntMap_t map, int key);
void IntIntMap_enumerate(IntIntMap_t map, int (*callback)(int key, int value, void *user_data), void *user_data);

IntPointerMap_t IntPointerMap_new(int number_of_buckets);
void IntPointerMap_setFreeCallback(IntPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void IntPointerMap_free(IntPointerMap_t map);
void IntPointerMap_clear(IntPointerMap_t map);
int IntPointerMap_hasKey(IntPointerMap_t map, int key);
void *IntPointerMap_get(IntPointerMap_t map, int key);
void IntPointerMap_set(IntPointerMap_t map, int key, void *value);
void IntPointerMap_remove(IntPointerMap_t map, int key);
void IntPointerMap_enumerate(IntPointerMap_t map, int (*callback)(int key, void *value, void *user_data), void *user_data);

LongIntMap_t LongIntMap_new(int number_of_buckets);
void LongIntMap_free(LongIntMap_t map);
void LongIntMap_clear(LongIntMap_t map);
int LongIntMap_hasKey(LongIntMap_t map, long key);
int LongIntMap_get(LongIntMap_t map, long key, int default_value);
void LongIntMap_set(LongIntMap_t map, long key, int value);
void LongIntMap_remove(LongIntMap_t map, long key);
void LongIntMap_enumerate(LongIntMap_t map, int (*callback)(long key, int value, void *user_data), void *user_data);

LongPointerMap_t LongPointerMap_new(int number_of_buckets);
void LongPointerMap_setFreeCallback(LongPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void LongPointerMap_free(LongPointerMap_t map);
void LongPointerMap_clear(LongPointerMap_t map);
int LongPointerMap_hasKey(LongPointerMap_t map, long key);
void *LongPointerMap_get(LongPointerMap_t map, long key);
void LongPointerMap_set(LongPointerMap_t map, long key, void *value);
void LongPointerMap_remove(LongPointerMap_t map, long key);
void LongPointerMap_enumerate(LongPointerMap_t map, int (*callback)(long key, void *value, void *user_data), void *user_data);

PointerIntMap_t PointerIntMap_new(int number_of_buckets);
void PointerIntMap_free(PointerIntMap_t map);
void PointerIntMap_clear(PointerIntMap_t map);
int PointerIntMap_hasKey(PointerIntMap_t map, void *key);
int PointerIntMap_get(PointerIntMap_t map, void *key, int default_value);
void PointerIntMap_set(PointerIntMap_t map, void *key, int value);
void PointerIntMap_remove(PointerIntMap_t map, void *key);
void PointerIntMap_enumerate(PointerIntMap_t map, int (*callback)(void *key, int value, void *user_data), void *user_data);

PointerPointerMap_t PointerPointerMap_new(int number_of_buckets);
void PointerPointerMap_setFreeCallback(PointerPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void PointerPointerMap_free(PointerPointerMap_t map);
void PointerPointerMap_clear(PointerPointerMap_t map);
int PointerPointerMap_hasKey(PointerPointerMap_t map, void *key);
void *PointerPointerMap_get(PointerPointerMap_t map, void *key);
void PointerPointerMap_set(PointerPointerMap_t map, void *key, void *value);
void PointerPointerMap_remove(PointerPointerMap_t map, void *key);
void PointerPointerMap_enumerate(PointerPointerMap_t map, int (*callback)(void *key, void *value, void *user_data), void *user_data);

BlobIntMap_t BlobIntMap_new(int number_of_buckets);
void BlobIntMap_free(BlobIntMap_t map);
void BlobIntMap_clear(BlobIntMap_t map);
int BlobIntMap_hasKey(BlobIntMap_t map, unsigned char *key, int key_size);
int BlobIntMap_get(BlobIntMap_t map, unsigned char *key, int key_size, int default_value);
void BlobIntMap_set(BlobIntMap_t map, unsigned char *key, int key_size, int value);
void BlobIntMap_remove(BlobIntMap_t map, unsigned char *key, int key_size);
void BlobIntMap_enumerate(BlobIntMap_t map, int (*callback)(unsigned char *key, int key_size, int value, void *user_data), void *user_data);

BlobPointerMap_t BlobPointerMap_new(int number_of_buckets);
void BlobPointerMap_setFreeCallback(BlobPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void BlobPointerMap_free(BlobPointerMap_t map);
void BlobPointerMap_clear(BlobPointerMap_t map);
int BlobPointerMap_hasKey(BlobPointerMap_t map, unsigned char *key, int key_size);
void *BlobPointerMap_get(BlobPointerMap_t map, unsigned char *key, int key_size);
void BlobPointerMap_set(BlobPointerMap_t map, unsigned char *key, int key_size, void *value);
void BlobPointerMap_remove(BlobPointerMap_t map, unsigned char *key, int key_size);
void BlobPointerMap_enumerate(BlobPointerMap_t map, int (*callback)(unsigned char *key, int key_size, void *value, void *user_data), void *user_data);

StringIntMap_t StringIntMap_new(int number_of_buckets);
void StringIntMap_free(StringIntMap_t map);
void StringIntMap_clear(StringIntMap_t map);
int StringIntMap_hasKey(StringIntMap_t map, unsigned char *key);
int StringIntMap_get(StringIntMap_t map, unsigned char *key, int default_value);
void StringIntMap_set(StringIntMap_t map, unsigned char *key, int value);
void StringIntMap_remove(StringIntMap_t map, unsigned char *key);
void StringIntMap_enumerate(StringIntMap_t map, int (*callback)(unsigned char *key, int value, void *user_data), void *user_data);

StringPointerMap_t StringPointerMap_new(int number_of_buckets);
void StringPointerMap_setFreeCallback(StringPointerMap_t map, void (*callback)(void *value, void *user_data), void *user_data);
void StringPointerMap_free(StringPointerMap_t map);
void StringPointerMap_clear(StringPointerMap_t map);
int StringPointerMap_hasKey(StringPointerMap_t map, unsigned char *key);
void *StringPointerMap_get(StringPointerMap_t map, unsigned char *key);
void StringPointerMap_set(StringPointerMap_t map, unsigned char *key, void *value);
void StringPointerMap_remove(StringPointerMap_t map, unsigned char *key);
void StringPointerMap_enumerate(StringPointerMap_t map, int (*callback)(unsigned char *key, void *value, void *user_data), void *user_data);

void quicksort(int number_of_elements, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data);
void heapsort(int number_of_elements, int (*compare_callback)(int first_element_number, int second_element_number, void *user_data), void (*exchange_callback)(int first_element_number, int second_element_number, void *user_data), void *user_data);

#ifdef __cplusplus
}
#endif

#endif
