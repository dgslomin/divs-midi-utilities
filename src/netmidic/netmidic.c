
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

static RtMidiInPtr midi_in = NULL;
static int socket_to_server;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --in <midi port> --server <hostname> <network port>\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	send(socket_to_server, message, message_size, 0);
}

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	shutdown(socket_to_server, 2);
}

int main(int argc, char **argv)
{
	char *server_hostname = NULL;
	int server_port = -1;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("netmidic", argv[i], "netmidic", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--server") == 0)
		{
			if (++i == argc) usage(argv[0]);
			server_hostname = argv[i];
			if (++i == argc) usage(argv[0]);
			server_port = atoi(argv[i]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (server_hostname == NULL)) usage(argv[0]);

	{
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
	}

	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}

