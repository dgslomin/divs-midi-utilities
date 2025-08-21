
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

struct PerformanceTimer
{
	struct timespec start;
};

typedef struct PerformanceTimer *PerformanceTimer_t;

PerformanceTimer_t PerformanceTimer_new(void)
{
	return (PerformanceTimer_t)(malloc(sizeof (struct PerformanceTimer)));
}

void PerformanceTimer_free(PerformanceTimer_t performance_timer)
{
	free(performance_timer);
}

void PerformanceTimer_start(PerformanceTimer_t performance_timer)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, &(performance_timer->start));
}

float PerformanceTimer_elapsed(PerformanceTimer_t performance_timer)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now);
	return ((now.tv_sec - performance_timer->start.tv_sec) * 1.0) + ((now.tv_nsec - performance_timer->start.tv_nsec) / 1000000000.0);
}

struct Smoother
{
	int number_of_samples;
	int sample_number;
	float total;
	float *samples;
};

Smoother_t Smoother_new(int number_of_samples)
{
	Smoother_t smoother = (Smoother_t)(malloc(sizeof (struct Smoother)));
	smoother->number_of_samples = number_of_samples;
	smoother->sample_number = 0;
	smoother->total = 0;
	smoother->samples = (float *)(malloc(number_of_samples * sizeof (float)));
	return smoother;
}

void Smoother_free(Smoother_t smoother)
{
	free(smoother->samples);
	free(smoother);
}

void Smoother_addSample(Smoother_t smoother, float sample)
{
	smoother->samples[smoother->sample_number] = sample;
	smoother->sample_number = (smoother->sample_number + 1) % smoother->number_of_samples;
	smoother->total = smoother->total + sample - smoother->samples[smoother->sample_number];
}

float Smoother_getAverage(Smoother_t smoother)
{
	return smoother->total / smoother->number_of_samples;
}

