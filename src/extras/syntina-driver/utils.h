#ifndef SYNTINA_DRIVER_UTILS_INCLUDED
#define SYNTINA_DRIVER_UTILS_INCLUDED

float Time_getTime(void);

typedef struct Smoother *Smoother_t;
Smoother_t Smoother_new(int number_of_samples);
void Smoother_free(Smoother_t smoother);
void Smoother_addSample(Smoother_t smoother, float sample);
float Smoother_getAverage(Smoother_t smoother);

int YesNoToggle_parse(const char *s);
int YesNoToggle_apply(int current_value, int yes_no_toggle);

int clamp_int(int value, int min, int max);

#endif
