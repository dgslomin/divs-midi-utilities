#ifndef READER_INCLUDED
#define READER_INCLUDED

typedef struct Reader *Reader_t;

Reader_t Reader_new(char *filename);
void Reader_free(Reader_t reader);
int Reader_getc(Reader_t reader);
void Reader_ungetc(Reader_t reader, int c);
char *Reader_getFilename(Reader_t reader);
int Reader_getLineNumber(Reader_t reader);

#endif
