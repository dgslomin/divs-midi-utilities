
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"

float Time_getTime(void)
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return (time.tv_sec * 1.0) + (time.tv_nsec / 1000000000.0);
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
	for (int sample_number = 0; sample_number < number_of_samples; sample_number++) smoother->samples[sample_number] = 0;
	return smoother;
}

void Smoother_free(Smoother_t smoother)
{
	free(smoother->samples);
	free(smoother);
}

void Smoother_addSample(Smoother_t smoother, float sample)
{
	smoother->total = smoother->total + sample - smoother->samples[smoother->sample_number];
	smoother->samples[smoother->sample_number] = sample;
	smoother->sample_number = (smoother->sample_number + 1) % smoother->number_of_samples;
}

float Smoother_getAverage(Smoother_t smoother)
{
	return smoother->total / smoother->number_of_samples;
}

int YesNoToggle_parse(const char *s)
{
	if (s == NULL) return -1;
	if (strcmp(s, "no") == 0) return 0;
	if (strcmp(s, "yes") == 0) return 1;
	if (strcmp(s, "toggle") == 0) return 2;
	return -1;
}

int YesNoToggle_apply(int current_value, int yes_no_toggle)
{
	if (yes_no_toggle == 0) return 0;
	if (yes_no_toggle == 1) return 1;
	if (yes_no_toggle == 2) return !current_value;
	return current_value;
}

int clamp_int(int value, int min, int max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

