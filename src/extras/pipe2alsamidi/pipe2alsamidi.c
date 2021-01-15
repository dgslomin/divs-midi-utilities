
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE 4096 /* just like pipes in the Linux kernel */

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s [ --fifo <fifo> ] [ --name <client name> ] [ --connect <client name or number to which to connect> <port number> ]\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	FILE *input_stream = stdin;
	char *client_name = "pipe2alsamidi";
	char *connect_client = NULL;
	int connect_port = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--fifo") == 0)
		{
			if (++i == argc) usage(argv[0]);
			input_stream = fopen(argv[i], "rb");

			if (input_stream == NULL)
			{
				fprintf(stderr, "Error: Cannot open \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--name") == 0)
		{
			if (++i == argc) usage(argv[0]);
			client_name = argv[i];
		}
		else if (strcmp(argv[i], "--connect") == 0)
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

	if (input_stream != NULL)
	{
		snd_seq_t *seq;
		int port;
		snd_midi_event_t *midi_event_parser;

		if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0)
		{
			fprintf(stderr, "Cannot open the ALSA sequencer.\n");
			exit(1);
		}

		snd_seq_set_client_name(seq, client_name);
		port = snd_seq_create_simple_port(seq, "from pipe", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

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
			snd_seq_connect_to(seq, port, connect_client_id, connect_port);
		}

		snd_midi_event_new(BUFFER_SIZE, &midi_event_parser);

		{
			unsigned char buffer[BUFFER_SIZE];
			snd_seq_event_t *event;
			int bytes_read;

			while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, input_stream)) > 0)
			{
				for (i = 0; i < bytes_read; i++)
				{
					if (snd_midi_event_encode_byte(midi_event_parser, buffer[i], event) == 1)
					{
						snd_seq_event_output_direct(seq, event);
					}
				}
			}
		}

		snd_midi_event_free(midi_event_parser);
		snd_seq_delete_simple_port(seq, port);
		snd_seq_close(seq);
	}

	if (input_stream != stdin) fclose(input_stream);
	return 0;
}

