
#include <midiutil-sockets.h>

void MidiUtilSockets_init(void)
{
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
}

void MidiUtilSockets_free(void)
{
#ifdef _WIN32
	WSACleanup();
#endif
}

