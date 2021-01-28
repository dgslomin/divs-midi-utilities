
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>

#include "midimsg.h"

int clamp(int i, int lower, int upper)
{
   if (i < lower) return lower;
   if (i > upper) return upper;
   return i;
}

int main(int argc, char *argv[])
{
   Byte midi_message[3];
   int input_fd, output_fd, message_type, pitch, pitch_wheel_value;
   int interval = 0;
   int does_it_count = 1;
   
   if (argc != 3)
   {
      fprintf(stderr, "\nUsage: %s <input fifo> <output fifo>\n\nPurpose: use the pitch bend wheel as an octave selector\n\n", argv[0]);

      return EXIT_FAILURE;
   }

   input_fd  = ((strcmp(argv[1], "-") == 0) ? 0 : open(argv[1], O_RDONLY));
   output_fd = ((strcmp(argv[2], "-") == 0) ? 1 : open(argv[2], O_WRONLY));

   while (True)
   {
      midimsgRead(/* fd = */ input_fd, /* message_return = */ midi_message);

      message_type = midimsgGetMessageType(midi_message);

      switch(message_type)
      {
         case MIDIMSG_NOTE_ON:
         case MIDIMSG_NOTE_OFF:
            pitch = midimsgGetPitch(midi_message);
            midimsgSetPitch(midi_message, clamp(pitch + interval, 0, 127));
            midimsgWrite(/* fd = */ output_fd, /* message = */ midi_message);
            break;
         case MIDIMSG_PITCH_WHEEL:
            pitch_wheel_value = midimsgGetPitchWheelValue(midi_message);
            if (does_it_count && pitch_wheel_value > 0x3500)
            {
               does_it_count = 0;
               interval += 12;
               break;
            }
            if (does_it_count && pitch_wheel_value < 0x500)
            {
               does_it_count = 0;
               interval -= 12;
               break;
            }
            if (!does_it_count && pitch_wheel_value > 0x1500 && 
               pitch_wheel_value < 0x2500)
            {
               does_it_count = 1;
               break;
            }
            break;
         default:
            midimsgWrite(/* fd = */ output_fd, /* message = */ midi_message);
      }

   }

   return EXIT_SUCCESS;
}

