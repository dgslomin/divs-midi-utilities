
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>

#ifndef byte
#define byte char
#endif

#ifndef True
#define True 1
#endif

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
   int r_fd = -1, w_fd = -1, rw_fd, bytes_read;
   fd_set read_fds;
   byte buffer[BUFFER_SIZE];

   if (argc != 4)
   {
      fprintf(stderr, "\nUsage: %s <device file> <input fifo> "
         "<output fifo>\n\n", argv[0]);
      fprintf(stderr, "Purpose: opens the device in rw mode, and attaches it "
         "to the specified fifos\n");
      fprintf(stderr, "         (use - for stdin or stdout).\n\n");

      return EXIT_FAILURE;
   }

   rw_fd = open(argv[1], O_RDWR);

   /*
   ** This odd looking set of open()'s and fcntl()'s is needed to allow the 
   ** input and output to be the same fifo, for loopback purposes.
   */

   if (strcmp(argv[2], "-") == 0) r_fd = STDIN_FILENO;
   if (strcmp(argv[3], "-") == 0) w_fd = STDOUT_FILENO;

   while ((r_fd < 0) || (w_fd < 0))
   {
      if (r_fd < 0) r_fd = open(argv[2], O_RDONLY | O_NONBLOCK);
      if (w_fd < 0) w_fd = open(argv[3], O_WRONLY | O_NONBLOCK);
   }

   if (r_fd != STDIN_FILENO)  
      fcntl(r_fd, F_SETFL, fcntl(r_fd, F_GETFL) & ~O_NONBLOCK);
   if (w_fd != STDOUT_FILENO) 
      fcntl(w_fd, F_SETFL, fcntl(w_fd, F_GETFL) & ~O_NONBLOCK);

   while (True)
   {
      FD_ZERO(&read_fds);
      FD_SET(r_fd,  &read_fds);
      FD_SET(rw_fd, &read_fds);
      select(((r_fd > rw_fd) ? r_fd : rw_fd) + 1, &read_fds, NULL, NULL, NULL);

      if (FD_ISSET(rw_fd, &read_fds))
      {
         bytes_read = read(rw_fd, buffer, BUFFER_SIZE);
         write(w_fd, buffer, bytes_read);
      }

      if (FD_ISSET(r_fd, &read_fds))
      {
         bytes_read = read(r_fd, buffer, BUFFER_SIZE);
         write(rw_fd, buffer, bytes_read);
      }
   }

   return EXIT_SUCCESS;
}

