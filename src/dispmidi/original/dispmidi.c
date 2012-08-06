
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>

#include "midimsg.h"

int main(int argc, char *argv[])
{
   Byte midi_message[3];
   int input_fd, message_type;
   
   if (argc != 2)
   {
      fprintf(stderr, "\nUsage: %s <input fifo>\n\n", 
         argv[0]);
      fprintf(stderr, "Purpose: diplays MIDI messages from the input fifo\n\n");

      return EXIT_FAILURE;
   }

   input_fd  = ((strcmp(argv[1], "-") == 0) ? 0 : open(argv[1], O_RDONLY));

   while (True)
   {
      midimsgRead(/* fd = */ input_fd, /* message_return = */ midi_message);

      switch(midimsgGetMessageType(midi_message))
      {
         case MIDIMSG_NOTE_OFF:
            printf("%d\tNote Off\t\t%d\t%d\n", 
               midimsgGetChannel(midi_message), 
               midi_message[1],
               midi_message[2]);
            break;
         case MIDIMSG_NOTE_ON:
            printf("%d\tNote On\t\t\t%d\t%d\n",
               midimsgGetChannel(midi_message), 
               midi_message[1],
               midi_message[2]);
            break;
         case MIDIMSG_KEY_PRESSURE:
            printf("%d\tAftertouch (key)\t\t%d\t%d\n", 
               midimsgGetChannel(midi_message), 
               midi_message[1],
               midi_message[2]);
            break;
         case MIDIMSG_PARAMETER:
            printf("%d\tController\t\t%d\t%d\n",
               midimsgGetChannel(midi_message), 
               midi_message[1],
               midi_message[2]);
            break;
         case MIDIMSG_PROGRAM:
            printf("%d\tProgram\t\t%d\n",
               midimsgGetChannel(midi_message), 
               midi_message[1]);
            break;
         case MIDIMSG_CHANNEL_PRESSURE:
            printf("%d\tAftertouch (channel)\t%d\n",
               midimsgGetChannel(midi_message), 
               midi_message[1]);
            break;
         case MIDIMSG_PITCH_WHEEL:
            printf("%d\tPitch Bend\t\t%x\n",
               midimsgGetChannel(midi_message),
               midimsgGetPitchWheelValue(midi_message));
            break;
      }
   }

   return EXIT_SUCCESS;
}

