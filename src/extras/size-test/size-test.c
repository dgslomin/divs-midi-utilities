
#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

SIZE_T get_memory_usage()
{
	PROCESS_MEMORY_COUNTERS process_memory_counters;
	GetProcessMemoryInfo(GetCurrentProcess(), &process_memory_counters, sizeof (process_memory_counters));
	return process_memory_counters.PagefileUsage;
}

int main(int argc, char **argv)
{
	SIZE_T base_memory_usage = get_memory_usage();
	MidiFile_load(argv[1]);
	printf("midi file data structure used %ld bytes of memory\n", get_memory_usage() - base_memory_usage);
	return 0;
}

