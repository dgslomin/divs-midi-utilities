
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>

#include "midimsg.h"


int main(int argc, char *argv[])
{
   Byte midi_message[3];
   int input_fd, output_fd, interval, message_type, pitch;
   
   if (argc != 4)
   {
      fprintf(stderr, "\nUsage: %s <input fifo> <output fifo> <interval>\n\n", 
         argv[0]);
      fprintf(stderr, "Purpose: filters MIDI messages from the input fifo to "
         "the output fifo\n");
      fprintf(stderr, "         (use - for stdin or stdout), transposing "
         "notes by the specified\n");
      fprintf(stderr, "         interval from those received.\n\n");

      return EXIT_FAILURE;
   }

   input_fd  = ((strcmp(argv[1], "-") == 0) ? 0 : open(argv[1], O_RDONLY));
   output_fd = ((strcmp(argv[2], "-") == 0) ? 1 : open(argv[2], O_WRONLY));
   sscanf(argv[3], "%d", &interval);

   while (True)
   {
      midimsgRead(/* fd = */ input_fd, /* message_return = */ midi_message);

      message_type = midimsgGetMessageType(midi_message);
      if (message_type == MIDIMSG_NOTE_ON || message_type == MIDIMSG_NOTE_OFF) 
      {
         pitch = midimsgGetPitch(midi_message);
         midimsgSetPitch(midi_message, pitch + interval);
      }
      midimsgWrite(/* fd = */ output_fd, /* message = */ midi_message);
   }

   return EXIT_SUCCESS;
}

