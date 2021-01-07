#ifndef MIDIUTIL_EXPAT_INCLUDED
#define MIDIUTIL_EXPAT_INCLUDED

/* Common helpers that work with expat. */

#include <expat.h>

#ifdef __cplusplus
extern "C"
{
#endif

int XML_ParseFile(XML_Parser parser, char *filename);

#ifdef __cplusplus
}
#endif

#endif
