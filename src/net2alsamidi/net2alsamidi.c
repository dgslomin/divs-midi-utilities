
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
	fprintf(stderr, "Usage: %s --port <port> [ --name <client name> ] [ --connect <client name or number to which to connect> <port number> ]\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	int listen_port = -1;
	char *client_name = "net2alsamidi";
	char *connect_client = NULL;
	int connect_port = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--port") == 0)
		{
			if (++i == argc) usage(argv[0]);
			listen_port = atoi(argv[i]);
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

	if (listen_port > 0)
	{
		snd_seq_t *seq;
		int port;

		if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0)
		{
			fprintf(stderr, "Cannot open the ALSA sequencer.\n");
			exit(1);
		}

		snd_seq_set_client_name(seq, client_name);
		port = snd_seq_create_simple_port(seq, "from NetMIDI client", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

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

		{
			int server_socket;
			struct sockaddr_in server_address;

			if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				fprintf(stderr, "Cannot start a NetMIDI server on port %d.\n", listen_port);
				exit(1);
			}

			server_address.sin_family = AF_INET;
			server_address.sin_port = htons(listen_port);
			server_address.sin_addr.s_addr = INADDR_ANY;

			if (bind(server_socket, (struct sockaddr *)(&server_address), sizeof(server_address)) < 0)
			{
				fprintf(stderr, "Cannot start a NetMIDI server on port %d.\n", listen_port);
				exit(1);
			}

			if (listen(server_socket, 1) < 0)
			{
				fprintf(stderr, "Cannot start a NetMIDI server on port %d.\n", listen_port);
				exit(1);
			}

			while (1)
			{
				int socket_to_client;

				if ((socket_to_client = accept(server_socket, NULL, NULL)) >= 0)
				{
					snd_midi_event_t *midi_event_parser;
					snd_seq_event_t *event;
					unsigned char buffer[BUFFER_SIZE];
					int bytes_read;

					{
						char one = 1;
						setsockopt(socket_to_client, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
					}

					snd_midi_event_new(BUFFER_SIZE, &midi_event_parser);

					while ((bytes_read = recv(socket_to_client, buffer, BUFFER_SIZE, 0)) > 0)
					{
						for (i = 0; i < bytes_read; i++)
						{
							if (snd_midi_event_encode_byte(midi_event_parser, buffer[i], event) == 1)
							{
								snd_seq_event_output_direct(seq, event);
							}
						}
					}

					snd_midi_event_free(midi_event_parser);
					close(socket_to_client);
				}
			}

			close(server_socket);
		}

		snd_seq_delete_simple_port(seq, port);
		snd_seq_close(seq);
	}
	else
	{
		usage(argv[0]);
	}

	return 0;
}

