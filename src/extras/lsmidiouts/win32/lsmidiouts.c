
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int i, max;

	for (i = 0, max = midiOutGetNumDevs(); i < max; i++)
	{
		MIDIOUTCAPS midi_out_caps;
		midiOutGetDevCaps(i, &midi_out_caps, sizeof(midi_out_caps));
		printf("%3d %s\n", i, midi_out_caps.szPname);
	}

	return 0;
}

