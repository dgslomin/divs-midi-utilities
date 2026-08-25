#ifndef SYNTINA_DRIVER_UTILS_INCLUDED
#define SYNTINA_DRIVER_UTILS_INCLUDED

float Time_getTime(void);

typedef struct Smoother *Smoother_t;
Smoother_t Smoother_new(int number_of_samples);
void Smoother_free(Smoother_t smoother);
void Smoother_addSample(Smoother_t smoother, float sample);
float Smoother_getAverage(Smoother_t smoother);

typedef struct Set *Set_t;
Set_t Set_new(int size);
void Set_free(Set_t set);
void Set_clear(Set_t set);
void Set_add(Set_t set, int value);
void Set_remove(Set_t set, int value);
int Set_has(Set_t set, int value);
int Set_count(Set_t set);
int Set_nth(Set_t set, int number);

int YesNoToggle_parse(const char *s);
int YesNoToggle_apply(int current_value, int yes_no_toggle);

int clamp_int(int value, int min, int max);

#endif
