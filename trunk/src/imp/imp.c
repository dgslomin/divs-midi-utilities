
/*
 * Imp - the Input Mapping Program
 *
 * Distinguishes amongst multiple keyboards plugged into the same computer at the same time (mice and joysticks are planned but not yet implemented), and maps their respective input into MIDI events.  This is an extension of my older programs qwertymidi and joycc.
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
	FUNCTION_EXIT,
	FUNCTION_NOTE,
	FUNCTION_TRANSPOSE,
	FUNCTION_VELOCITY,
	FUNCTION_CONTROLLER,
	FUNCTION_PANIC
}
FUNCTION;

typedef struct key *KEY;
typedef struct keyboard *KEYBOARD;
typedef struct keyboard_alias *KEYBOARD_ALIAS;

struct key
{
	int is_down; /* used to cancel key repeat */
	FUNCTION function;

	union
	{
		struct
		{
			int note;
			int release_note; /* separate to avoid hanging notes when transposing */
			int channel;
		}
		note;

		struct
		{
			int is_absolute;
			int is_momentary;
			int amount;
			int release_amount; /* makes the combination of momentary and absolute work properly */
		}
		transpose;

		struct
		{
			int is_absolute;
			int is_momentary;
			int amount;
			int release_amount;
		}
		velocity;

		struct
		{
			int controller;
			int is_absolute;
			int is_momentary;
			int amount;
			int release_amount;
			int channel;
		}
		controller;
	}
	u;
};

struct keyboard
{
	HANDLE handle;
	KEY keys[512];
};

struct keyboard_alias
{
	char *name;
	int handle;
};

int number_of_keyboards = 0;
KEYBOARD *keyboards = NULL;
int midi_out_number = MIDI_MAPPER;
int default_channel = 0;
int velocity = 64;
int program = -1;
int transposition_amount = 0;
int hotkey_scan_code = -1;
int hotkey_shift = 0;
int hotkey_control = 0;
int hotkey_alt = 0;
int hotkey_win = 0;
int number_of_keyboard_aliases = 0;
KEYBOARD_ALIAS *keyboard_aliases = NULL;
HMIDIOUT midi_out = 0;
int notes_down[128 * 16]; /* avoids early cutoffs when multiple keys are mapped to the same note */
int controller_amounts[128 * 16];
int pitchbend_amounts[16];

static void usage(char *program_name)
{
	char msg[1024];
	sprintf(msg, "%s [ --midi-out-name <str> | --midi-out-number <n> ] [ --channel <n> ] [ --velocity <n> ] [ --program-number <n> ] [ ( --hotkey-name <str> | --hotkey-scan-code <n> ) [ --hotkey-shift (1 | 0) ] [ --hotkey-control (1 | 0) ] [ --hotkey-alt (1 | 0) ] [ --hotkey-win (1 | 0) ] ] [ --keyboard-alias-number <n>:<str> | --keyboard-alias-handle <n>:<str> ] ... [ <filename> ]", program_name);
	MessageBox(NULL, msg, "Imp", MB_OK | MB_ICONINFORMATION);
	exit(1);
}

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

static void send_midi_program(HMIDIOUT midi_out, int channel, int program)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0xC0 | (BYTE)(channel);
	u.bData[1] = (BYTE)(program);
	u.bData[2] = 0;
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

static void send_midi_controller(HMIDIOUT midi_out, int channel, int controller, int amount)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0xB0 | (BYTE)(channel);
	u.bData[1] = (BYTE)(controller);
	u.bData[2] = (BYTE)(amount);
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

static void send_midi_pitchbend(HMIDIOUT midi_out, int channel, int amount)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0xE0 | (BYTE)(channel);
	u.bData[1] = (BYTE)(amount & 0x7F);
	u.bData[2] = (BYTE)(amount >> 7);
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
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
				midi_out_number = convert_midi_out_name_to_number((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "midi-out-number") == 0)
			{
				midi_out_number = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "channel") == 0)
			{
				default_channel = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "velocity") == 0)
			{
				velocity = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "program-number") == 0)
			{
				program = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "hotkey-name") == 0)
			{
				hotkey_scan_code = GetScanCodeByKeyName((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "hotkey-scan-code") == 0)
			{
				hotkey_scan_code = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "hotkey-shift") == 0)
			{
				hotkey_shift = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "hotkey-control") == 0)
			{
				hotkey_control = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "hotkey-alt") == 0)
			{
				hotkey_alt = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "hotkey-win") == 0)
			{
				hotkey_win = atoi((char *)(attributes[attribute_offset + 1]));
			}
		}
	}
	else if (strcmp((char *)(name), "key") == 0)
	{
		int attribute_offset;
		FUNCTION function = FUNCTION_NONE;
		int scan_code = -1;
		int keyboard_handle = -1;
		int note = -1;
		int is_absolute = 0;
		int is_momentary = 0;
		int amount = 0;
		int controller = -1;
		int channel = default_channel;

		for (attribute_offset = 0; attributes[attribute_offset] != NULL; attribute_offset += 2)
		{
			if (strcmp((char *)(attributes[attribute_offset]), "function") == 0)
			{
				char *function_name = (char *)(attributes[attribute_offset + 1]);

				if (strcmp(function_name, "exit") == 0)
				{
					function = FUNCTION_EXIT;
				}
				else if (strcmp(function_name, "note") == 0)
				{
					function = FUNCTION_NOTE;
				}
				else if (strcmp(function_name, "transpose") == 0)
				{
					function = FUNCTION_TRANSPOSE;
				}
				else if (strcmp(function_name, "velocity") == 0)
				{
					function = FUNCTION_VELOCITY;
				}
				else if (strcmp(function_name, "controller") == 0)
				{
					function = FUNCTION_CONTROLLER;
				}
				else if (strcmp(function_name, "panic") == 0)
				{
					function = FUNCTION_PANIC;
				}
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "name") == 0)
			{
				scan_code = GetScanCodeByKeyName((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "scan-code") == 0)
			{
				scan_code = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "keyboard") == 0)
			{
				int keyboard_alias_number;

				for (keyboard_alias_number = 0; keyboard_alias_number < number_of_keyboard_aliases; keyboard_alias_number++)
				{
					if (strcmp(keyboard_aliases[keyboard_alias_number]->name, (char *)(attributes[attribute_offset + 1])) == 0)
					{
						keyboard_handle = keyboard_aliases[keyboard_alias_number]->handle;
						break;
					}
				}
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "keyboard-number") == 0)
			{
				int keyboard_number = atoi((char *)(attributes[attribute_offset + 1]));

				if (keyboard_number < number_of_keyboards)
				{
					keyboard_handle = (int)(keyboards[keyboard_number]->handle);
				}
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "keyboard-handle") == 0)
			{
				keyboard_handle = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "note-name") == 0)
			{
				note = convert_note_name_to_number((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "note-number") == 0)
			{
				note = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "absolute") == 0)
			{
				is_absolute = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "momentary") == 0)
			{
				is_momentary = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "amount") == 0)
			{
				amount = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "controller-number") == 0)
			{
				controller = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "channel") == 0)
			{
				channel = atoi((char *)(attributes[attribute_offset + 1]));
			}
		}

		{
			int keyboard_number;
			KEYBOARD keyboard = NULL;

			for (keyboard_number = 0; keyboard_number < number_of_keyboards; keyboard_number++)
			{
				if (keyboards[keyboard_number]->handle == (HANDLE)(keyboard_handle))
				{
					keyboard = keyboards[keyboard_number];
					break;
				} 
			}

			if (keyboard == NULL)
			{
				MessageBox(NULL, "No such keyboard.", "Imp", MB_OK | MB_ICONERROR);
				exit(1);
			}

			if (scan_code >= 0)
			{
				KEY key = keyboard->keys[scan_code];

				if (function == FUNCTION_EXIT)
				{
					key->function = FUNCTION_EXIT;
				}
				else if ((function == FUNCTION_NOTE) && (note >= 0))
				{
					key->function = FUNCTION_NOTE;
					key->u.note.note = note;
					key->u.note.channel = channel;
				}
				else if (function == FUNCTION_TRANSPOSE)
				{
					key->function = FUNCTION_TRANSPOSE;
					key->u.transpose.is_absolute = is_absolute;
					key->u.transpose.is_momentary = is_momentary;
					key->u.transpose.amount = amount;
				}
				else if (function == FUNCTION_VELOCITY)
				{
					key->function = FUNCTION_VELOCITY;
					key->u.velocity.is_absolute = is_absolute;
					key->u.velocity.is_momentary = is_momentary;
					key->u.velocity.amount = amount;
				}
				else if ((function == FUNCTION_CONTROLLER) && (controller >= 0))
				{
					key->function = FUNCTION_CONTROLLER;
					key->u.controller.controller = controller;
					key->u.controller.is_absolute = is_absolute;
					key->u.controller.is_momentary = is_momentary;
					key->u.controller.amount = amount;
					key->u.controller.channel = channel;
				}
				else if (function == FUNCTION_PANIC)
				{
					key->function = FUNCTION_PANIC;
				}
			}
		}
	}
	else if (strcmp((char *)(name), "init-controller") == 0)
	{
		int attribute_offset;
		int controller = -1;
		int amount = 0;
		int channel = default_channel;

		for (attribute_offset = 0; attributes[attribute_offset] != NULL; attribute_offset += 2)
		{
			if (strcmp((char *)(attributes[attribute_offset]), "number") == 0)
			{
				controller = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "amount") == 0)
			{
				amount = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "channel") == 0)
			{
				channel = atoi((char *)(attributes[attribute_offset + 1]));
			}
		}

		if (controller > 0)
		{
			controller_amounts[(channel * 128) + controller] = amount;
		}
	}
	else if (strcmp((char *)(name), "init-pitchbend") == 0)
	{
		int attribute_offset;
		int amount = 0;
		int channel = default_channel;

		for (attribute_offset = 0; attributes[attribute_offset] != NULL; attribute_offset += 2)
		{
			if (strcmp((char *)(attributes[attribute_offset]), "amount") == 0)
			{
				amount = atoi((char *)(attributes[attribute_offset + 1]));
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "semitones") == 0)
			{
				amount = (int)(atof((char *)(attributes[attribute_offset + 1])) * 4096.0) + 8192;
			}
			else if (strcmp((char *)(attributes[attribute_offset]), "channel") == 0)
			{
				channel = atoi((char *)(attributes[attribute_offset + 1]));
			}
		}

		pitchbend_amounts[channel] = amount;
	}
}

static LRESULT WINAPI window_message_handler(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		case WM_INPUT:
		{
			if (wparam == RIM_INPUT)
			{
				RAWINPUT *raw_input;

				{
					HRAWINPUT raw_input_handle = (HRAWINPUT)(lparam);
					int raw_input_size;

					GetRawInputData(raw_input_handle, RID_INPUT, NULL, &raw_input_size, sizeof(RAWINPUTHEADER));
					raw_input = (RAWINPUT *)(malloc(raw_input_size));
					GetRawInputData(raw_input_handle, RID_INPUT, raw_input, &raw_input_size, sizeof(RAWINPUTHEADER));
				}

				if (raw_input->header.dwType == RIM_TYPEKEYBOARD)
				{
					if (raw_input->header.hDevice != 0) /* Discard synthetic keystrokes like those sent via keybd_event(), below. */
					{
						int old_style_scan_code = raw_input->data.keyboard.MakeCode;
						int is_an_extended_key = raw_input->data.keyboard.Flags & RI_KEY_E0;
						int scan_code = old_style_scan_code + (is_an_extended_key ? 256 : 0);
						int keyboard_number;

						for (keyboard_number = 0; keyboard_number < number_of_keyboards; keyboard_number++)
						{
							KEYBOARD keyboard = keyboards[keyboard_number];

							if ((keyboard->handle == raw_input->header.hDevice) || (keyboard->handle == (HANDLE)(-1)))
							{
								KEY key = keyboard->keys[scan_code];

								if ((raw_input->data.keyboard.Message == WM_KEYDOWN) || (raw_input->data.keyboard.Message == WM_SYSKEYDOWN))
								{
									if (!(key->is_down))
									{
										key->is_down = 1;

										if (key->function == FUNCTION_EXIT)
										{
											PostQuitMessage(0);
										}
										else if (key->function == FUNCTION_NOTE)
										{
											int note = key->u.note.note + transposition_amount;

											while (note > 127) note -= 12;
											while (note < 0) note += 12;

											if (!notes_down[(key->u.note.channel * 128) + note])
											{
												notes_down[(key->u.note.channel * 128) + note] = 1;
												send_midi_note_on(midi_out, key->u.note.channel, note, (velocity > 127) ? 127 : (velocity < 0) ? 0 : velocity);
												key->u.note.release_note = note;
											}
										}
										else if (key->function == FUNCTION_TRANSPOSE)
										{
											if (key->u.transpose.is_absolute)
											{
												if (key->u.transpose.is_momentary)
												{
													key->u.transpose.release_amount = key->u.transpose.amount - transposition_amount;
												}

												transposition_amount = key->u.transpose.amount;
											}
											else
											{
												transposition_amount += key->u.transpose.amount;
											}
										}
										else if (key->function == FUNCTION_VELOCITY)
										{
											if (key->u.velocity.is_absolute)
											{
												if (key->u.velocity.is_momentary)
												{
													key->u.velocity.release_amount = key->u.velocity.amount - velocity;
												}

												velocity = key->u.velocity.amount;
											}
											else
											{
												velocity += key->u.velocity.amount;
											}
										}
										else if (key->function == FUNCTION_CONTROLLER)
										{
											int controller = key->u.controller.controller;

											if (key->u.controller.is_absolute)
											{
												if (key->u.controller.is_momentary)
												{
													key->u.controller.release_amount = key->u.controller.amount - controller_amounts[(key->u.controller.channel * 128) + controller];
												}

												controller_amounts[(key->u.controller.channel * 128) + controller] = key->u.controller.amount;
											}
											else
											{
												controller_amounts[(key->u.controller.channel * 128) + controller] += key->u.controller.amount;
											}

											{
												int amount = controller_amounts[(key->u.controller.channel * 128) + controller];
												send_midi_controller(midi_out, key->u.controller.channel, controller, (amount > 127) ? 127 : (amount < 0) ? 0 : amount);
											}
										}
										else if (key->function == FUNCTION_PANIC)
										{
											int channel;
											int note;

											for (channel = 0; channel < 16; channel++)
											{
												for (note = 0; note < 127; note++)
												{
													send_midi_note_off(midi_out, channel, note, 0);
													notes_down[(channel * 128) + note] = 0;
												}
											}
										}
									}
								}
								else if ((raw_input->data.keyboard.Message == WM_KEYUP) || (raw_input->data.keyboard.Message == WM_SYSKEYUP))
								{
									key->is_down = 0;

									if (key->function == FUNCTION_NOTE)
									{
										int release_note = key->u.note.release_note;

										if (notes_down[(key->u.note.channel * 128) + release_note])
										{
											notes_down[(key->u.note.channel * 128) + release_note] = 0;
											send_midi_note_off(midi_out, key->u.note.channel, release_note, 0);
										}
									}
									else if (key->function == FUNCTION_TRANSPOSE)
									{
										if (key->u.transpose.is_momentary)
										{
											if (key->u.transpose.is_absolute)
											{
												transposition_amount -= key->u.transpose.release_amount;
											}
											else
											{
												transposition_amount -= key->u.transpose.amount;
											}
										}
									}
									else if (key->function == FUNCTION_VELOCITY)
									{
										if (key->u.velocity.is_momentary)
										{
											if (key->u.velocity.is_absolute)
											{
												velocity -= key->u.velocity.release_amount;
											}
											else
											{
												velocity -= key->u.velocity.amount;
											}
										}
									}
									else if (key->function == FUNCTION_CONTROLLER)
									{
										if (key->u.controller.is_momentary)
										{
											int controller = key->u.controller.controller;

											if (key->u.controller.is_absolute)
											{
												controller_amounts[(key->u.controller.channel * 128) + controller] -= key->u.controller.release_amount;
											}
											else
											{
												controller_amounts[(key->u.controller.channel * 128) + controller] -= key->u.controller.amount;
											}

											{
												int amount = controller_amounts[(key->u.controller.channel * 128) + controller];
												send_midi_controller(midi_out, key->u.controller.channel, controller, (amount > 127) ? 127 : (amount < 0) ? 0 : amount);
											}
										}
									}
								}
							}
						}

						/* Allow caps, number, and scroll lock to be used as a trigger keys without affecting the lock state. */

						if ((raw_input->data.keyboard.VKey == VK_CAPITAL) && (raw_input->data.keyboard.Message == WM_KEYDOWN))
						{
							keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_CAPITAL, 0, 0, 0);
							keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
						}
						else if ((raw_input->data.keyboard.VKey == VK_NUMLOCK) && (raw_input->data.keyboard.Message == WM_KEYDOWN))
						{
							keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_NUMLOCK, 0, 0, 0);
							keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
						}
						else if ((raw_input->data.keyboard.VKey == VK_SCROLL) && (raw_input->data.keyboard.Message == WM_KEYDOWN))
						{
							keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_SCROLL, 0, 0, 0);
							keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);
						}
					}
				}

				free(raw_input);
			}

			return DefWindowProc(window, message, wparam, lparam);
		}
		case WM_HOTKEY:
		{
			if (wparam == 0)
			{
				SetForegroundWindow(window);
			}

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
	{
		unsigned int number_of_raw_input_devices;
		RAWINPUTDEVICELIST *raw_input_device_list;

		GetRawInputDeviceList(NULL, &number_of_raw_input_devices, sizeof(RAWINPUTDEVICELIST));
		raw_input_device_list = (RAWINPUTDEVICELIST *)(malloc(sizeof(RAWINPUTDEVICELIST) * number_of_raw_input_devices));
		GetRawInputDeviceList(raw_input_device_list, &number_of_raw_input_devices, sizeof(RAWINPUTDEVICELIST));

		{
			unsigned int raw_input_device_number;

			for (raw_input_device_number = 0; raw_input_device_number < number_of_raw_input_devices; raw_input_device_number++)
			{
				if (raw_input_device_list[raw_input_device_number].dwType == RIM_TYPEKEYBOARD)
				{
					KEYBOARD keyboard = (KEYBOARD)(malloc(sizeof(struct keyboard)));
					keyboard->handle = raw_input_device_list[raw_input_device_number].hDevice;

					{
						int scan_code;

						for (scan_code = 0; scan_code < 512; scan_code++)
						{
							KEY key = (KEY)(malloc(sizeof(struct key)));
							key->is_down = 0;
							key->function = FUNCTION_NONE;
							keyboard->keys[scan_code] = key;
						}
					}

					number_of_keyboards++;
					keyboards = realloc(keyboards, sizeof(KEYBOARD) * number_of_keyboards);
					keyboards[number_of_keyboards - 1] = keyboard;
				}
			}
		}

		free(raw_input_device_list);
	}

	{
		KEYBOARD keyboard = (KEYBOARD)(malloc(sizeof(struct keyboard)));
		keyboard->handle = (HANDLE)(-1);

		{
			int scan_code;

			for (scan_code = 0; scan_code < 512; scan_code++)
			{
				KEY key = (KEY)(malloc(sizeof(struct key)));
				key->is_down = 0;
				key->function = FUNCTION_NONE;
				keyboard->keys[scan_code] = key;
			}
		}

		number_of_keyboards++;
		keyboards = realloc(keyboards, sizeof(KEYBOARD) * number_of_keyboards);
		keyboards[number_of_keyboards - 1] = keyboard;
	}

	{
		int channel;
		int i;

		for (channel = 0; channel < 16; channel++)
		{
			for (i = 0; i < 128; i++)
			{
				notes_down[(channel * 128) + i] = 0;
				controller_amounts[(channel * 128) + i] = -1;
			}

			pitchbend_amounts[channel] = 8192;
		}
	}

	{
		int i;
		char filename_buffer[1024];
		char *filename = NULL;
		XML_Parser xml_parser;
		FILE *in;
		void *buffer;
		int bytes_read;

		for (i = 1; i < __argc; i++)
		{
			if (strcmp(__argv[i], "--help") == 0)
			{
				usage(__argv[0]);
			}
			else if (strcmp(__argv[i], "--midi-out-name") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				midi_out_number = convert_midi_out_name_to_number(__argv[i]);
			}
			else if (strcmp(__argv[i], "--midi-out-number") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				midi_out_number = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--channel") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				default_channel = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--velocity") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				velocity = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--program-number") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				program = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--hotkey-name") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				hotkey_scan_code = GetScanCodeByKeyName(__argv[i]);
			}
			else if (strcmp(__argv[i], "--hotkey-scan-code") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				hotkey_scan_code = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--hotkey-shift") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				hotkey_shift = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--hotkey-control") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				hotkey_control = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--hotkey-alt") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				hotkey_alt = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--hotkey-win") == 0)
			{
				if (++i == __argc) usage(__argv[0]);
				hotkey_win = atoi(__argv[i]);
			}
			else if (strcmp(__argv[i], "--keyboard-alias-number") == 0)
			{
				if (++i == __argc) usage(__argv[0]);

				{
					KEYBOARD_ALIAS keyboard_alias = malloc(sizeof (struct keyboard_alias));
					int keyboard_number;
					if (sscanf(__argv[i], "%[^:]:%d", keyboard_alias->name, &keyboard_number) != 2) usage(__argv[0]);

					if (keyboard_number < number_of_keyboards)
					{
						keyboard_alias->handle = (int)(keyboards[keyboard_number]->handle);
					}
					else
					{
						keyboard_alias->handle = -1;
					}

					number_of_keyboard_aliases++;
					keyboard_aliases = realloc(keyboard_aliases, sizeof(KEYBOARD_ALIAS) * number_of_keyboard_aliases);
					keyboard_aliases[number_of_keyboard_aliases - 1] = keyboard_alias;
				}
			}
			else if (strcmp(__argv[i], "--keyboard-alias-handle") == 0)
			{
				if (++i == __argc) usage(__argv[0]);

				{
					KEYBOARD_ALIAS keyboard_alias = malloc(sizeof (struct keyboard_alias));
					if (sscanf(__argv[i], "%[^:]:%d", keyboard_alias->name, &(keyboard_alias->handle)) != 2) usage(__argv[0]);
					number_of_keyboard_aliases++;
					keyboard_aliases = realloc(keyboard_aliases, sizeof(KEYBOARD_ALIAS) * number_of_keyboard_aliases);
					keyboard_aliases[number_of_keyboard_aliases - 1] = keyboard_alias;
				}
			}
			else
			{
				filename = __argv[i];
			}
		}

		if (filename == NULL)
		{
			OPENFILENAME open_filename;
			open_filename.lStructSize = sizeof (OPENFILENAME);
			open_filename.hwndOwner = NULL;
			open_filename.lpstrFilter = "Imp files\0*.imp\0All files\0*.*\0";
			open_filename.lpstrCustomFilter = NULL;
			open_filename.lpstrFile = filename_buffer;
			open_filename.nMaxFile = 1024;
			open_filename.lpstrFileTitle = NULL;
			open_filename.lpstrInitialDir = NULL;
			open_filename.lpstrTitle = NULL;
			open_filename.Flags = 0;
			open_filename.lpstrDefExt = "imp";
			open_filename.pvReserved = NULL;
			open_filename.dwReserved = 0;
			open_filename.FlagsEx = 0;
			filename_buffer[0] = '\0';

			if (!GetOpenFileName(&open_filename))
			{
				return 1;
			}

			filename = filename_buffer;
		}

		if ((in = fopen(filename, "rb")) == NULL)
		{
			MessageBox(NULL, "Cannot open the specified config file.", "Imp", MB_OK | MB_ICONERROR);
			return 1;
		}

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

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		MessageBox(NULL, "Cannot open MIDI output port.", "Imp", MB_OK | MB_ICONERROR);
		return 1;
	}

	{
		WNDCLASS window_class;

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
	}

	{
		HWND window = CreateWindowEx(WS_EX_WINDOWEDGE, "imp", "Imp", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, -1000, -1000, 0, 0, NULL, NULL, instance, NULL);
		ShowWindow(window, show_state);

		{
			RAWINPUTDEVICE raw_input_device;

			raw_input_device.usUsagePage = 1;
			raw_input_device.usUsage = 6;
			raw_input_device.dwFlags = RIDEV_NOLEGACY;
			raw_input_device.hwndTarget = window;
			RegisterRawInputDevices(&raw_input_device, 1, sizeof(RAWINPUTDEVICE));
		}

		if (hotkey_scan_code >= 0)
		{
			int hotkey_vkey = MapVirtualKey(hotkey_scan_code, 3);

			if (!RegisterHotKey(window, 0, (hotkey_shift ? MOD_SHIFT : 0) | (hotkey_control ? MOD_CONTROL : 0) | (hotkey_alt ? MOD_ALT : 0) | (hotkey_win ? MOD_WIN : 0), hotkey_vkey))
			{
				MessageBox(NULL, "Cannot register hotkey.", "Imp", MB_OK | MB_ICONINFORMATION);
				midiOutClose(midi_out);
				return 0;
			}
		}
	}

	{
		int channel;
		int i;

		for (channel = 0; channel < 16; channel++)
		{
			if (program >= 0)
			{
				send_midi_program(midi_out, channel, program);
			}

			for (i = 0; i < 128; i++)
			{
				if (controller_amounts[(channel * 128) + i] < 0)
				{
					controller_amounts[(channel * 128) + i] = 0;
				}
				else
				{
					send_midi_controller(midi_out, channel, i, controller_amounts[(channel * 128) + i]);
				}
			}

			send_midi_pitchbend(midi_out, channel, pitchbend_amounts[channel]);
		}
	}

	{
		MSG message;

		while (GetMessage(&message, NULL, 0, 0))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	midiOutClose(midi_out);
	return 0;
}

