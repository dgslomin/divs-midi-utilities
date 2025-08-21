#ifndef SYNTINA_DRIVER_UTILS_INCLUDED
#define SYNTINA_DRIVER_UTILS_INCLUDED

typedef struct PerformanceTimer *PerformanceTimer_t;
PerformanceTimer_t PerformanceTimer_new(void);
void PerformanceTimer_free(PerformanceTimer_t performance_timer);
void PerformanceTimer_start(PerformanceTimer_t performance_timer);
float PerformanceTimer_elapsed(PerformanceTimer_t performance_timer);

typedef struct Smoother *Smoother_t;
Smoother_t Smoother_new(int number_of_samples);
void Smoother_free(Smoother_t smoother);
void Smoother_addSample(Smoother_t smoother, float sample);
float Smoother_getAverage(Smoother_t smoother);

#endif
