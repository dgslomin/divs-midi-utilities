
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <simple-collections.h>
#include <midifile.h>
#include <seqer.h>

typedef struct SeqerDocument *SeqerDocument_t;

struct SeqerApplication
{
	PointerArray_t windows;
	MidiFileDivisionType_t preferred_midi_file_division_type;
	int preferred_midi_file_resolution;
	SeqerWindowType_t preferred_window_type;
	SeqerPianoRollMode_t preferred_piano_roll_mode;
	void (*draw_line_callback)(SeqerWindow_t window, int x1, int y1, int x2, int y2, int color);
	void (*draw_rectangle_callback)(SeqerWindow_t window, int x, int y, int width, int height, int color, int fill_color);
	void (*draw_circle_callback)(SeqerWindow_t window, int x, int y, int diameter, int color, int fill_color);
	void (*measure_string_callback)(SeqerWindow_t window, char *string, int *width_out, int *height_out);
	void (*draw_string_callback)(SeqerWindow_t window, int x, int y, char *string, int color);
	void (*filename_change_callback)(SeqerWindow_t window);
	void (*redraw_callback)(SeqerWindow_t window);
	void (*configure_horizontal_scrollbar_callback)(SeqerWindow_t window, int max, int line, int page, int current);
	void (*configure_vertical_scrollbar_callback)(SeqerWindow_t window, int max, int line, int page, int current);
	void *user_data;
};

struct SeqerWindow
{
	SeqerApplication_t application;
	SeqerDocument_t document;
	void *user_data;
	MidiFileMeasureBeat_t measure_beat;
	int width;
	int height;
	int horizontal_scroll_offset;
	int vertical_scroll_offset;
	SeqerWindowType_t type;

	union
	{
		struct
		{
			SeqerPianoRollMode_t mode;
		}
		piano_roll;

		struct
		{
			int dummy;
		}
		event_list;
	}
	u;
};

struct SeqerDocument
{
	PointerArray_t windows;
	MidiFile_t midi_file;
	String_t filename;
	int modified;
};

SeqerApplication_t SeqerApplication_new(void)
{
	SeqerApplication_t application = (SeqerApplication_t)(malloc(sizeof (struct SeqerApplication)));
	application->windows = PointerArray_new(8);
	application->preferred_midi_file_division_type = MIDI_FILE_DIVISION_TYPE_PPQ;
	application->preferred_midi_file_resolution = 960;
	application->preferred_window_type = SEQER_WINDOW_TYPE_PIANO_ROLL;
	application->preferred_piano_roll_mode = SEQER_PIANO_ROLL_MODE_SINGLE;
	application->draw_line_callback = NULL;
	application->draw_rectangle_callback = NULL;
	application->draw_circle_callback = NULL;
	application->measure_string_callback = NULL;
	application->draw_string_callback = NULL;
	application->filename_change_callback = NULL;
	application->redraw_callback = NULL;
	application->configure_horizontal_scrollbar_callback = NULL;
	application->configure_vertical_scrollbar_callback = NULL;
	application->user_data = NULL;
	return application;
}

void SeqerApplication_free(SeqerApplication_t application)
{
	int window_number;
	for (window_number = SeqerApplication_getNumberOfWindows(application) - 1; window_number >= 0; window_number--) SeqerWindow_free(SeqerApplication_getWindow(application, window_number));
	PointerArray_free(application->windows);
	free(application);
}

int SeqerApplication_getNumberOfWindows(SeqerApplication_t application)
{
	return PointerArray_getSize(application->windows);
}

SeqerWindow_t SeqerApplication_getWindow(SeqerApplication_t application, int window_number)
{
	return (SeqerWindow_t)(PointerArray_get(application->windows, window_number));
}

void SeqerApplication_setPreferredMidiFileDivisionType(SeqerApplication_t application, MidiFileDivisionType_t preferred_midi_file_division_type)
{
	application->preferred_midi_file_division_type = preferred_midi_file_division_type;
}

MidiFileDivisionType_t SeqerApplication_getPreferredMidiFileDivisionType(SeqerApplication_t application)
{
	return application->preferred_midi_file_division_type;
}

void SeqerApplication_setPreferredMidiFileResolution(SeqerApplication_t application, int preferred_midi_file_resolution)
{
	application->preferred_midi_file_resolution = preferred_midi_file_resolution;
}

int SeqerApplication_getPreferredMidiFileResolution(SeqerApplication_t application)
{
	return application->preferred_midi_file_resolution;
}

void SeqerApplication_setPreferredWindowType(SeqerApplication_t application, SeqerWindowType_t preferred_window_type)
{
	application->preferred_window_type = preferred_window_type;
}

SeqerWindowType_t SeqerApplication_getPreferredWindowType(SeqerApplication_t application)
{
	return application->preferred_window_type;
}

void SeqerApplication_setPreferredPianoRollMode(SeqerApplication_t application, SeqerPianoRollMode_t preferred_piano_roll_mode)
{
	application->preferred_piano_roll_mode = preferred_piano_roll_mode;
}

SeqerPianoRollMode_t SeqerApplication_getPreferredPianoRollMode(SeqerApplication_t application)
{
	return application->preferred_piano_roll_mode;
}

void SeqerApplication_setDrawLineCallback(SeqerApplication_t application, void (*draw_line_callback)(SeqerWindow_t window, int x1, int y1, int x2, int y2, int color))
{
	application->draw_line_callback = draw_line_callback;
}

void SeqerApplication_setDrawRectangleCallback(SeqerApplication_t application, void (*draw_rectangle_callback)(SeqerWindow_t window, int x, int y, int width, int height, int color, int fill_color))
{
	application->draw_rectangle_callback = draw_rectangle_callback;
}

void SeqerApplication_setDrawCircleCallback(SeqerApplication_t application, void (*draw_circle_callback)(SeqerWindow_t window, int x, int y, int diameter, int color, int fill_color))
{
	application->draw_circle_callback = draw_circle_callback;
}

void SeqerApplication_setMeasureStringCallback(SeqerApplication_t application, void (*measure_string_callback)(SeqerWindow_t window, char *string, int *width_out, int *height_out))
{
	application->measure_string_callback = measure_string_callback;
}

void SeqerApplication_setDrawStringCallback(SeqerApplication_t application, void (*draw_string_callback)(SeqerWindow_t window, int x, int y, char *string, int color))
{
	application->draw_string_callback = draw_string_callback;
}

void SeqerApplication_setFilenameChangeCallback(SeqerApplication_t application, void (*filename_change_callback)(SeqerWindow_t window))
{
	application->filename_change_callback = filename_change_callback;
}

void SeqerApplication_setRedrawCallback(SeqerApplication_t application, void (*redraw_callback)(SeqerWindow_t window))
{
	application->redraw_callback = redraw_callback;
}

void SeqerApplication_setConfigureHorizontalScrollbarCallback(SeqerApplication_t application, void (*configure_horizontal_scrollbar_callback)(SeqerWindow_t window, int max, int line, int page, int current))
{
	application->configure_horizontal_scrollbar_callback = configure_horizontal_scrollbar_callback;
}

void SeqerApplication_setConfigureVerticalScrollbarCallback(SeqerApplication_t application, void (*configure_vertical_scrollbar_callback)(SeqerWindow_t window, int max, int line, int page, int current))
{
	application->configure_vertical_scrollbar_callback = configure_vertical_scrollbar_callback;
}

void SeqerApplication_setUserData(SeqerApplication_t application, void *user_data)
{
	application->user_data = user_data;
}

void *SeqerApplication_getUserData(SeqerApplication_t application)
{
	return application->user_data;
}

SeqerWindow_t SeqerWindow_new(SeqerApplication_t application, SeqerWindow_t base_window)
{
	SeqerWindow_t window = (SeqerWindow_t)(malloc(sizeof (struct SeqerWindow)));
	window->application = application;
	PointerArray_add(application->windows, window);

	if (base_window == NULL)
	{
		window->document = (SeqerDocument_t)(malloc(sizeof (struct SeqerDocument)));
		window->document->windows = PointerArray_new(8);
		window->document->midi_file = MidiFile_new(1, application->preferred_midi_file_division_type, application->preferred_midi_file_resolution);
		window->document->filename = String_new(32);
		window->document->modified = 0;
		PointerArray_add(window->document->windows, window);
		window->user_data = NULL;
		window->measure_beat = MidiFileMeasureBeat_new();
		window->width = 0;
		window->height = 0;
		window->horizontal_scroll_offset = 0;
		window->vertical_scroll_offset = 0;
		window->type = application->preferred_window_type;

		switch (window->type)
		{
			case SEQER_WINDOW_TYPE_PIANO_ROLL:
			{
				window->u.piano_roll.mode = application->preferred_piano_roll_mode;
				break;
			}
			case SEQER_WINDOW_TYPE_EVENT_LIST:
			{
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else
	{
		window->document = base_window->document;
		PointerArray_add(window->document->windows, window);
		window->user_data = base_window->user_data;
		window->measure_beat = MidiFileMeasureBeat_new();
		window->width = base_window->width;
		window->height = base_window->height;
		window->horizontal_scroll_offset = base_window->horizontal_scroll_offset;
		window->vertical_scroll_offset = base_window->vertical_scroll_offset;
		window->type = base_window->type;

		switch (window->type)
		{
			case SEQER_WINDOW_TYPE_PIANO_ROLL:
			{
				window->u.piano_roll.mode = base_window->u.piano_roll.mode;
				break;
			}
			case SEQER_WINDOW_TYPE_EVENT_LIST:
			{
				break;
			}
			default:
			{
				break;
			}
		}
	}

	return window;
}

void SeqerWindow_free(SeqerWindow_t window)
{
	int window_number;

	for (window_number = SeqerApplication_getNumberOfWindows(window->application) - 1; window_number >= 0; window_number--)
	{
		if (SeqerApplication_getWindow(window->application, window_number) == window)
		{
			PointerArray_remove(window->application->windows, window_number);
			break;
		}
	}

	for (window_number = PointerArray_getSize(window->document->windows) - 1; window_number >= 0; window_number--)
	{
		if ((SeqerWindow_t)(PointerArray_get(window->document->windows, window_number)) == window)
		{
			PointerArray_remove(window->document->windows, window_number);
			break;
		}
	}

	if (PointerArray_getSize(window->document->windows) == 0)
	{
		MidiFile_free(window->document->midi_file);
		PointerArray_free(window->document->windows);
		free(window->document);
	}

	MidiFileMeasureBeat_free(window->measure_beat);
	free(window);
}

SeqerApplication_t SeqerWindow_getApplication(SeqerWindow_t window)
{
	return window->application;
}

int SeqerWindow_shouldPromptForSave(SeqerWindow_t window)
{
	return ((PointerArray_getSize(window->document->windows) == 1) && window->document->modified);
}

int SeqerWindow_shouldPromptForFilename(SeqerWindow_t window)
{
	return (SeqerWindow_getFilename(window)[0] == '\0');
}

void SeqerWindow_newMidiFile(SeqerWindow_t window)
{
	if (PointerArray_getSize(window->document->windows) == 1)
	{
		MidiFile_free(window->document->midi_file);
		String_clear(window->document->filename);
	}
	else
	{
		int window_number;

		for (window_number = PointerArray_getSize(window->document->windows) - 1; window_number >= 0; window_number--)
		{
			if ((SeqerWindow_t)(PointerArray_get(window->document->windows, window_number)) == window)
			{
				PointerArray_remove(window->document->windows, window_number);
				break;
			}
		}

		window->document = (SeqerDocument_t)(malloc(sizeof (struct SeqerDocument)));
		window->document->windows = PointerArray_new(8);
		window->document->filename = String_new(32);
		PointerArray_add(window->document->windows, window);
	}

	window->document->midi_file = MidiFile_new(1, window->application->preferred_midi_file_division_type, window->application->preferred_midi_file_resolution);
	window->document->modified = 0;
	window->horizontal_scroll_offset = 0;
	(*(window->application->redraw_callback))(window);
	(*(window->application->configure_horizontal_scrollbar_callback))(window, 0, 0, window->width, window->horizontal_scroll_offset);
	(*(window->application->configure_vertical_scrollbar_callback))(window, 128 * 5, 5, window->height, window->vertical_scroll_offset);
	(*(window->application->filename_change_callback))(window);
}

int SeqerWindow_loadMidiFile(SeqerWindow_t window, char *filename)
{
	MidiFile_t midi_file = MidiFile_load(filename);
	if (midi_file == NULL) return -1;

	if (PointerArray_getSize(window->document->windows) == 1)
	{
		MidiFile_free(window->document->midi_file);
	}
	else
	{
		int window_number;

		for (window_number = PointerArray_getSize(window->document->windows) - 1; window_number >= 0; window_number--)
		{
			if ((SeqerWindow_t)(PointerArray_get(window->document->windows, window_number)) == window)
			{
				PointerArray_remove(window->document->windows, window_number);
				break;
			}
		}

		window->document = (SeqerDocument_t)(malloc(sizeof (struct SeqerDocument)));
		window->document->windows = PointerArray_new(8);
		window->document->filename = String_new(32);
		PointerArray_add(window->document->windows, window);
	}

	window->document->midi_file = midi_file;
	String_set(window->document->filename, (unsigned char *)(filename));
	window->document->modified = 0;
	window->horizontal_scroll_offset = 0;
	(*(window->application->redraw_callback))(window);

	{
		float last_beat = 0.0;
		MidiFileEvent_t last_event = MidiFile_getLastEvent(window->document->midi_file);
		if (last_event != NULL) last_beat = MidiFile_getBeatFromTick(window->document->midi_file, MidiFileEvent_getTick(last_event));
		(*(window->application->configure_horizontal_scrollbar_callback))(window, (int)(last_beat * 20), 20, window->width, window->horizontal_scroll_offset);
	}

	(*(window->application->configure_vertical_scrollbar_callback))(window, 128 * 5, 5, window->height, window->vertical_scroll_offset);
	(*(window->application->filename_change_callback))(window);
	return 0;
}

int SeqerWindow_saveMidiFile(SeqerWindow_t window, char *filename)
{
	if ((filename == NULL) || (filename[0] == '\0'))
	{
		filename = (char *)(String_get(window->document->filename));

		if (filename[0] == '\0')
		{
			return -1;
		}
		else
		{
			return MidiFile_save(window->document->midi_file, filename);
		}
	}
	else
	{
		if (MidiFile_save(window->document->midi_file, filename) < 0)
		{
			return -1;
		}
		else
		{
			int window_number;

			String_set(window->document->filename, (unsigned char *)(filename));

			for (window_number = PointerArray_getSize(window->document->windows) - 1; window_number >= 0; window_number--)
			{
				(*(window->application->filename_change_callback))((SeqerWindow_t)(PointerArray_get(window->document->windows, window_number)));
			}

			return 0;
		}
	}
}

char *SeqerWindow_getFilename(SeqerWindow_t window)
{
	return (char *)(String_get(window->document->filename));
}

int SeqerWindow_getModified(SeqerWindow_t window)
{
	return window->document->modified;
}

MidiFile_t SeqerWindow_getMidiFile(SeqerWindow_t window)
{
	return window->document->midi_file;
}

void SeqerWindow_setSize(SeqerWindow_t window, int width, int height)
{
	window->width = width;
	window->height = height;

	{
		float last_beat = 0.0;
		MidiFileEvent_t last_event = MidiFile_getLastEvent(window->document->midi_file);
		if (last_event != NULL) last_beat = MidiFile_getBeatFromTick(window->document->midi_file, MidiFileEvent_getTick(last_event));
		(*(window->application->configure_horizontal_scrollbar_callback))(window, (int)(last_beat * 20), 20, window->width, window->horizontal_scroll_offset);
	}

	(*(window->application->configure_vertical_scrollbar_callback))(window, 128 * 5, 5, window->height, window->vertical_scroll_offset);
}

int SeqerWindow_getWidth(SeqerWindow_t window)
{
	return window->width;
}

int SeqerWindow_getHeight(SeqerWindow_t window)
{
	return window->height;
}

void SeqerWindow_scrollHorizontal(SeqerWindow_t window, int position)
{
	window->horizontal_scroll_offset = position;
	(*(window->application->redraw_callback))(window);
}

void SeqerWindow_scrollVertical(SeqerWindow_t window, int position)
{
	window->vertical_scroll_offset = position;
	(*(window->application->redraw_callback))(window);
}

static int _grayscale_to_rgb(int grayscale)
{
	return (grayscale << 16) | (grayscale << 8) | grayscale;
}

static int _gradient_rgb(int min_rgb, int max_rgb, int min_value, int max_value, int value)
{
	int min_r = (min_rgb >> 16) & 0xFF;
	int min_g = (min_rgb >> 8) & 0xFF;
	int min_b = min_rgb & 0xFF;
	int max_r = (max_rgb >> 16) & 0xFF;
	int max_g = (max_rgb >> 8) & 0xFF;
	int max_b = max_rgb & 0xFF;
	int r = ((value - min_value) * (max_r - min_r) / (max_value - min_value)) + min_r;
	int g = ((value - min_value) * (max_g - min_g) / (max_value - min_value)) + min_g;
	int b = ((value - min_value) * (max_b - min_b) / (max_value - min_value)) + min_b;
	return (r << 16) | (g << 8) | b;
}

static int _min(int a, int b)
{
	return (a < b) ? a : b;
}

static int _max(int a, int b)
{
	return (a > b) ? a : b;
}

void SeqerWindow_paint(SeqerWindow_t window)
{
	switch (window->type)
	{
		case SEQER_WINDOW_TYPE_PIANO_ROLL:
		{
			(*(window->application->draw_rectangle_callback))(window, 0, 0, window->width, window->height, 0xFFFFFF, 0xFFFFFF);

			{
				int beat;

				for (beat = 0; beat * 20 - window->horizontal_scroll_offset < window->width; beat++)
				{
					MidiFile_setMeasureBeatFromTick(window->document->midi_file, MidiFile_getTickFromBeat(window->document->midi_file, beat), window->measure_beat);

					if ((int)(MidiFileMeasureBeat_getBeat(window->measure_beat)) == 1)
					{
						(*(window->application->draw_line_callback))(window, (beat * 20) - window->horizontal_scroll_offset, 0, (beat * 20) - window->horizontal_scroll_offset, window->height, 0xEEEEEE);
					}
					else
					{
						(*(window->application->draw_line_callback))(window, (beat * 20) - window->horizontal_scroll_offset, 0, (beat * 20) - window->horizontal_scroll_offset, window->height, 0xF7F7F7);
					}
				}
			}

			{
				int note;

				for (note = 0; note < 128; note++)
				{
					int black_note[] = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
					if (black_note[note % 12]) (*(window->application->draw_rectangle_callback))(window, 0, (128 * 5) - (note * 5) - 2 - window->vertical_scroll_offset, window->width, 4, 0xF7F7F7, 0xF7F7F7);
					(*(window->application->draw_line_callback))(window, 0, (128 * 5) - (note * 5) - 3 - window->vertical_scroll_offset, window->width, (128 * 5) - (note * 5) - 3 - window->vertical_scroll_offset, 0xEEEEEE);
				}
			}

			{
				MidiFileEvent_t event;

				for (event = MidiFile_getFirstEvent(window->document->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
				{
					float beat = MidiFile_getBeatFromTick(window->document->midi_file, MidiFileEvent_getTick(event));
					if ((int)(beat * 20) - window->horizontal_scroll_offset > window->width) break;

					if (MidiFileEvent_isNoteStartEvent(event))
					{
						int note = MidiFileNoteStartEvent_getNote(event);
						int velocity = MidiFileNoteStartEvent_getVelocity(event);
						float end_beat = MidiFile_getBeatFromTick(window->document->midi_file, MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(event)));
						int track_number = MidiFileTrack_getNumber(MidiFileEvent_getTrack(event));
						int track_colors[] = { 0xFF0000, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF, 0x7F0000, 0x7F7F00, 0x007F00, 0x007F7F, 0x00007F, 0x7F007F };
						int fill_color = _gradient_rgb(0xFFFFFF, track_colors[(track_number + 11) % 12], 0, 128, velocity);
						int line_color = _gradient_rgb(0x000000, fill_color, 0, 10, 9);
						(*(window->application->draw_rectangle_callback))(window, (int)(beat * 20) - window->horizontal_scroll_offset, (128 * 5) - (note * 5) - 2 - window->vertical_scroll_offset, _max((int)((end_beat - beat) * 20), 3), 4, line_color, fill_color);
					}
				}
			}

			break;
		}
		case SEQER_WINDOW_TYPE_EVENT_LIST:
		{
			/* TODO */
			break;
		}
		default:
		{
			break;
		}
	}
}

void SeqerWindow_mouseDown(SeqerWindow_t window, int button_number, int x, int y)
{
	/* TODO */
}

void SeqerWindow_mouseUp(SeqerWindow_t window, int button_number, int x, int y)
{
	/* TODO */
}

void SeqerWindow_mouseMove(SeqerWindow_t window, int x, int y)
{
	/* TODO */
}

void SeqerWindow_setUserData(SeqerWindow_t window, void *user_data)
{
	window->user_data = user_data;
}

void *SeqerWindow_getUserData(SeqerWindow_t window)
{
	return window->user_data;
}

SeqerWindowType_t SeqerWindow_getType(SeqerWindow_t window)
{
	return window->type;
}

void SeqerWindow_setType(SeqerWindow_t window, SeqerWindowType_t type)
{
	/* TODO: don't forget cleanup */
	window->type = type;
}

void SeqerPianoRollWindow_setMode(SeqerWindow_t window, SeqerPianoRollMode_t mode)
{
	window->u.piano_roll.mode = mode;
}

SeqerPianoRollMode_t SeqerPianoRollWindow_getMode(SeqerWindow_t window)
{
	return window->u.piano_roll.mode;
}

