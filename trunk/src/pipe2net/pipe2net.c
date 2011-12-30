
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/fcntl.h>

#define BUFFER_SIZE 4096 /* just like pipes in the Linux kernel */

void usage(char *program_name)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: %s <fifo> <hostname> <port>\n", program_name);
	fprintf(stderr, "If <fifo> is \"-\" or omitted, stdin is used.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Purpose: copies from a pipe to a TCP connection\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	char *output_hostname;
	char *output_port_str;
	int output_port;
	int input_fd;
	struct hostent *output_host;
	struct sockaddr_in output_address;
	int output_socket;
	int bytes_read;
	unsigned char buffer[BUFFER_SIZE];

	switch (argc)
	{
		case 3:
			output_hostname = argv[1];
			output_port_str = argv[2];
			break;
		case 4:
			output_hostname = argv[2];
			output_port_str = argv[3];
			break;
		default:
			usage(argv[0]);
			exit(1);
	}

	if (sscanf(output_port_str, "%d", &output_port) < 1)
	{
		usage(argv[0]);
		exit(1);
	}

	if ((argc == 3) || (strcmp(argv[1], "-") == 0))
	{
		input_fd = 0;
	}
	else
	{
		if ((input_fd = open(argv[1], O_RDONLY)) < 0)
		{
			fprintf(stderr, "%s fatal error: Could not open %s.\n", argv[0], argv[1]);
			exit(1);
		}
	}

	if ((output_host = gethostbyname(output_hostname)) == NULL)
	{
		fprintf(stderr, "%s fatal error: Unknown host %s.\n", argv[0], output_hostname);
		exit(1);
	}

	if ((output_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "%s fatal error: Could not create socket.\n", argv[0]);
	}

	output_address.sin_family = AF_INET;
	output_address.sin_port = htons(output_port);
	memcpy(&(output_address.sin_addr.s_addr), output_host->h_addr_list[0], output_host->h_length);

	if (connect(output_socket, (struct sockaddr *)(&output_address), sizeof(output_address)) < 0)
	{
		fprintf(stderr, "%s fatal error: Could not connect to %s:%d.\n", argv[0], output_hostname, output_port);
		exit(1);
	}

	while ((bytes_read = read(input_fd, buffer, BUFFER_SIZE)) > 0)
	{
		write(output_socket, buffer, bytes_read);
	}

	close(input_fd);
	close(output_socket);

	return 0;
}

