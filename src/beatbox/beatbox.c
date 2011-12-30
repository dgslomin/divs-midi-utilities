
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef REENTRANT
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#endif
#include <expat.h>
#include <sndfile.h>
#include "beatbox.h"

typedef struct beatbox_waveform *BeatboxWaveform_t;
typedef struct beatbox_instrument *BeatboxInstrument_t;
typedef struct beatbox_cutoff_group *BeatboxCutoffGroup_t;
typedef struct beatbox_voice *BeatboxVoice_t;

struct beatbox
{
	BeatboxWaveform_t first_waveform;
	BeatboxInstrument_t first_instrument;
	BeatboxCutoffGroup_t first_cutoff_group;

	struct
	{
		BeatboxInstrument_t instrument;
		BeatboxCutoffGroup_t cutoff_group;
	}
	note_mapping[128];

	BeatboxVoice_t first_inactive_voice;
	BeatboxVoice_t first_active_voice;
	BeatboxVoice_t last_active_voice;

#ifdef REENTRANT
#ifdef WIN32
	HANDLE lock;
#else
	pthread_mutex_t lock;
#endif
#endif
};

struct beatbox_waveform
{
	char *filename;
	float *data;
	long frames;
	BeatboxWaveform_t next_waveform;
};

struct beatbox_instrument
{
	char *name;
	BeatboxWaveform_t waveform;
	BeatboxInstrument_t next_instrument;
};

struct beatbox_cutoff_group
{
	char *name;
	BeatboxVoice_t first_voice;
	BeatboxCutoffGroup_t next_cutoff_group;
};

struct beatbox_voice
{
	BeatboxInstrument_t instrument;
	BeatboxCutoffGroup_t cutoff_group;
	int velocity;
	int cutoff;
	long frame;
	BeatboxVoice_t previous_voice;
	BeatboxVoice_t next_voice;
	BeatboxVoice_t previous_voice_in_cutoff_group;
	BeatboxVoice_t next_voice_in_cutoff_group;
};

static BeatboxWaveform_t get_waveform(Beatbox_t beatbox, char *waveform_filename)
{
	BeatboxWaveform_t waveform;

	if (waveform_filename == NULL)
	{
		return NULL;
	}

	for (waveform = beatbox->first_waveform; waveform != NULL; waveform = waveform->next_waveform)
	{
		if (strcmp(waveform->filename, waveform_filename) == 0)
		{
			break;
		}
	}

	if (waveform == NULL)
	{
		SF_INFO sfinfo;
		SNDFILE *sndfile;
		float *data;
		long frames;

		sfinfo.format = 0;

		if ((sndfile = sf_open(waveform_filename, SFM_READ, &sfinfo)) == NULL)
		{
			data = NULL;
			frames = 0;
		}
		else
		{
			data = (float *)(malloc(sizeof(float) * (long)(sfinfo.frames) * sfinfo.channels));
			sf_readf_float(sndfile, data, sfinfo.frames);
			sf_close(sndfile);

			if (sfinfo.channels == 1)
			{
				float *modified_data = (float *)(malloc(sizeof(float) * (long)(sfinfo.frames) * 2));
				long i;

				for (i = 0; i < sfinfo.frames; i++)
				{
					modified_data[i * 2] = data[i];
					modified_data[(i * 2) + 1] = data[i];
				}

				free(data);
				data = modified_data;
			}
			else if (sfinfo.channels > 2)
			{
				float *modified_data = (float *)(malloc(sizeof(float) * (long)(sfinfo.frames) * 2));
				long i;

				for (i = 0; i < sfinfo.frames; i++)
				{
					modified_data[i * 2] = data[i * sfinfo.channels];
					modified_data[(i * 2) + 1] = data[(i * sfinfo.channels) + 1];
				}

				free(data);
				data = modified_data;
			}

			if (sfinfo.samplerate == 44100)
			{
				frames = sfinfo.frames;
			}
			else
			{
				float *modified_data;
				long i;

				frames = sfinfo.frames * 44100 / sfinfo.samplerate;
				modified_data = (float *)(malloc(sizeof(float) * frames * 2));

				for (i = 0; i < frames; i++)
				{
					modified_data[i * 2] = data[(i * sfinfo.samplerate / 44100) * 2];
					modified_data[(i * 2) + 1] = data[((i * sfinfo.samplerate / 44100) * 2) + 1];
				}

				free(data);
				data = modified_data;
			}
		}

		if (data != NULL)
		{
			waveform = (BeatboxWaveform_t)(malloc(sizeof(struct beatbox_waveform)));
			waveform->filename = strdup(waveform_filename);
			waveform->data = data;
			waveform->frames = frames;
			waveform->next_waveform = beatbox->first_waveform;
			beatbox->first_waveform = waveform;
		}
	}

	return waveform;
}

static BeatboxInstrument_t get_instrument(Beatbox_t beatbox, char *instrument_name)
{
	BeatboxInstrument_t instrument;

	if (instrument_name == NULL)
	{
		return NULL;
	}

	for (instrument = beatbox->first_instrument; instrument != NULL; instrument = instrument->next_instrument)
	{
		if (strcmp(instrument->name, instrument_name) == 0)
		{
			break;
		}
	}

	if (instrument == NULL)
	{
		instrument = (BeatboxInstrument_t)(malloc(sizeof(struct beatbox_instrument)));
		instrument->name = strdup(instrument_name);
		instrument->waveform = NULL;
		instrument->next_instrument = beatbox->first_instrument;
		beatbox->first_instrument = instrument;
	}

	return instrument;
}

static BeatboxCutoffGroup_t get_cutoff_group(Beatbox_t beatbox, char *cutoff_group_name)
{
	BeatboxCutoffGroup_t cutoff_group;

	if (cutoff_group_name == NULL)
	{
		return NULL;
	}

	for (cutoff_group = beatbox->first_cutoff_group; cutoff_group != NULL; cutoff_group = cutoff_group->next_cutoff_group)
	{
		if (strcmp(cutoff_group->name, cutoff_group_name) == 0)
		{
			break;
		}
	}

	if (cutoff_group == NULL)
	{
		cutoff_group = (BeatboxCutoffGroup_t)(malloc(sizeof(struct beatbox_cutoff_group)));
		cutoff_group->name = strdup(cutoff_group_name);
		cutoff_group->first_voice = NULL;
		cutoff_group->next_cutoff_group = beatbox->first_cutoff_group;
		beatbox->first_cutoff_group = cutoff_group;
	}

	return cutoff_group;
}

Beatbox_t Beatbox_new(int number_of_voices)
{
	Beatbox_t beatbox = (Beatbox_t)(malloc(sizeof(struct beatbox)));
	int i;

	beatbox->first_waveform = NULL;
	beatbox->first_instrument = NULL;
	beatbox->first_cutoff_group = NULL;

	for (i = 0; i < 128; i++)
	{
		beatbox->note_mapping[i].instrument = NULL;
		beatbox->note_mapping[i].cutoff_group = NULL;
	}

	beatbox->first_inactive_voice = NULL;

	for (i = 0; i < number_of_voices; i++)
	{
		BeatboxVoice_t new_voice = (BeatboxVoice_t)(malloc(sizeof(struct beatbox_voice)));
		new_voice->instrument = NULL;
		new_voice->cutoff_group = NULL;
		new_voice->velocity = 0;
		new_voice->cutoff = 0;
		new_voice->previous_voice = NULL;
		new_voice->next_voice = beatbox->first_inactive_voice;
		new_voice->previous_voice_in_cutoff_group = NULL;
		new_voice->next_voice_in_cutoff_group = NULL;

		if (beatbox->first_inactive_voice != NULL)
		{
			beatbox->first_inactive_voice->previous_voice = new_voice;
		}

		beatbox->first_inactive_voice = new_voice;
	}

	beatbox->first_active_voice = NULL;
	beatbox->last_active_voice = NULL;

#ifdef REENTRANT
#ifdef WIN32
	beatbox->lock = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutex_init(&(beatbox->lock), NULL);
#endif
#endif

	return beatbox;
}

void Beatbox_free(Beatbox_t beatbox)
{
	BeatboxWaveform_t waveform, next_waveform;
	BeatboxInstrument_t instrument, next_instrument;
	BeatboxCutoffGroup_t cutoff_group, next_cutoff_group;
	BeatboxVoice_t voice, next_voice;

	for (waveform = beatbox->first_waveform; waveform != NULL; waveform = next_waveform)
	{
		next_waveform = waveform->next_waveform;
		free(waveform->filename);
		free(waveform->data);
		free(waveform);
	}

	for (instrument = beatbox->first_instrument; instrument != NULL; instrument = next_instrument)
	{
		next_instrument = instrument->next_instrument;
		free(instrument->name);
		free(instrument);
	}

	for (cutoff_group = beatbox->first_cutoff_group; cutoff_group != NULL; cutoff_group = next_cutoff_group)
	{
		next_cutoff_group = cutoff_group->next_cutoff_group;
		free(cutoff_group->name);
		free(cutoff_group);
	}

	for (voice = beatbox->first_inactive_voice; voice != NULL; voice = next_voice)
	{
		next_voice = voice->next_voice;
		free(voice);
	}

	for (voice = beatbox->first_active_voice; voice != NULL; voice = next_voice)
	{
		next_voice = voice->next_voice;
		free(voice);
	}

#ifdef REENTRANT
#ifdef WIN32
	CloseHandle(beatbox->lock);
#else
	pthread_mutex_destroy(&(beatbox->lock));
#endif
#endif

	free(beatbox);
}

void Beatbox_setInstrumentWaveform(Beatbox_t beatbox, char *instrument_name, char *waveform_filename)
{
	BeatboxInstrument_t instrument = get_instrument(beatbox, instrument_name);

	if (instrument != NULL)
	{
		instrument->waveform = get_waveform(beatbox, waveform_filename);
	}
}

void Beatbox_setNoteInstrument(Beatbox_t beatbox, int note, char *instrument_name)
{
	beatbox->note_mapping[note].instrument = get_instrument(beatbox, instrument_name);
}

void Beatbox_setNoteCutoffGroup(Beatbox_t beatbox, int note, char *cutoff_group_name)
{
	beatbox->note_mapping[note].cutoff_group = get_cutoff_group(beatbox, cutoff_group_name);
}

static void xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **attributes)
{
	Beatbox_t beatbox = (Beatbox_t)(user_data);

	if (strcmp(name, "instrument") == 0)
	{
		int i;
		char *instrument_name = NULL;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "name") == 0)
			{
				instrument_name = (char *)(attributes[i + 1]);
			}
			else if (strcmp(attributes[i], "waveform") == 0)
			{
				Beatbox_setInstrumentWaveform(beatbox, instrument_name, (char *)(attributes[i + 1]));
			}
		}
	}
	else if (strcmp(name, "note") == 0)
	{
		int i;
		int note_number = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "number") == 0)
			{
				note_number = atoi(attributes[i + 1]);
			}
			else if (strcmp(attributes[i], "instrument") == 0)
			{
				Beatbox_setNoteInstrument(beatbox, note_number, (char *)(attributes[i + 1]));
			}
			else if (strcmp(attributes[i], "cutoff-group") == 0)
			{
				Beatbox_setNoteCutoffGroup(beatbox, note_number, (char *)(attributes[i + 1]));
			}
		}
	}
}

void Beatbox_loadConfig(Beatbox_t beatbox, char *filename)
{
	XML_Parser parser;
	FILE *f;
	void *buffer;
	int bytes_read;

	if ((f = fopen(filename, "rb")) == NULL) return;

	parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, beatbox);
	XML_SetStartElementHandler(parser, xml_start_element_handler);

	do
	{
		buffer = XML_GetBuffer(parser, 1024);
		bytes_read = fread(buffer, 1, 1024, f);
		XML_ParseBuffer(parser, bytes_read, (bytes_read == 0));
	}
	while (bytes_read > 0);

	XML_ParserFree(parser);

	fclose(f);
}

void Beatbox_noteOn(Beatbox_t beatbox, int note, int velocity)
{
	BeatboxInstrument_t instrument;
	BeatboxCutoffGroup_t cutoff_group;
	BeatboxVoice_t voice;

#ifdef REENTRANT
#ifdef WIN32
	WaitForSingleObject(beatbox->lock, INFINITE);
#else
	pthread_mutex_lock(&(beatbox->lock));
#endif
#endif

	instrument = beatbox->note_mapping[note].instrument;
	cutoff_group = beatbox->note_mapping[note].cutoff_group;

	if (cutoff_group != NULL)
	{
		for (voice = cutoff_group->first_voice; voice != NULL; voice = voice->next_voice_in_cutoff_group)
		{
			voice->cutoff = 1;
		}
	}

	if ((instrument != NULL) && (instrument->waveform != NULL))
	{
		if (beatbox->first_inactive_voice == NULL)
		{
			voice = beatbox->first_active_voice;
			beatbox->first_active_voice = voice->next_voice;

			if (beatbox->first_active_voice == NULL)
			{
				beatbox->last_active_voice = NULL;
			}
			else
			{
				beatbox->first_active_voice->previous_voice = NULL;
			}

			if (voice->previous_voice_in_cutoff_group != NULL)
			{
				voice->previous_voice_in_cutoff_group->next_voice_in_cutoff_group = voice->next_voice_in_cutoff_group;
			}

			if (voice->next_voice_in_cutoff_group != NULL)
			{
				voice->next_voice_in_cutoff_group->previous_voice_in_cutoff_group = voice->previous_voice_in_cutoff_group;
			}
		}
		else
		{
			voice = beatbox->first_inactive_voice;
			beatbox->first_inactive_voice = voice->next_voice;

			if (beatbox->first_inactive_voice != NULL)
			{
				beatbox->first_inactive_voice->previous_voice = NULL;
			}
		}

		voice->next_voice = NULL;

		if (beatbox->last_active_voice == NULL)
		{
			beatbox->first_active_voice = voice;
		}
		else
		{
			beatbox->last_active_voice->next_voice = voice;
		}

		voice->previous_voice = beatbox->last_active_voice;
		beatbox->last_active_voice = voice;

		voice->previous_voice_in_cutoff_group = NULL;

		if (cutoff_group == NULL)
		{
			voice->next_voice_in_cutoff_group = NULL;
		}
		else
		{
			voice->next_voice_in_cutoff_group = cutoff_group->first_voice;

			if (cutoff_group->first_voice != NULL)
			{
				cutoff_group->first_voice->previous_voice_in_cutoff_group = voice;
			}

			cutoff_group->first_voice = voice;
		}

		voice->instrument = instrument;
		voice->cutoff_group = cutoff_group;
		voice->velocity = velocity;
		voice->cutoff = 0;
		voice->frame = 0;
	}

#ifdef REENTRANT
#ifdef WIN32
	ReleaseMutex(beatbox->lock);
#else
	pthread_mutex_unlock(&(beatbox->lock));
#endif
#endif
}

void Beatbox_computeAudio(Beatbox_t beatbox, float *data, unsigned long frames)
{
	unsigned long i;
	BeatboxVoice_t voice, next_voice;

#ifdef REENTRANT
#ifdef WIN32
	WaitForSingleObject(beatbox->lock, INFINITE);
#else
	pthread_mutex_lock(&(beatbox->lock));
#endif
#endif

	for (i = 0; i < frames; i++)
	{
		data[i * 2] = 0.0;
		data[(i * 2) + 1] = 0.0;
	}

	for (voice = beatbox->first_active_voice; voice != NULL; voice = next_voice)
	{
		next_voice = voice->next_voice;

		for (i = 0; i < frames; i++)
		{
			if ((voice->cutoff) || (voice->frame == voice->instrument->waveform->frames))
			{
				if (voice->previous_voice == NULL)
				{
					beatbox->first_active_voice = voice->next_voice;
				}
				else
				{
					voice->previous_voice->next_voice = voice->next_voice;
				}

				if (voice->next_voice == NULL)
				{
					beatbox->last_active_voice = voice->previous_voice;
				}
				else
				{
					voice->next_voice->previous_voice = voice->previous_voice;
				}

				voice->previous_voice = NULL;
				voice->next_voice = beatbox->first_inactive_voice;
				beatbox->first_inactive_voice->previous_voice = voice;
				beatbox->first_inactive_voice = voice;

				if (voice->cutoff_group != NULL)
				{
					if (voice->previous_voice_in_cutoff_group == NULL)
					{
						voice->cutoff_group->first_voice = voice->next_voice_in_cutoff_group;
					}
					else
					{
						voice->previous_voice_in_cutoff_group->next_voice_in_cutoff_group = voice->next_voice_in_cutoff_group;
					}

					if (voice->next_voice_in_cutoff_group != NULL)
					{
						voice->next_voice_in_cutoff_group->previous_voice = voice->previous_voice_in_cutoff_group;
					}
				}

				break;
			}
			else
			{
				data[i * 2] += voice->instrument->waveform->data[voice->frame * 2] * voice->velocity / 127;
				data[(i * 2) + 1] += voice->instrument->waveform->data[(voice->frame * 2) + 1] * voice->velocity / 127;
				(voice->frame)++;
			}
		}
	}

#ifdef REENTRANT
#ifdef WIN32
	ReleaseMutex(beatbox->lock);
#else
	pthread_mutex_unlock(&(beatbox->lock));
#endif
#endif
}

