
/*
 * Brainstorm - a dictatation machine for MIDI
 * Copyright 2000 David G. Slomin
 * Brainstorm is Free Software provided under terms of the BSD license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "midimsg.h"

struct midi_event
{
	struct timeval timestamp;
	Byte message[3];
	struct midi_event *next;
};
typedef struct midi_event midi_event_t;

const char file_header[] = 
{
	'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 0, 0, 1, 0, 120,
	'M', 'T', 'r', 'k', 0, 0, 0, 0
};

const char file_trailer[] =
{
	0, 0xff, 0x2f, 0
};

static char *prefix;
static int timeout;
static midi_event_t event_list_head;
static midi_event_t *last_event;

void generate_filename(char *prefix, char *output_buffer)
{
	time_t current_time;
	struct tm *current_time_struct;
	char current_time_string[1024];

	current_time = time(NULL);
	current_time_struct = localtime(&current_time);
	strftime(current_time_string, 1024, "%Y.%m.%d.%H.%M.%S", current_time_struct);

	sprintf(output_buffer, "%s.%s.mid", prefix, current_time_string);
}

void write_variable_length_quantity(long value, int fd)
{
	Byte buffer[4];
	int offset = 3;

	while (1)
	{
		buffer[offset] = value & 0x7f;
		if (offset < 3) buffer[offset] |= 0x80;
		value >>= 7;
		if ((value == 0) || (offset == 0)) break;
		offset--;
	}

	write(fd, buffer + offset, 4 - offset);
}

void write_four_byte_int(long value, int fd)
{
	Byte buffer[4];

	buffer[0] = (Byte)(value >> 24);
	buffer[1] = (Byte)((value >> 16) & 0xff);
	buffer[2] = (Byte)((value >> 8) & 0xff);
	buffer[3] = (Byte)(value & 0xff);

	write(fd, buffer, 4);
}

void alarm_handler(int signum)
{
	char output_filename[1024];
	int output_fd;
	midi_event_t *current_event, *next_event;
	long last_time_sec, last_time_usec, delta_time;
	off_t ending_offset;

	if (event_list_head.next != NULL)
	{
		generate_filename(prefix, output_filename);
		output_fd = creat(output_filename, 0666);
		write(output_fd, file_header, sizeof(file_header));

		last_time_sec = event_list_head.next->timestamp.tv_sec;
		last_time_usec = event_list_head.next->timestamp.tv_usec;

		for (current_event = event_list_head.next; current_event != NULL; current_event = next_event)
		{
			delta_time = (((((current_event->timestamp.tv_sec - last_time_sec) * 1000000) + (current_event->timestamp.tv_usec - last_time_usec)) / 1000) * 240) / 1000;
			last_time_sec = current_event->timestamp.tv_sec;
			last_time_usec = current_event->timestamp.tv_usec;

			write_variable_length_quantity(delta_time, output_fd);
			midimsgWrite(output_fd, current_event->message);

			next_event = current_event->next;
			free(current_event);
		}

		write(output_fd, file_trailer, sizeof(file_trailer));

		ending_offset = lseek(output_fd, 0, SEEK_CUR);
		lseek(output_fd, 18, SEEK_SET);
		write_four_byte_int(ending_offset - sizeof(file_header), output_fd);

		close(output_fd);

		last_event = &event_list_head;
		last_event->next = NULL;
	}

	alarm(timeout);
}

int main(int argc, char *argv[])
{
	int input_fd;
	midi_event_t *new_event;

	/* Begin startup. */

	if (argc != 4)
	{
		fprintf(stderr, "\nUsage: %s <input fifo> <prefix> <timeout>\n", argv[0]);
		fprintf(stderr, "Purpose: records MIDI data in the background\n\n");

		return 1;
	}

	input_fd = ((strcmp(argv[1], "-") == 0) ? 0 : open(argv[1], O_RDONLY));
	prefix = argv[2];
	sscanf(argv[3], "%d", &timeout);

	signal(SIGALRM, alarm_handler);

	last_event = &event_list_head;
	last_event->next = NULL;
	alarm(timeout);

	/* End startup. */

	/* Begin record loop. */

	while (1)
	{
		new_event = malloc(sizeof(midi_event_t));

		midimsgRead(input_fd, new_event->message);
		gettimeofday(&(new_event->timestamp), NULL);
		new_event->next = NULL;

		alarm(timeout);

		/*
		 * The following two lines have to be atomic, but since we just reset
		 * the alarm, we don't have to worry about locks.  Cute, huh?
		 */

		last_event->next = new_event;
		last_event = new_event;
	}

	/* End record loop. */

	return 0;
}

