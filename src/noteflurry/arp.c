
/* arp mode is a classic arpeggiator which extends its loop length based on how many notes are being played */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>
#include "noteflurry.h"
#include "arp.h"

void arp_init(void)
{
}

void arp_cleanup(void)
{
}

void arp_note_on(long current_time_msecs, int channel, int note, int velocity)
{
}

void arp_note_off(long current_time_msecs, int channel, int note, int velocity)
{
}

void arp_update_players(long current_time_msecs)
{
	long next_beat_time_msecs = last_beat_time_msecs + 60000 / tempo_bpm;

	if (next_beat_time_msecs <= current_time_msecs)
	{
		
		last_beat_time_msecs = next_beat_time_msecs;
	}
}

