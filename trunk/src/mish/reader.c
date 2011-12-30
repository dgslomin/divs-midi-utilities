
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"

struct Reader
{
	FILE *file;
	char *filename;
	int cr_count;
	int lf_count;
};

Reader_t Reader_new(char *filename)
{
	FILE *f;
	Reader_t reader;

	if ((f = fopen(filename, "r")) == NULL) return NULL;
	reader = (Reader_t)(malloc(sizeof(struct Reader)));
	reader->file = f;
	reader->filename = strdup(filename);
	reader->cr_count = 0;
	reader->lf_count = 0;
	return reader;
}

void Reader_free(Reader_t reader)
{
	fclose(reader->file);
	free(reader->filename);
	free(reader);
}

int Reader_getc(Reader_t reader)
{
	int c = fgetc(reader->file);
	if (c == '\r') (reader->cr_count)++;
	if (c == '\n') (reader->lf_count)++;
	return c;
}

void Reader_ungetc(Reader_t reader, int c)
{
	if (c == '\r') (reader->cr_count)--;
	if (c == '\n') (reader->lf_count)--;
	ungetc(c, reader->file);
}

char *Reader_getFilename(Reader_t reader)
{
	return reader->filename;
}

int Reader_getLineNumber(Reader_t reader)
{
	return ((reader->cr_count > reader->lf_count) ? reader->cr_count : reader->lf_count) + 1;
}

