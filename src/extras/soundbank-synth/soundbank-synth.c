
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <portaudio.h>

#ifdef USE_SFIZZ
#include <sfizz.h>
#endif

#ifdef USE_FLUIDSYNTH
#include <fluidsynth.h>
#endif

#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

typedef enum
{
	SYNTH_ENGINE_NONE,
	SYNTH_ENGINE_SFIZZ,
	SYNTH_ENGINE_FLUIDSYNTH
}
SynthEngine_t;

SynthEngine_t synth_engine = SYNTH_ENGINE_NONE;

#ifdef USE_SFIZZ
sfizz_synth_t *sfizz_synth;
#endif

#ifdef USE_FLUIDSYNTH
fluid_settings_t *fluidsynth_settings;
fluid_synth_t *fluidsynth_synth;
#endif

static RtMidiInPtr midi_in;
PaStream *audio_out;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --in <port> [ --sample-rate <default: 44100> ] [ --buffer-size <default: 256> ] [ --soundbank <filename> ] [ --program <number> ]\n", program_name);
	exit(1);
}

static int string_ends_with(char *str, char *suffix)
{
	if (str == NULL || suffix == NULL) return 0;
	int str_len = strlen(str);
	int suffix_len = strlen(suffix);
	return ((str_len >= suffix_len) && (strcmp(str + str_len - suffix_len, suffix) == 0));
}

static int load_soundbank(char *soundbank_filename)
{
#ifdef USE_SFIZZ
	if (string_ends_with(soundbank_filename, ".sfz") && sfizz_load_file(sfizz_synth, soundbank_filename))
	{
		synth_engine = SYNTH_ENGINE_SFIZZ;
		return 1;
	}
#endif

#ifdef USE_FLUIDSYNTH
	fluid_sfont_t *fluidsynth_sfont = fluid_synth_get_sfont(fluidsynth_synth, 0);

	if (fluidsynth_sfont != NULL)
	{
		fluid_synth_sfunload(fluidsynth_synth, fluid_sfont_get_id(fluidsynth_sfont), 1);
	}

	if (fluid_synth_sfload(fluidsynth_synth, soundbank_filename, 1) != FLUID_FAILED)
	{
		synth_engine = SYNTH_ENGINE_FLUIDSYNTH;
		return 1;
	}
#endif

	synth_engine = SYNTH_ENGINE_NONE;
	return 0;
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			switch (synth_engine)
			{
				case SYNTH_ENGINE_SFIZZ:
				{
#ifdef USE_SFIZZ
					sfizz_send_note_off(sfizz_synth, 0, MidiUtilNoteOffMessage_getNote(message), MidiUtilNoteOffMessage_getVelocity(message));
#endif
					break;
				}
				case SYNTH_ENGINE_FLUIDSYNTH:
				{
#ifdef USE_FLUIDSYNTH
					fluid_synth_noteoff(fluidsynth_synth, MidiUtilNoteOffMessage_getChannel(message), MidiUtilNoteOffMessage_getNote(message));
#endif
					break;
				}
				default:
				{
					break;
				}

				break;
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			switch (synth_engine)
			{
				case SYNTH_ENGINE_SFIZZ:
				{
#ifdef USE_SFIZZ
					sfizz_send_note_on(sfizz_synth, 0, MidiUtilNoteOnMessage_getNote(message), MidiUtilNoteOnMessage_getVelocity(message));
#endif
					break;
				}
				case SYNTH_ENGINE_FLUIDSYNTH:
				{
#ifdef USE_FLUIDSYNTH
					fluid_synth_noteon(fluidsynth_synth, MidiUtilNoteOnMessage_getChannel(message), MidiUtilNoteOnMessage_getNote(message), MidiUtilNoteOnMessage_getVelocity(message));
#endif
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE:
		{
			switch (synth_engine)
			{
				case SYNTH_ENGINE_SFIZZ:
				{
#ifdef USE_SFIZZ
					sfizz_send_poly_aftertouch(sfizz_synth, 0, MidiUtilKeyPressureMessage_getNote(message), MidiUtilKeyPressureMessage_getAmount(message));
#endif
					break;
				}
				case SYNTH_ENGINE_FLUIDSYNTH:
				{
#ifdef USE_FLUIDSYNTH
					fluid_synth_key_pressure(fluidsynth_synth, MidiUtilKeyPressureMessage_getChannel(message), MidiUtilKeyPressureMessage_getNote(message), MidiUtilKeyPressureMessage_getAmount(message));
#endif
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			switch (synth_engine)
			{
				case SYNTH_ENGINE_SFIZZ:
				{
#ifdef USE_SFIZZ
					sfizz_send_cc(sfizz_synth, 0, MidiUtilControlChangeMessage_getNumber(message), MidiUtilControlChangeMessage_getValue(message));
#endif
					break;
				}
				case SYNTH_ENGINE_FLUIDSYNTH:
				{
#ifdef USE_FLUIDSYNTH
					fluid_synth_cc(fluidsynth_synth, MidiUtilControlChangeMessage_getChannel(message), MidiUtilControlChangeMessage_getNumber(message), MidiUtilControlChangeMessage_getValue(message));
#endif
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PROGRAM_CHANGE:
		{
			switch (synth_engine)
			{
				case SYNTH_ENGINE_SFIZZ:
				{
					// not supported
					break;
				}
				case SYNTH_ENGINE_FLUIDSYNTH:
				{
#ifdef USE_FLUIDSYNTH
					fluid_synth_program_change(fluidsynth_synth, MidiUtilProgramChangeMessage_getChannel(message), MidiUtilProgramChangeMessage_getNumber(message));
#endif
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CHANNEL_PRESSURE:
		{
			switch (synth_engine)
			{
				case SYNTH_ENGINE_SFIZZ:
				{
#ifdef USE_SFIZZ
					sfizz_send_channel_aftertouch(sfizz_synth, 0, MidiUtilChannelPressureMessage_getAmount(message));
#endif
					break;
				}
				case SYNTH_ENGINE_FLUIDSYNTH:
				{
#ifdef USE_FLUIDSYNTH
					fluid_synth_channel_pressure(fluidsynth_synth, MidiUtilChannelPressureMessage_getChannel(message), MidiUtilChannelPressureMessage_getAmount(message));
#endif
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL:
		{
			switch (synth_engine)
			{
				case SYNTH_ENGINE_SFIZZ:
				{
#ifdef USE_SFIZZ
					sfizz_send_pitch_wheel(sfizz_synth, 0, MidiUtilPitchWheelMessage_getValue(message));
#endif
					break;
				}
				case SYNTH_ENGINE_FLUIDSYNTH:
				{
#ifdef USE_FLUIDSYNTH
					fluid_synth_pitch_bend(fluidsynth_synth, MidiUtilPitchWheelMessage_getChannel(message), MidiUtilPitchWheelMessage_getValue(message));
#endif
					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_SYSEX:
		{
			// Message format:  0xF0 0x7D soundbank filename (must be 7-bit ASCII, not zero terminated) 0xF7

			if (message[1] != 0x7D) return;

			if (message_size > 1024)
			{
				fprintf(stderr, "Warning:  Soundbank filename too long.\n");
				return;
			}

			char soundbank_filename[1024];
			memcpy(soundbank_filename, message + 2, message_size - 3);
			soundbank_filename[message_size - 3] = '\0';

			if (!load_soundbank(soundbank_filename))
			{
				fprintf(stderr, "Warning:  Cannot load soundbank \"%s\".\n", soundbank_filename);
				return;
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

static int compute_audio_buffer(const void *input, void *output, unsigned long number_of_frames, const PaStreamCallbackTimeInfo *time_info, PaStreamCallbackFlags status_flags, void *user_data)
{
	while (1)
	{
		unsigned char message[1024];
		size_t message_size = 1024;
		if (rtmidi_in_get_message(midi_in, message, &message_size) <= 0) break;
		handle_midi_message(0, message, message_size, NULL);
	}

	float *left_output = ((float **)(output))[0];
	float *right_output = ((float **)(output))[1];

	switch (synth_engine)
	{
		case SYNTH_ENGINE_SFIZZ:
		{
#ifdef USE_SFIZZ
			sfizz_render_block(sfizz_synth, (float **)(output), 2, number_of_frames);
#endif
			break;
		}
		case SYNTH_ENGINE_FLUIDSYNTH:
		{
#ifdef USE_FLUIDSYNTH
			fluid_synth_write_float(fluidsynth_synth, number_of_frames, left_output, 0, 1, right_output, 0, 1);
#endif
			break;
		}
		default:
		{
			memset(left_output, 0, sizeof(float) * number_of_frames);
			memset(right_output, 0, sizeof(float) * number_of_frames);
			break;
		}
	}

	return 0;
}

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	Pa_StopStream(audio_out);
	Pa_CloseStream(audio_out);
	Pa_Terminate();
#ifdef USE_SFIZZ
	sfizz_free(sfizz_synth);
#endif
#ifdef USE_FLUIDSYNTH
	delete_fluid_synth(fluidsynth_synth);
	delete_fluid_settings(fluidsynth_settings);
#endif
}

int main(int argc, char **argv)
{
	int i;
	char *midi_in_port = NULL;
	int sample_rate = 44100;
	int buffer_size = 256;
	char *soundbank_filename = NULL;
	int program_number = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_in_port = argv[i];
		}
		else if (strcmp(argv[i], "--sample-rate") == 0)
		{
			if (++i == argc) usage(argv[0]);
			sample_rate = atol(argv[i]);
		}
		else if (strcmp(argv[i], "--buffer-size") == 0)
		{
			if (++i == argc) usage(argv[0]);
			buffer_size = atol(argv[i]);
		}
		else if (strcmp(argv[i], "--soundbank") == 0)
		{
			if (++i == argc) usage(argv[0]);
			soundbank_filename = argv[i];
		}
		else if (strcmp(argv[i], "--program") == 0)
		{
			if (++i == argc) usage(argv[0]);
			program_number = atol(argv[i]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_in_port == NULL) usage(argv[0]);

#ifdef USE_SFIZZ
	sfizz_synth = sfizz_create_synth();
	sfizz_set_sample_rate(sfizz_synth, sample_rate);
	sfizz_set_samples_per_block(sfizz_synth, buffer_size);
#endif

#ifdef USE_FLUIDSYNTH
	fluidsynth_settings = new_fluid_settings();
	fluid_settings_setnum(fluidsynth_settings, "synth.sample-rate", sample_rate);
	fluid_settings_setnum(fluidsynth_settings, "synth.gain", 0.9);
	fluidsynth_synth = new_fluid_synth(fluidsynth_settings);
#endif

	if (soundbank_filename != NULL)
	{
		if (!load_soundbank(soundbank_filename))
		{
			fprintf(stderr, "Error:  Cannot load soundbank \"%s\".\n", soundbank_filename);
			exit(1);
		}
	}

#ifdef USE_FLUIDSYNTH
	if (program_number >= 0)
	{
		fluid_synth_program_change(fluidsynth_synth, 0, program_number);
	}
#endif

	if ((midi_in = rtmidi_open_in_port("soundbank-synth", midi_in_port, "soundbank-synth", NULL, NULL)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", midi_in_port);
		exit(1);
	}

	rtmidi_in_ignore_types(midi_in, 0, 1, 1);

	Pa_Initialize();

	if (Pa_OpenDefaultStream(&audio_out, 0, 2, paFloat32 | paNonInterleaved, sample_rate, buffer_size, compute_audio_buffer, NULL) != paNoError)
	{
		fprintf(stderr, "Error:  Cannot open audio output.\n");
		exit(1);
	}

	Pa_StartStream(audio_out);

	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}

