
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int i, max;

	for (i = 0, max = midiInGetNumDevs(); i < max; i++)
	{
		MIDIINCAPS midi_in_caps;
		midiInGetDevCaps(i, &midi_in_caps, sizeof(midi_in_caps));
		printf("%3d %s\n", i, midi_in_caps.szPname);
	}

	return 0;
}

