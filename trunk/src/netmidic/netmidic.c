
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

SOCKET socket_to_server;
HMIDIIN midi_in;

void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD param2)
{
	if (msg_type == MIM_DATA)
	{
		union
		{
			DWORD dwData;
			BYTE bData[4];
		}
		u;

		u.dwData = midi_msg;

		switch (u.bData[0] & 0xF0)
		{
			case 0x80:
			case 0x90:
			case 0xA0:
			case 0xB0:
			case 0xE0:
			{
				send(socket_to_server, u.bData, 3, 0);
				break;
			}
			case 0xC0:
			case 0xD0:
			{
				send(socket_to_server, u.bData, 2, 0);
				break;
			}
			default:
			{
				/* Ignore everything else*/
				break;
			}
		}
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	midiInStop(midi_in);
	midiInClose(midi_in);
	closesocket(socket_to_server);
	return FALSE;
}

int main(int argc, char **argv)
{
	int i;
	int midi_in_number = 0;
	char *hostname = "localhost";
	int port = 13949;

	WORD winsock_version = MAKEWORD(1, 1);
	WSADATA winsock_data;
	struct sockaddr_in server_address;
	struct hostent *host;
	char one = 1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &midi_in_number);
		}
		else if (strcmp(argv[i], "--hostname") == 0)
		{
			if (++i >= argc) break;
			hostname = argv[i];
		}
		else if (strcmp(argv[i], "--port") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &port);
		}
		else
		{
			printf("Usage: %s [--in <n>] [--hostname <name>] [--port <n>]\n", argv[0]);
			return 1;
		}
	}

	if (WSAStartup(winsock_version, &winsock_data) != 0)
	{
		printf("Cannot initialize sockets.\n"); 
		return 1;
	}

	if ((socket_to_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Cannot create a socket.\n");
		return 1;
	}

	if ((host = gethostbyname(hostname)) == NULL)
	{
		printf("Cannot resolve hostname %s.\n", hostname);
		return 1;
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons((short)(port));
	memcpy((char *)(&(server_address.sin_addr)), host->h_addr, host->h_length);

	if (connect(socket_to_server, (struct sockaddr *)(&server_address), sizeof(server_address)) != 0)
	{
		printf("Cannot connect to port %d on %s.\n", port, hostname);
		return 1;
	}

	setsockopt(socket_to_server, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

	if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI input port #%d.\n", midi_in_number);
		return 1;
	}

	SetConsoleCtrlHandler(control_handler, TRUE);

	midiInStart(midi_in);

	Sleep(INFINITE);
	return 0;
}

