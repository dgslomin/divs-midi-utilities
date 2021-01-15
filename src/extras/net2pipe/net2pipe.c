
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>

#define BUFFER_SIZE 4096 /* just like pipes in the Linux kernel */

void usage(char *program_name)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: %s <port> <fifo>\n", program_name);
	fprintf(stderr, "If <fifo> is \"-\" or omitted, stdout is used.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Purpose: copies from a TCP connection to a pipe\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	int server_port;
	int server_socket;
	struct sockaddr_in server_address;
	int input_socket;
	int output_fd;
	int bytes_read;
	unsigned char buffer[BUFFER_SIZE];

	if ((argc != 2) && (argc != 3))
	{
		usage(argv[0]);
		exit(1);
	}

	if (sscanf(argv[1], "%d", &server_port) < 1)
	{
		usage(argv[0]);
		exit(1);
	}


	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "%s fatal error: Could not create socket.\n", argv[0]);
		exit(1);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	server_address.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_socket, (struct sockaddr *)(&server_address), sizeof(server_address)) < 0)
	{
		fprintf(stderr, "%s fatal error: Could not bind to port %d.\n", argv[0], server_port);
		exit(1);
	}

	if (listen(server_socket, 1) < 0)
	{
		fprintf(stderr, "%s fatal error: Could not listen on port %d.\n", argv[0], server_port);
		exit(1);
	}

	if ((input_socket = accept(server_socket, NULL, NULL)) < 0)
	{
		fprintf(stderr, "%s fatal error: Could not accept connections on port %d.\n", argv[0], server_port);
		exit(1);
	}

	if ((argc == 2) || (strcmp(argv[2], "-") == 0))
	{
		output_fd = 1;
	}
	else
	{
		if ((output_fd = open(argv[2], O_WRONLY)) < 0)
		{
			fprintf(stderr, "%s fatal error: Could not open %s.\n", argv[0], argv[2]);
		}
	}

	while ((bytes_read = read(input_socket, buffer, BUFFER_SIZE)) > 0)
	{
		write(output_fd, buffer, bytes_read);
	}

	close(input_socket);
	close(output_fd);

	return 0;
}

