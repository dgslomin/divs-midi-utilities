
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

#include "keynames.h"

struct keyboard;

HWND edit_control;
int number_of_keyboards = 0;
int *keyboard_handles = NULL;

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
					if (((raw_input->data.keyboard.Message == WM_KEYDOWN) || (raw_input->data.keyboard.Message == WM_SYSKEYDOWN)) && (raw_input->header.hDevice != 0))
					{
						int keyboard_number;
						int keyboard_handle = (int)(raw_input->header.hDevice);
						int old_style_scan_code = raw_input->data.keyboard.MakeCode;
						int is_an_extended_key = raw_input->data.keyboard.Flags & RI_KEY_E0;
						int scan_code = old_style_scan_code + (is_an_extended_key ? 256 : 0);
						char *key_name = GetKeyNameByScanCode(scan_code);

						for (keyboard_number = 0; keyboard_number < number_of_keyboards; keyboard_number++)
						{
							if (keyboard_handles[keyboard_number] == keyboard_handle)
							{
								break;
							}
						}

						edit_control_printf("<key keyboard-number=\"%d\" keyboard-handle=\"%d\" name=\"%s\" scan-code=\"%d\" />\r\n", keyboard_number, keyboard_handle, key_name, scan_code);

						/* Allow caps, number, and scroll lock to be used as a trigger keys without affecting the lock state. */

						if (raw_input->data.keyboard.VKey == VK_CAPITAL)
						{
							keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_CAPITAL, 0, 0, 0);
							keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
						}
						else if (raw_input->data.keyboard.VKey == VK_NUMLOCK)
						{
							keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_NUMLOCK, 0, 0, 0);
							keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
						}
						else if (raw_input->data.keyboard.VKey == VK_SCROLL)
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
	HWND window;

	{
		WNDCLASS window_class;

		window_class.lpszClassName = "input-tester";
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

	window = CreateWindowEx(WS_EX_CLIENTEDGE, "input-tester", "Input Tester", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance, NULL);
	edit_control = CreateWindow("EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, window, NULL, instance, NULL);
	SendMessage(edit_control, WM_SETFONT, (WPARAM)(GetStockObject(ANSI_VAR_FONT)), (LPARAM)(TRUE));
	ShowWindow(window, show_state);

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
					number_of_keyboards++;
					keyboard_handles = realloc(keyboard_handles, number_of_keyboards * sizeof(int));
					keyboard_handles[number_of_keyboards - 1] = (int)(raw_input_device_list[raw_input_device_number].hDevice);
				}
			}
		}

		free(raw_input_device_list);
	}

	{
		RAWINPUTDEVICE raw_input_device;

		raw_input_device.usUsagePage = 1;
		raw_input_device.usUsage = 6;
		raw_input_device.dwFlags = RIDEV_NOLEGACY;
		raw_input_device.hwndTarget = window;
		RegisterRawInputDevices(&raw_input_device, 1, sizeof(RAWINPUTDEVICE));
	}

	{
		MSG message;

		while (GetMessage(&message, NULL, 0, 0))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	return 0;
}

