
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/fcntl.h>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE 4096 /* just like pipes in the Linux kernel */

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --server <hostname> <port> [ --name <client name> ] [ --connect <client name or number to which to connect> <port number> ]\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	char *server_hostname = NULL;
	int server_port = -1;
	char *client_name = "alsamidi2net";
	char *connect_client = NULL;
	int connect_port = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--server") == 0)
		{
			if (++i == argc) usage(argv[0]);
			server_hostname = argv[i];
			if (++i == argc) usage(argv[0]);
			server_port = atoi(argv[i]);
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

	if ((server_hostname != NULL) && (server_port > 0))
	{
		snd_seq_t *seq;
		int port;
		snd_midi_event_t *midi_event_parser;

		if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT, 0) < 0)
		{
			fprintf(stderr, "Cannot open the ALSA sequencer.\n");
			exit(1);
		}

		snd_seq_set_client_name(seq, client_name);
		port = snd_seq_create_simple_port(seq, "to NetMIDI server", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

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

		snd_midi_event_new(BUFFER_SIZE, &midi_event_parser);
		snd_midi_event_no_status(midi_event_parser, 1);

		{
			int socket_to_server;
			struct hostent *server_host;
			struct sockaddr_in server_address;

			if ((socket_to_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				fprintf(stderr, "Cannot connect to NetMIDI server on %s port %d.\n", server_hostname, server_port);
				exit(1);
			}

			if ((server_host = gethostbyname(server_hostname)) == NULL)
			{
				fprintf(stderr, "Cannot connect to NetMIDI server on %s port %d.\n", server_hostname, server_port);
				exit(1);
			}

			server_address.sin_family = AF_INET;
			server_address.sin_port = htons(server_port);
			memcpy(&(server_address.sin_addr.s_addr), server_host->h_addr_list[0], server_host->h_length);

			if (connect(socket_to_server, (struct sockaddr *)(&server_address), sizeof(server_address)) < 0)
			{
				fprintf(stderr, "Cannot connect to NetMIDI server on %s port %d.\n", server_hostname, server_port);
				exit(1);
			}

			{
				char one = 1;
				setsockopt(socket_to_server, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
			}

			{
				unsigned char buffer[BUFFER_SIZE];
				snd_seq_event_t *event;

				while (snd_seq_event_input(seq, &event) >= 0)
				{
					int bytes_decoded = (int)(snd_midi_event_decode(midi_event_parser, buffer, BUFFER_SIZE, event));
					send(socket_to_server, buffer, bytes_decoded, 0);
				}
			}

			close(socket_to_server);
		}

		snd_midi_event_free(midi_event_parser);
		snd_seq_delete_simple_port(seq, port);
		snd_seq_close(seq);
	}
	else
	{
		usage(argv[0]);
	}

	return 0;
}

