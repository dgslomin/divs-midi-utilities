
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static int should_shutdown = 0;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --port <network port> --out <midi port>\n", program_name);
	exit(1);
}

static void handle_interrupt(void *user_data)
{
	should_shutdown = 1;
}

int main(int argc, char **argv)
{
	int listen_port = -1;
	RtMidiOutPtr midi_out = NULL;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--port") == 0)
		{
			if (++i == argc) usage(argv[0]);
			listen_port = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("netmidid", argv[i], "netmidid")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((listen_port < 0) || (midi_out == NULL)) usage(argv[0]);

	MidiUtil_setInterruptHandler(handle_interrupt, NULL);

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

		while (!should_shutdown)
		{
			int socket_to_client;

			if ((socket_to_client = accept(server_socket, NULL, NULL)) >= 0)
			{
				int should_disconnect = 0;

				{
					char one = 1;
					setsockopt(socket_to_client, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
				}

				while (!should_shutdown)
				{
					unsigned char message[MIDI_UTIL_MESSAGE_SIZE_SHORT_MESSAGE];
					int message_size;

					if (recv(socket_to_client, &(message[0]), 1, 0) != 1) break;

					if ((message_size = MidiUtilMessage_getSize(message)) > 0)
					{
						if (recv(socket_to_client, &(message[1]), message_size - 1, 0) != message_size - 1) break;
						rtmidi_out_send_message(midi_out, message, message_size);
					}
				}

				shutdown(socket_to_client, 2);
			}
		}

		shutdown(server_socket, 2);
	}

	rtmidi_close_port(midi_out);
	return 0;
}

