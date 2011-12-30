
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>

#include "midimsg.h"

int main(int argc, char *argv[])
{
   Byte midi_message[3];
   int input_fd, output_fd, i, intervals[16], message_type, pitch;

   if (argc < 4)
   {
      fprintf(stderr, "\nUsage: %s <input_fifo> <output_fifo> <interval 1> "
         "<interval 2> ...\n\n", argv[0]);
      fprintf(stderr, "Purpose: filters MIDI messages from the input fifo to "
         "the output fifo\n");
      fprintf(stderr, "         (use - for stdin or stdout), adding notes at "
         "the specified intervals\n");
      fprintf(stderr, "         from those received.\n\n");

      return EXIT_FAILURE;
   }

   input_fd  = ((strcmp(argv[1], "-") == 0) ? 0 : open(argv[1], O_RDONLY));
   output_fd = ((strcmp(argv[2], "-") == 0) ? 1 : open(argv[2], O_WRONLY));

   for (i = 0; i < argc - 3; i++)
   {
      sscanf(argv[i + 3], "%d", &(intervals[i]));
   }

   while (True)
   {
      midimsgRead(/* fd = */ input_fd, /* message_return = */ midi_message);
      midimsgWrite(/* fd = */ output_fd, /* message = */ midi_message);

      message_type = midimsgGetMessageType(midi_message);
      if (message_type == MIDIMSG_NOTE_ON || message_type == MIDIMSG_NOTE_OFF) 
      {
         pitch = midimsgGetPitch(midi_message);

         for (i = 0; i < argc - 3; i++)
         {
            midimsgSetPitch(midi_message, pitch + intervals[i]);
            midimsgWrite(/* fd = */ output_fd, /* message = */ midi_message);
         }
      }
      else
      {
         midimsgWrite(/* fd = */ output_fd, /* message = */ midi_message);
      }
   }
 
   return EXIT_SUCCESS;
}

