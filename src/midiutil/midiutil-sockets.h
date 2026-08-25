#ifndef MIDIUTIL_SOCKETS_INCLUDED
#define MIDIUTIL_SOCKETS_INCLUDED

/* Common helpers that work with sockets. */

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

#ifdef __cplusplus
extern "C"
{
#endif

void MidiUtilSockets_init(void);
void MidiUtilSockets_free(void);

#ifdef __cplusplus
}
#endif

#endif
