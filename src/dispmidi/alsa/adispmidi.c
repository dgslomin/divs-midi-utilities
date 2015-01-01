
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s [ --connect <client name or number> <port number> ]\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	char *connect_client = NULL;
	int connect_port = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--connect") == 0)
		{
			if (++i == argc) usage(argv[0]);
			connect_client = argv[i];
			if (++i == argc) usage(argv[0]);
			connect_port = atoi(argv[i]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	{
		snd_seq_t *seq;
		int port;

		if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT, 0) < 0)
		{
			fprintf(stderr, "Cannot open the ALSA sequencer.\n");
			exit(1);
		}

		snd_seq_set_client_name(seq, "Dispmidi");
		port = snd_seq_create_simple_port(seq, "Dispmidi input port", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

		if ((connect_client != NULL) && (connect_port >= 0))
		{
			int connect_client_id = -1;

			{
				snd_seq_client_info_t *client_info;
				snd_seq_client_info_malloc(&client_info);

				while (snd_seq_query_next_client(seq, client_info) == 0)
				{
					if (strcmp(snd_seq_client_info_get_name(client_info), connect_client) == 0)
					{
						connect_client_id = snd_seq_client_info_get_client(client_info);
						break;
					}
				}

				snd_seq_client_info_free(client_info);
			}

			if (connect_client_id < 0) connect_client_id = atoi(connect_client);
			snd_seq_connect_from(seq, port, connect_client_id, connect_port);
		}

		setbuf(stdout, NULL);

		{
			snd_seq_event_t *event;

			while (snd_seq_event_input(seq, &event) >= 0)
			{
				switch (event->type)
				{
					case SND_SEQ_EVENT_NOTEOFF:
					{
						printf("%d\tNote Off\t\t%d\t%d\n", event->data.note.channel, event->data.note.note, event->data.note.velocity);
						break;
					}
					case SND_SEQ_EVENT_NOTEON:
					{
						printf("%d\tNote On\t\t\t%d\t%d\n", event->data.note.channel, event->data.note.note, event->data.note.velocity);
						break;
					}
					case SND_SEQ_EVENT_KEYPRESS:
					{
						printf("%d\tAftertouch (key)\t\t%d\t%d\n", event->data.note.channel, event->data.note.note, event->data.note.velocity);
						break;
					}
					case SND_SEQ_EVENT_CONTROLLER:
					{
						printf("%d\tController\t\t%d\t%d\n", event->data.control.channel, event->data.control.param, event->data.control.value);
						break;
					}
					case SND_SEQ_EVENT_PGMCHANGE:
					{
						printf("%d\tProgram\t\t%d\n", event->data.control.channel, event->data.control.value);
						break;
					}
					case SND_SEQ_EVENT_CHANPRESS:
					{
						printf("%d\tAftertouch (channel)\t%d\n", event->data.control.channel, event->data.control.value);
						break;
					}
					case SND_SEQ_EVENT_PITCHBEND:
					{
						printf("%d\tPitch Bend\t\t%x\n", event->data.control.channel, event->data.control.value);
						break;
					}
					default:
					{
						/* I'm ignoring the pseudoevents which ALSA provides as convenience features (SND_SEQ_EVENT_NOTE, SND_SEQ_EVENT_CONTROL14, SND_SEQ_EVENT_NONREGPARAM, and SND_SEQ_EVENT_REGPARAM).  Hopefully I can find some way to convince ALSA to normalize them into true MIDI events. */
						break;
					}
				}
			}
		}
	}

	return 0;
}

