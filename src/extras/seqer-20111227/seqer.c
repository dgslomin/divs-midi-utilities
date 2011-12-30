
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <commctrl.h>

#include <simple-collections.h>
#include <midifile.h>

#define STATUS_BAR_ID 0

static HINSTANCE g_application_instance;
static HWND g_window_handle;
static MidiFile_t g_midi_file;

static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam)
{
	switch (message_id)
	{
		case WM_CREATE:
		{
			HWND status_bar = GetDlgItem(window_handle, STATUS_BAR_ID);
			SendMessage(status_bar, WM_CREATE, 0, 0);
			/* TODO */
			return 0;
		}
		case WM_PAINT:
		{
			HWND status_bar = GetDlgItem(window_handle, STATUS_BAR_ID);
			SendMessage(status_bar, WM_PAINT, 0, 0);
			/* TODO */
			return 0;
		}
		case WM_SIZE:
		{
			HWND status_bar = GetDlgItem(window_handle, STATUS_BAR_ID);
			SendMessage(status_bar, WM_SIZE, 0, 0);
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		default:
		{
			return DefWindowProc(window_handle, message_id, wparam, lparam);
		}
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show)
{
	InitCommonControls();
	g_application_instance = instance;

	{
		WNDCLASSEX window_class;
		window_class.cbSize = sizeof(window_class);
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = WindowProc;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = instance;
		window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_class.hbrBackground = GetStockObject(WHITE_BRUSH);
		window_class.lpszMenuName = "SeqerMainMenu";
		window_class.lpszClassName = "SeqerWindow";
		window_class.hIconSm = LoadImage(instance, MAKEINTRESOURCE(5), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		RegisterClassEx(&window_class);
	}

	g_midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);

	g_window_handle = CreateWindowEx(0, "SeqerWindow", "Seqer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND) NULL, (HMENU) NULL, g_application_instance, NULL);
	CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, g_window_handle, (HMENU)(STATUS_BAR_ID), g_application_instance, NULL);
	ShowWindow(g_window_handle, show);

	{
		MSG message;

		while (GetMessage(&message, (HWND)(NULL), 0, 0) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	MidiFile_free(g_midi_file);
	return 0;
}

