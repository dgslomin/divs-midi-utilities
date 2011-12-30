
/*
 * Imp - the Input Mapping Program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <expat.h>

typedef enum
{
	FUNCTION_NONE,
	FUNCTION_NOTE,
	FUNCTION_EXIT
}
FUNCTION;

typedef struct scan_code_mapping *SCAN_CODE_MAPPING;
typedef struct keyboard *KEYBOARD;
typedef struct engine *ENGINE;

struct scan_code_mapping
{
	FUNCTION function;
	int note;
};

struct keyboard
{
	char *name;
	HANDLE device_handle;
	SCAN_CODE_MAPPING scan_code_mappings[256];
};

struct engine
{
	int number_of_keyboards;
	KEYBOARD *keyboards;
	char *midi_out_name;
	int midi_out_number;
	int channel;
	int velocity;
	HMIDIOUT midi_out;
	int keyboard_to_identify;
	int notes_down[128];
};

HWND window;
HWND edit_control;
ENGINE engine;

static int convert_midi_out_name_to_number(char *name)
{
	int midi_out_number, maximum_midi_out_number;

	for (midi_out_number = 0, maximum_midi_out_number = midiOutGetNumDevs(); midi_out_number < maximum_midi_out_number; midi_out_number++)
	{
		MIDIOUTCAPS midi_out_caps;
		midiOutGetDevCaps(midi_out_number, &midi_out_caps, sizeof(midi_out_caps));
		if (strcmp(midi_out_caps.szPname, name) == 0) return midi_out_number;
	}

	return MIDI_MAPPER;
}

static int convert_key_name_to_scan_code(char *key_name)
{
	static const char *key_names[] = { "esc", "escape", "1", "!", "2", "@", "3", "#", "4", "$", "5", "^", "6", "&", "7", "&", "8", "*", "9", "(", "0", ")", "-", "_", "=", "+", "bksp", "backspace", "tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "{", "]", "}", "enter", "return", "ctrl", "control", "lctrl", "rctrl", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", ":", "\'", "\"", "`", "~", "shift", "lshift", "\\", "|", "z", "x", "c", "v", "b", "n", "m", ",", "<", ".", ">", "/", "?", "rshift", "prtsc", "printscreen", "alt", "lalt", "ralt", "space", "caps", "capslock", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12", "num", "numlock", "scroll", "scrolllock", "home", "keypad7", "up", "keypad8", "pgup", "pageup", "keypad9", "keypad-", "keypadminus", "left", "keypad4", "center", "keypad5", "right", "keypad6", "keypad+", "keypadplus", "end", "keypad1", "down", "keypad2", "pgdn", "pagedown", "keypad3", "ins", "insert", "del", "delete", NULL };
	static const int scan_code_for_each_key_name[] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 26, 27, 27, 28, 28, 29, 29, 29, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 45, 46, 47, 48, 49, 50, 51, 51, 52, 52, 53, 53, 54, 55, 55, 56, 56, 56, 57, 58, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 87, 88, 69, 69, 70, 70, 71, 71, 72, 72, 73, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80, 80, 81, 81, 81, 82, 82, 83, 83 };
	int key_name_number;
	int scan_code = -1;

	for (key_name_number = 0; key_names[key_name_number] != NULL; key_name_number++)
	{
		if (_stricmp(key_name, key_names[key_name_number]) == 0)
		{
			scan_code = scan_code_for_each_key_name[key_name_number];
			break;
		}
	}

	return scan_code;
}

static int convert_note_name_to_number(char *name)
{
	static const char *chromatic_names[] = { "Cb", "C#", "C", "Db", "D#", "D", "Eb", "E#", "E", "Fb", "F#", "F", "Gb", "G#", "G", "Ab", "A#", "A", "Bb", "B#", "B", NULL };
	static const int chromatic_notes[] = { -1, 1, 0, 1, 3, 2, 3, 5, 4, 4, 6, 5, 6, 8, 7, 8, 10, 9, 10, 12, 11 };
	int chromatic_number;

	for (chromatic_number = 0; chromatic_names[chromatic_number] != NULL; chromatic_number++)
	{
		int chromatic_name_length = strlen(chromatic_names[chromatic_number]);

		if (strncmp(name, chromatic_names[chromatic_number], chromatic_name_length) == 0)
		{
			int octave = atoi(name + chromatic_name_length);

			return 12 + (octave * 12) + chromatic_notes[chromatic_number];
		}
	}

	return -1;
}

static void xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **attributes)
{
	if (strcmp((char *)(name), "imp") == 0)
	{
		int attribute_offset;

		for (attribute_offset = 0; attributes[attribute_offset] != NULL; attribute_offset += 2)
		{
			if (strcmp((char *)(attributes[attribute_offset]), "midi-out-name") == 0)
			{
				engine->midi_out_name = strdup((char *)(attributes[attribute_offset + 1]));
				engine->midi_out_number = convert_midi_out_name_to_number(engine->midi_out_name);
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "midi-out-number") == 0)
			{
				engine->midi_out_number = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "channel") == 0)
			{
				engine->channel = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "velocity") == 0)
			{
				engine->velocity = atoi((char *)(attributes[attribute_offset + 1]));
			}
		}
	}
	else if (strcmp((char *)(name), "key") == 0)
	{
		int attribute_offset;
		FUNCTION function = FUNCTION_NONE;
		int scan_code = -1;
		char *keyboard_name = "";
		int note = -1;
		int keyboard_number;
		KEYBOARD keyboard;

		for (attribute_offset = 0; attributes[attribute_offset] != NULL; attribute_offset += 2)
		{
			if (strcmp((char *)(attributes[attribute_offset]), "function") == 0)
			{
				char *function_name = (char *)(attributes[attribute_offset + 1]);

				if (strcmp(function_name, "note") == 0)
				{
					function = FUNCTION_NOTE;
				}
				else if (strcmp(function_name, "exit") == 0)
				{
					function = FUNCTION_EXIT;
				}
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "scan-code") == 0)
			{
				scan_code = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "name") == 0)
			{
				scan_code = convert_key_name_to_scan_code((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "keyboard") == 0)
			{
				keyboard_name = (char *)(attributes[attribute_offset + 1]);
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "note-number") == 0)
			{
				note = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "note-name") == 0)
			{
				note = convert_note_name_to_number((char *)(attributes[attribute_offset + 1]));
			}
		}

		for (keyboard_number = 0; keyboard_number < engine->number_of_keyboards; keyboard_number++)
		{
			if (strcmp(keyboard_name, engine->keyboards[keyboard_number]->name) == 0)
			{
				break;
			}
		}

		if (keyboard_number == engine->number_of_keyboards)
		{
			int scan_code_to_initialize;

			(engine->number_of_keyboards)++;
			engine->keyboards = (KEYBOARD *)(realloc(engine->keyboards, engine->number_of_keyboards * sizeof(KEYBOARD)));
			keyboard = (KEYBOARD)(malloc(sizeof (struct keyboard)));
			keyboard->name = strdup(keyboard_name);
			keyboard->device_handle = 0;

			for (scan_code_to_initialize = 0; scan_code_to_initialize < 256; scan_code_to_initialize++)
			{ 
				SCAN_CODE_MAPPING scan_code_mapping = (SCAN_CODE_MAPPING)(malloc(sizeof (struct scan_code_mapping)));
				scan_code_mapping->function = FUNCTION_NONE;
				keyboard->scan_code_mappings[scan_code_to_initialize] = scan_code_mapping;
			}

			engine->keyboards[keyboard_number] = keyboard;
		}
		else
		{
			keyboard = engine->keyboards[keyboard_number];
		}


		if (scan_code >= 0)
		{
			SCAN_CODE_MAPPING scan_code_mapping = keyboard->scan_code_mappings[scan_code];

			if ((function == FUNCTION_NOTE) && (note >= 0))
			{
				scan_code_mapping->function = FUNCTION_NOTE;
				scan_code_mapping->note = note;
			}
			else if (function == FUNCTION_EXIT)
			{
				scan_code_mapping->function = FUNCTION_EXIT;
			}
		}
	}
}

static void load_engine(char *filename)
{
	XML_Parser xml_parser;
	FILE *in;
	void *buffer;
	int bytes_read;

	if ((in = fopen(filename, "rb")) == NULL) return;

	xml_parser = XML_ParserCreate(NULL);
	XML_SetStartElementHandler(xml_parser, xml_start_element_handler);

	do
	{
		buffer = XML_GetBuffer(xml_parser, 1024);
		bytes_read = fread(buffer, 1, 1024, in);
		XML_ParseBuffer(xml_parser, bytes_read, (bytes_read == 0));
	}
	while (bytes_read > 0);

	XML_ParserFree(xml_parser);

	fclose(in);
}

static void edit_control_printf(char *format, ...)
{
	char buffer[1024];
	int edit_control_text_length;

	va_list args;
	va_start(args, format);
	_vsnprintf(buffer, 1024, format, args);
	edit_control_text_length = SendMessage(edit_control, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(edit_control, EM_SETSEL, (WPARAM)(edit_control_text_length), (LPARAM)(edit_control_text_length));
	SendMessage(edit_control, EM_REPLACESEL, (WPARAM)(FALSE), (LPARAM)(buffer));
	va_end(args);
}

static void connect_raw_input()
{
	RAWINPUTDEVICE raw_input_device;

	raw_input_device.usUsagePage = 1;
	raw_input_device.usUsage = 6;
	raw_input_device.dwFlags = RIDEV_NOLEGACY;
	raw_input_device.hwndTarget = window;
	RegisterRawInputDevices(&raw_input_device, 1, sizeof(RAWINPUTDEVICE));
}

static void disconnect_raw_input()
{
	RAWINPUTDEVICE raw_input_device;

	raw_input_device.usUsagePage = 1;
	raw_input_device.usUsage = 6;
	raw_input_device.dwFlags = RIDEV_REMOVE;
	raw_input_device.hwndTarget = window;
	RegisterRawInputDevices(&raw_input_device, 1, sizeof(RAWINPUTDEVICE));
}

static void send_midi_note_on(HMIDIOUT midi_out, int channel, int pitch, int velocity)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0x90 | (BYTE)(channel);
	u.bData[1] = (BYTE)(pitch);
	u.bData[2] = (BYTE)(velocity);
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

static void send_midi_note_off(HMIDIOUT midi_out, int channel, int pitch, int velocity)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0x80 | (BYTE)(channel);
	u.bData[1] = (BYTE)(pitch);
	u.bData[2] = (BYTE)(velocity);
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

static LRESULT WINAPI window_message_handler(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		case WM_INPUT:
		{
			if (wparam == RIM_INPUT)
			{
				HRAWINPUT raw_input_handle = (HRAWINPUT)(lparam);
				int raw_input_size;
				RAWINPUT *raw_input;

				GetRawInputData(raw_input_handle, RID_INPUT, NULL, &raw_input_size, sizeof(RAWINPUTHEADER));
				raw_input = (RAWINPUT *)(malloc(raw_input_size));
				GetRawInputData(raw_input_handle, RID_INPUT, raw_input, &raw_input_size, sizeof(RAWINPUTHEADER));

				if (raw_input->header.dwType == RIM_TYPEKEYBOARD)
				{
					if (raw_input->data.keyboard.Message == WM_KEYDOWN)
					{
						/* Allow caps, number, and scroll lock to be used as a trigger keys without affecting the lock state. */

						if (raw_input->data.keyboard.MakeCode == 58)
						{
							keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_CAPITAL, 0, 0, 0);
							keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
						}
						else if (raw_input->data.keyboard.MakeCode == 69)
						{
							keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_NUMLOCK, 0, 0, 0);
							keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
						}
						else if (raw_input->data.keyboard.MakeCode == 70)
						{
							keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_SCROLL, 0, 0, 0);
							keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);
						}
					}

					if (engine->keyboard_to_identify < engine->number_of_keyboards)
					{
						if (raw_input->data.keyboard.Message == WM_KEYDOWN)
						{
							engine->keyboards[engine->keyboard_to_identify]->device_handle = raw_input->header.hDevice;
							edit_control_printf(" ok.\r\n\r\n");
							(engine->keyboard_to_identify)++;

							if (engine->keyboard_to_identify < engine->number_of_keyboards)
							{
								edit_control_printf("Please identify the keyboard called \"%s\"...", engine->keyboards[engine->keyboard_to_identify]->name);
							}
							else
							{
								edit_control_printf("Ready to play!\r\n");
							}
						}
					}
					else
					{
						KEYBOARD keyboard = NULL;

						if (engine->number_of_keyboards == 1)
						{
							keyboard = engine->keyboards[0];
						}
						else
						{
							int keyboard_number;

							for (keyboard_number = 0; keyboard_number < engine->number_of_keyboards; keyboard_number++)
							{
								keyboard = engine->keyboards[keyboard_number];

								if (keyboard->device_handle == raw_input->header.hDevice)
								{
									break;
								}
							}

							if (keyboard_number == engine->number_of_keyboards)
							{
								keyboard = NULL;
							}
						}

						if (keyboard != NULL)
						{
							SCAN_CODE_MAPPING scan_code_mapping = keyboard->scan_code_mappings[raw_input->data.keyboard.MakeCode];

							if (scan_code_mapping->function == FUNCTION_NOTE)
							{
								int note = scan_code_mapping->note;

								if (raw_input->data.keyboard.Message == WM_KEYDOWN)
								{
									if (!engine->notes_down[note])
									{
										engine->notes_down[note] = 1;
										send_midi_note_on(engine->midi_out, engine->channel, note, engine->velocity);
									}
								}
								else
								{
									if (engine->notes_down[note])
									{
										engine->notes_down[note] = 0;
										send_midi_note_off(engine->midi_out, engine->channel, note, 0);
									}
								}
							}
							else if ((scan_code_mapping->function == FUNCTION_EXIT) && (raw_input->data.keyboard.Message == WM_KEYDOWN))
							{
								PostQuitMessage(0);
							}
						}
					}
				}

				free(raw_input);
			}

			return DefWindowProc(window, message, wparam, lparam);
		}
		case WM_SETFOCUS:
		{
			SetFocus(edit_control);
			return 0;
		}
		case WM_SIZE:
		{
			MoveWindow(edit_control, 0, 0, LOWORD(lparam), HIWORD(lparam), TRUE);
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		default:
		{
			return DefWindowProc(window, message, wparam, lparam);
		}
	}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show_state)
{
	WNDCLASS window_class;
	int note_to_initialize;
	MSG message;

	window_class.lpszClassName = "imp";
	window_class.lpfnWndProc = window_message_handler;
	window_class.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	window_class.hInstance = instance;
	window_class.hIcon = NULL;
	window_class.hCursor = NULL;
	window_class.hbrBackground = NULL;
	window_class.lpszMenuName = NULL;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	RegisterClass(&window_class);

	window = CreateWindowEx(WS_EX_CLIENTEDGE, "imp", "Imp", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance, NULL);
	edit_control = CreateWindow("EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, window, NULL, instance, NULL);
	SendMessage(edit_control, WM_SETFONT, (WPARAM)(GetStockObject(ANSI_VAR_FONT)), (LPARAM)(TRUE));
	ShowWindow(window, show_state);

	engine = (ENGINE)(malloc(sizeof (struct engine)));
	engine->number_of_keyboards = 0;
	engine->keyboards = NULL;
	engine->midi_out_name = NULL;
	engine->midi_out_number = MIDI_MAPPER;
	engine->channel = 0;
	engine->velocity = 64;
	engine->midi_out = 0;

	for (note_to_initialize = 0; note_to_initialize < 128; note_to_initialize++)
	{
		engine->notes_down[note_to_initialize] = 0;
	}

	if (__argc == 2)
	{
		load_engine(__argv[1]);

		if (midiOutOpen(&(engine->midi_out), engine->midi_out_number, 0, 0, 0) == MMSYSERR_NOERROR)
		{
			if (engine->number_of_keyboards == 0)
			{
				edit_control_printf("An empty mapping is not very useful!\r\n");
				engine->keyboard_to_identify = 0;
			}
			else if (engine->number_of_keyboards == 1)
			{
				edit_control_printf("Ready to play!\r\n");
				engine->keyboard_to_identify = 1;
			}
			else
			{
				edit_control_printf("The mapping refers to more than one keyboard; before you can start playing, you will have to press a key on each one in turn to identify it.\r\n\r\n");
				edit_control_printf("Please identify the keyboard called \"%s\"...", engine->keyboards[0]->name);
				engine->keyboard_to_identify = 0;
			}
		}
		else
		{
			if (engine->midi_out_name == NULL)
			{
				edit_control_printf("Cannot open MIDI output port #%d.\n", engine->midi_out_number);
			}
			else
			{
				edit_control_printf("Cannot open MIDI output port \"%s\".\n", engine->midi_out_name);
			}

			engine->keyboard_to_identify = engine->number_of_keyboards;
		}
	}
	else
	{
		edit_control_printf("Usage:  %s <filename>\r\n", __argv[0]);
	}

	if (engine->number_of_keyboards > 0)
	{
		connect_raw_input();
	}

	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if (engine->number_of_keyboards > 0)
	{
		disconnect_raw_input();
	}

	if (engine->midi_out != 0)
	{
		midiOutClose(engine->midi_out);
	}

	return 0;
}

