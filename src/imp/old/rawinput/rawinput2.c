
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

HWND edit_control;

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
				HRAWINPUT raw_input_handle = (HRAWINPUT)(lparam);
				int raw_input_size;
				RAWINPUT *raw_input;
				int raw_input_device_name_size;
				char *raw_input_device_name;

				GetRawInputData(raw_input_handle, RID_INPUT, NULL, &raw_input_size, sizeof(RAWINPUTHEADER));
				raw_input = (RAWINPUT *)(malloc(raw_input_size));
				GetRawInputData(raw_input_handle, RID_INPUT, raw_input, &raw_input_size, sizeof(RAWINPUTHEADER));

				GetRawInputDeviceInfo(raw_input->header.hDevice, RIDI_DEVICENAME, NULL, &raw_input_device_name_size);
				raw_input_device_name = malloc(raw_input_device_name_size);
				GetRawInputDeviceInfo(raw_input->header.hDevice, RIDI_DEVICENAME, raw_input_device_name, &raw_input_device_name_size);

				if (raw_input->header.dwType == RIM_TYPEKEYBOARD)
				{
					edit_control_printf("dev = %s, scancode = %d, vkey = %d, message = %d\r\n", raw_input_device_name, raw_input->data.keyboard.MakeCode, raw_input->data.keyboard.VKey, raw_input->data.keyboard.Message);
				}

				free(raw_input_device_name);
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
	HWND window;
	RAWINPUTDEVICE raw_input_device;
	MSG message;

	window_class.lpszClassName = "rawinput2";
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

	window = CreateWindowEx(WS_EX_CLIENTEDGE, "rawinput2", "rawinput2", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance, NULL);
	edit_control = CreateWindow("EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, window, NULL, instance, NULL);
	SendMessage(edit_control, WM_SETFONT, (WPARAM)(GetStockObject(ANSI_VAR_FONT)), (LPARAM)(TRUE));
	ShowWindow(window, show_state);

	raw_input_device.usUsagePage = 1;
	raw_input_device.usUsage = 6;
	raw_input_device.dwFlags = RIDEV_NOLEGACY;
	raw_input_device.hwndTarget = window;
	RegisterRawInputDevices(&raw_input_device, 1, sizeof(RAWINPUTDEVICE));

	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return 0;
}

