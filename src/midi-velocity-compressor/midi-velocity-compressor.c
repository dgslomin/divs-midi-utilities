
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <assert.h>

static void usage(char *program_name)
{
	printf("Usage: %s \n"
           "      --ratio <compressor_ratio>\n"
           "      --hi <high_velocity_threshold>\n"
           "      --lo <low_velocity_threshold>\n"
           "      --note-eq <note_equal_filter>\n"
           "      --note-ne <note_not_equal_filter>\n"
           "      --note-le <note_less_than_or_equal_filter>\n"
           "      --note-ge <note_greater_than_or_equal_filter>\n"
           "      --out <output_file_name>\n"
           "      <input_file_name>\n", program_name);
	exit(1);
}

struct prefs_t
{
    char *infile;
    char *outfile;
    float ratio;
    unsigned int lo_vel_threshold;
    unsigned int hi_vel_threshold;
    unsigned int note_le_filter;
    unsigned int note_ge_filter;
    int note_eq_filter;
    int note_ne_filter;
} prefs;

void get_args(int argc, char **argv)
{
    int i;

    /* Initial values */
    prefs.infile = "in.mid";
    prefs.outfile = "out.mid";
    prefs.ratio = 0.5;
    prefs.lo_vel_threshold = 0;
    prefs.hi_vel_threshold = 127;
    prefs.note_le_filter = 127;
    prefs.note_ge_filter = 0;
    prefs.note_eq_filter = -1;
    prefs.note_ne_filter = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "--lo") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefs.lo_vel_threshold = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--hi") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefs.hi_vel_threshold = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--ratio") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefs.ratio = atof(argv[i]);
		}
		else if (strcmp(argv[i], "--note-le") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefs.note_le_filter = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--note-ge") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefs.note_ge_filter = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--note-eq") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefs.note_eq_filter = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--note-ne") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefs.note_ne_filter = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefs.outfile = argv[i];
		}
		else if (i == argc - 1)
		{
			prefs.infile = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

    /* Asserts */
    assert(prefs.infile);
    assert(prefs.outfile);
    assert(prefs.note_ne_filter >= -1);
    assert(prefs.note_ne_filter <= 127);
    assert(prefs.note_eq_filter >= -1);
    assert(prefs.note_eq_filter <= 127);
    assert(prefs.note_le_filter <= 127);
    assert(prefs.note_le_filter > 0);
    assert(prefs.note_ge_filter < 127);
    assert(prefs.lo_vel_threshold < 127);
    assert(prefs.hi_vel_threshold <= 127);
    assert(prefs.hi_vel_threshold > 0);
    assert(prefs.ratio >= 0);
    assert(prefs.ratio < 1);
}

static void visit_event(MidiFileEvent_t event, void *data)
{
    int lo = prefs.lo_vel_threshold;
    int hi = prefs.hi_vel_threshold;

    if(MidiFileEvent_isNoteStartEvent(event))
    {
        int note = MidiFileNoteOnEvent_getNote(event);
        int vel  = MidiFileNoteOnEvent_getVelocity(event);

        if ( (note == prefs.note_eq_filter) ||
             (note <= prefs.note_le_filter &&
              note >= prefs.note_ge_filter &&
              note != prefs.note_ne_filter) )
        {
            if(vel > hi)       vel = hi + ((vel - hi) * prefs.ratio);
            else if(vel < lo)  vel = lo - ((lo - vel) * prefs.ratio);

            assert(vel > 0 && vel <= 127);
            MidiFileNoteOnEvent_setVelocity(event, vel);
        }
    }
}

int main(int argc, char **argv)
{
	MidiFile_t midi_file;

    get_args(argc, argv);

	if ((midi_file = MidiFile_load(prefs.infile)) == NULL)
	{
		printf("Error: Cannot load MIDI file \"%s\".\n", prefs.infile);
		return 1;
	}

    MidiFile_visitEvents(midi_file, visit_event, NULL);

	MidiFile_save(midi_file, prefs.outfile);

	return 0;
}

