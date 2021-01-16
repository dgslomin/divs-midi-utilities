#ifndef MIDIUTIL_EXPAT_INCLUDED
#define MIDIUTIL_EXPAT_INCLUDED

/* Common helpers that work with expat. */

#include <expat.h>

#ifdef __cplusplus
extern "C"
{
#endif

int XML_ParseFile(XML_Parser parser, char *filename, char *error_message, size_t error_message_size);

#ifdef __cplusplus
}
#endif

#endif
