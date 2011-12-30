
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int main(int argc, char **argv)
{
	int i;
	int midi_out_number = MIDI_MAPPER;
	int port = 13949;

	HMIDIOUT midi_out;
	WORD winsock_version = MAKEWORD(1, 1);
	WSADATA winsock_data;
	SOCKET server_socket;
	struct sockaddr_in server_address;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &midi_out_number);
		}
		else if (strcmp(argv[i], "--port") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &port);
		}
		else
		{
			printf("Usage: %s [--out <n>] [--port <n>]\n", argv[0]);
			return 1;
		}
	}

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.\n", midi_out_number);
		return 1;
	}

	if (WSAStartup(winsock_version, &winsock_data) != 0)
	{
		printf("Cannot initialize sockets.\n"); 
		return 1;
	}

	if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Cannot create a socket.\n");
		return 1;
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons((short)(port));
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_socket, (struct sockaddr *)(&server_address), sizeof(server_address)) != 0)
	{
		printf("Cannot listen on port %d.\n", port);
		return 1;
	}

	if (listen(server_socket, SOMAXCONN) != 0)
	{
		printf("Cannot listen on port %d.\n", port);
		return 1;
	}

	while (1)
	{
		SOCKET socket_to_client;
		char one = 1;

		if ((socket_to_client = accept(server_socket, NULL, NULL)) == INVALID_SOCKET)
		{
			printf("Cannot accept client connection.\n");
			return 1;
		}

		setsockopt(socket_to_client, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

		while (1)
		{
			union
			{
				BYTE bData[4];
				DWORD dwData;
			}
			u;

			if (recv(socket_to_client, &(u.bData[0]), 1, 0) != 1) break;

			switch (u.bData[0] & 0xF0)
			{
				case 0x80:
				case 0x90:
				case 0xA0:
				case 0xB0:
				case 0xE0:
				{
					if (recv(socket_to_client, &(u.bData[1]), 1, 0) != 1) goto label1;
					if (recv(socket_to_client, &(u.bData[2]), 1, 0) != 1) goto label1;
					u.bData[3] = 0;
					midiOutShortMsg(midi_out, u.dwData);
					break;
				}
				case 0xC0:
				case 0xD0:
				{
					if (recv(socket_to_client, &(u.bData[1]), 1, 0) != 1) goto label1;
					u.bData[2] = 0;
					u.bData[3] = 0;
					midiOutShortMsg(midi_out, u.dwData);
					break;
				}
				default:
				{
					/*
					 * Ignore all of the following:
					 *
					 *    system common messages (including sysex)
					 *    realtime messages
					 *    messages with running status
					 *    message fragments
					 */
					break;
				}
			}
		}

		label1:

		closesocket(socket_to_client);
	}

	return 0;
}

