
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>
#include <midiutil-expat.h>

int XML_ParseFile(XML_Parser parser, char *filename, char *error_message, size_t error_message_size)
{
	FILE *f;
	void *buffer;
	int bytes_read;

	if (parser == NULL)
	{
		if (error_message != NULL) snprintf(error_message, error_message_size, "Null parser");
		return -1;
	}

	if (filename == NULL)
	{
		if (error_message != NULL) snprintf(error_message, error_message_size, "Null filename");
		return -1;
	}

	if ((f = fopen(filename, "rb")) == NULL)
	{
		if (error_message != NULL) snprintf(error_message, error_message_size, "Cannot open \"%s\"", filename);
		return -1;
	}

	do
	{
		buffer = XML_GetBuffer(parser, 1024);
		bytes_read = fread(buffer, 1, 1024, f);
		XML_ParseBuffer(parser, bytes_read, (bytes_read == 0));
	}
	while (bytes_read > 0);

	fclose(f);

	if (XML_GetErrorCode(parser) == XML_ERROR_NONE) return 0;
	if (error_message != NULL) snprintf(error_message, error_message_size, "\"%s\" at line %d of \"%s\"", XML_ErrorString(XML_GetErrorCode(parser)), (int)(XML_GetCurrentLineNumber(parser)), filename);
	return -1;
}

