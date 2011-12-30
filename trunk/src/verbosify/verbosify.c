
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>

#include "midimsg.h"


int main(int argc, char *argv[])
{
   Byte midi_message[3];
   int input_fd, output_fd;
   
   if (argc != 3)
   {
      fprintf(stderr, "\nUsage: %s <input fifo> <output fifo>\n\nPurpose: changes MIDI messages into a canonical form\n\n", argv[0]);
      return EXIT_FAILURE;
   }

   input_fd  = ((strcmp(argv[1], "-") == 0) ? 0 : open(argv[1], O_RDONLY));
   output_fd = ((strcmp(argv[2], "-") == 0) ? 1 : open(argv[2], O_WRONLY));

   while (True)
   {
      midimsgRead(/* fd = */ input_fd, /* message_return = */ midi_message);
      midimsgWrite(/* fd = */ output_fd, /* message = */ midi_message);
   }

   return EXIT_SUCCESS;
}

