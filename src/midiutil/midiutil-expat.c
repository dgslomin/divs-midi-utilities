
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>
#include <midiutil-expat.h>

int XML_ParseFile(XML_Parser parser, char *filename)
{
	FILE *f;
	void *buffer;
	int bytes_read;

	if ((parser == NULL) || (filename == NULL)) return -1;
	if ((f = fopen(filename, "rb")) == NULL) return -1;

	do
	{
		buffer = XML_GetBuffer(parser, 1024);
		bytes_read = fread(buffer, 1, 1024, f);
		XML_ParseBuffer(parser, bytes_read, (bytes_read == 0));
	}
	while (bytes_read > 0);

	fclose(f);
	return 0;
}

