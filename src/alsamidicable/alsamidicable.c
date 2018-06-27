
/*
 * Why not just use aconnect?  This lets you refer to ports by name, and will
 * wait for them to be created if the application which provides them hasn't
 * started yet.  Also an arguably cleaner interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

typedef enum
{
	MODE_DEFAULT,
	MODE_LIST_PORTS,
	MODE_LIST_CONNECTIONS,
	MODE_CONNECT,
	MODE_DISCONNECT
}
Mode_t;

void list_ports(void)
{
	snd_seq_t *sequencer;
	snd_seq_client_info_t *client_info;
	snd_seq_port_info_t *port_info;

	snd_seq_open(&sequencer, "default", SND_SEQ_OPEN_DUPLEX, 0);
	snd_seq_client_info_malloc(&client_info);
	snd_seq_port_info_malloc(&port_info);

	while (snd_seq_query_next_client(sequencer, client_info) >= 0)
	{
		snd_seq_port_info_set_client(port_info, snd_seq_client_info_get_client(client_info));
		snd_seq_port_info_set_port(port_info, -1);

		while (snd_seq_query_next_port(sequencer, port_info) >= 0)
		{
			if (snd_seq_port_info_get_capability(port_info) & SND_SEQ_PORT_CAP_SUBS_WRITE)
			{
				printf("--to \"%s\" \"%s\"\n", snd_seq_client_info_get_name(client_info), snd_seq_port_info_get_name(port_info));
			}
		}

		snd_seq_port_info_set_port(port_info, -1);

		while (snd_seq_query_next_port(sequencer, port_info) >= 0)
		{
			if (snd_seq_port_info_get_capability(port_info) & SND_SEQ_PORT_CAP_SUBS_READ)
			{
				printf("--from \"%s\" \"%s\"\n", snd_seq_client_info_get_name(client_info), snd_seq_port_info_get_name(port_info));
			}
		}
	}

	snd_seq_port_info_free(port_info);
	snd_seq_client_info_free(client_info);
	snd_seq_close(sequencer);
}

void list_connections(void)
{
	snd_seq_t *sequencer;
	snd_seq_client_info_t *client_info;
	snd_seq_client_info_t *connected_client_info;
	snd_seq_port_info_t *port_info;
	snd_seq_port_info_t *connected_port_info;
	snd_seq_query_subscribe_t *subscriptions;

	snd_seq_open(&sequencer, "default", SND_SEQ_OPEN_DUPLEX, 0);
	snd_seq_client_info_malloc(&client_info);
	snd_seq_client_info_malloc(&connected_client_info);
	snd_seq_port_info_malloc(&port_info);
	snd_seq_port_info_malloc(&connected_port_info);
	snd_seq_query_subscribe_malloc(&subscriptions);

	while (snd_seq_query_next_client(sequencer, client_info) >= 0)
	{
		snd_seq_port_info_set_client(port_info, snd_seq_client_info_get_client(client_info));
		snd_seq_port_info_set_port(port_info, -1);

		while (snd_seq_query_next_port(sequencer, port_info) >= 0)
		{
			if (snd_seq_port_info_get_capability(port_info) & SND_SEQ_PORT_CAP_SUBS_READ)
			{
				snd_seq_query_subscribe_set_root(subscriptions, snd_seq_port_info_get_addr(port_info));
				snd_seq_query_subscribe_set_type(subscriptions, SND_SEQ_QUERY_SUBS_READ);
				snd_seq_query_subscribe_set_index(subscriptions, 0);

				while (snd_seq_query_port_subscribers(sequencer, subscriptions) >= 0)
				{
					snd_seq_addr_t *connected_port_address = (snd_seq_addr_t *)(snd_seq_query_subscribe_get_addr(subscriptions));
					snd_seq_get_any_port_info(sequencer, connected_port_address->client, connected_port_address->port, connected_port_info);
					snd_seq_get_any_client_info(sequencer, snd_seq_port_info_get_client(connected_port_info), connected_client_info);
					printf("--from \"%s\" \"%s\" --to \"%s\" \"%s\"\n", snd_seq_client_info_get_name(client_info), snd_seq_port_info_get_name(port_info), snd_seq_client_info_get_name(connected_client_info), snd_seq_port_info_get_name(connected_port_info));
					snd_seq_query_subscribe_set_index(subscriptions, snd_seq_query_subscribe_get_index(subscriptions) + 1);
				}
			}
		}
	}

	snd_seq_query_subscribe_free(subscriptions);
	snd_seq_port_info_free(connected_port_info);
	snd_seq_port_info_free(port_info);
	snd_seq_client_info_free(connected_client_info);
	snd_seq_client_info_free(client_info);
	snd_seq_close(sequencer);
}

snd_seq_addr_t *get_port_address(snd_seq_t *sequencer, snd_seq_client_info_t *client_info, snd_seq_port_info_t *port_info, char *client_name, char *port_name, unsigned int capability)
{
	while (snd_seq_query_next_client(sequencer, client_info) >= 0)
	{
		if (strcmp(snd_seq_client_info_get_name(client_info), client_name) == 0) {
			snd_seq_port_info_set_client(port_info, snd_seq_client_info_get_client(client_info));
			snd_seq_port_info_set_port(port_info, -1);

			while (snd_seq_query_next_port(sequencer, port_info) >= 0)
			{
				if ((snd_seq_port_info_get_capability(port_info) & capability) && (strcmp(snd_seq_port_info_get_name(port_info), port_name) == 0))
				{
					return (snd_seq_addr_t *)(snd_seq_port_info_get_addr(port_info));
				}
			}
		}
	}

	return NULL;
}

snd_seq_addr_t *wait_for_port_address(snd_seq_t *sequencer, snd_seq_client_info_t *client_info, snd_seq_port_info_t *port_info, char *client_name, char *port_name, unsigned int capability)
{
	snd_seq_addr_t *port_address;

	while ((port_address = get_port_address(sequencer, client_info, port_info, client_name, port_name, capability)) == NULL)
	{
		usleep(250000);
	}

	return port_address;
}

void connect_or_disconnect(Mode_t mode, char *from_client_name, char *from_port_name, char *to_client_name, char *to_port_name)
{
	snd_seq_t *sequencer;
	snd_seq_client_info_t *client_info;
	snd_seq_port_info_t *port_info;
	snd_seq_port_subscribe_t *subscription;

	snd_seq_open(&sequencer, "default", SND_SEQ_OPEN_DUPLEX, 0);
	snd_seq_client_info_malloc(&client_info);
	snd_seq_port_info_malloc(&port_info);
	snd_seq_port_subscribe_malloc(&subscription);

	snd_seq_port_subscribe_set_sender(subscription, wait_for_port_address(sequencer, client_info, port_info, from_client_name, from_port_name, SND_SEQ_PORT_CAP_SUBS_READ));
	snd_seq_port_subscribe_set_dest(subscription, wait_for_port_address(sequencer, client_info, port_info, to_client_name, to_port_name, SND_SEQ_PORT_CAP_SUBS_WRITE));

	switch (mode)
	{
		case MODE_CONNECT:
		{
			snd_seq_subscribe_port(sequencer, subscription);
			break;
		}
		case MODE_DISCONNECT:
		{
			snd_seq_unsubscribe_port(sequencer, subscription);
			break;
		}
		default:
		{
			break;
		}
	}

	snd_seq_port_subscribe_free(subscription);
	snd_seq_port_info_free(port_info);
	snd_seq_client_info_free(client_info);
	snd_seq_close(sequencer);
}

void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --list-ports\n", program_name);
	fprintf(stderr, "Usage: %s --list-connections\n", program_name);
	fprintf(stderr, "Usage: %s --connect --from <client> <port> --to <client> <port>\n", program_name);
	fprintf(stderr, "Usage: %s --disconnect --from <client> <port> --to <client> <port>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	Mode_t mode = MODE_DEFAULT;
	char *from_client_name = NULL;
	char *from_port_name = NULL;
	char *to_client_name = NULL;
	char *to_port_name = NULL;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--list-ports") == 0)
		{
			mode = MODE_LIST_PORTS;
		}
		else if (strcmp(argv[i], "--list-connections") == 0)
		{
			mode = MODE_LIST_CONNECTIONS;
		}
		else if (strcmp(argv[i], "--connect") == 0)
		{
			mode = MODE_CONNECT;
		}
		else if (strcmp(argv[i], "--disconnect") == 0)
		{
			mode = MODE_DISCONNECT;
		}
		else if (strcmp(argv[i], "--from") == 0)
		{
			if (++i == argc) usage(argv[0]);
			from_client_name = argv[i];
			if (++i == argc) usage(argv[0]);
			from_port_name = argv[i];
		}
		else if (strcmp(argv[i], "--to") == 0)
		{
			if (++i == argc) usage(argv[0]);
			to_client_name = argv[i];
			if (++i == argc) usage(argv[0]);
			to_port_name = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	switch (mode)
	{
		case MODE_LIST_PORTS:
		{
			list_ports();
			break;
		}
		case MODE_LIST_CONNECTIONS:
		{
			list_connections();
			break;
		}
		case MODE_CONNECT:
		case MODE_DISCONNECT:
		{
			if (from_client_name == NULL) usage(argv[0]);
			if (from_port_name == NULL) usage(argv[0]);
			if (to_client_name == NULL) usage(argv[0]);
			if (to_port_name == NULL) usage(argv[0]);
			connect_or_disconnect(mode, from_client_name, from_port_name, to_client_name, to_port_name);
			break;
		}
		default:
		{
			usage(argv[0]);
			break;
		}
	}

	return 0;
}

