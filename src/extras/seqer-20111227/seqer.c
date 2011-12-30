
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <commctrl.h>

#include <simple-collections.h>
#include <midifile.h>

static HINSTANCE g_application_instance;
static HWND g_window_handle;
static HWND g_canvas_handle;
static HWND g_status_bar_handle;
static MidiFile_t g_midi_file;

static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam)
{
	switch (message_id)
	{
		case WM_SIZE:
		{
			SendMessage(g_status_bar_handle, WM_SIZE, 0, 0);

			{
				RECT client_rect;
				RECT status_bar_rect;
				GetClientRect(window_handle, &client_rect);
				GetWindowRect(g_status_bar_handle, &status_bar_rect);
				SetWindowPos(g_canvas_handle, NULL, 0, 0, client_rect.right, client_rect.bottom - (status_bar_rect.bottom - status_bar_rect.top), SWP_NOZORDER);
			}

			break;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(window_handle, message_id, wparam, lparam);
}

static LRESULT CALLBACK CanvasWindowProc(HWND canvas_handle, UINT message_id, WPARAM wparam, LPARAM lparam)
{
	switch (message_id)
	{
		case WM_PAINT:
		{
			RECT client_rect;
			PAINTSTRUCT paint_struct;
			HDC device_context;
			HDC canvas_device_context;
			HBITMAP bitmap;
			HPEN pen;
			HBRUSH brush;
			GetClientRect(canvas_handle, &client_rect);
			device_context = BeginPaint(canvas_handle, &paint_struct);
			canvas_device_context = CreateCompatibleDC(device_context);
			bitmap = SelectObject(canvas_device_context, CreateCompatibleBitmap(device_context, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top));
			pen = SelectObject(canvas_device_context, CreatePen(PS_SOLID, 0, RGB(0xFF, 0xFF, 0xFF)));
			brush = SelectObject(canvas_device_context, CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF)));
			Rectangle(canvas_device_context, 0, 0, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top);
			// TODO: paint 
			BitBlt(device_context, 0, 0, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top, canvas_device_context, 0, 0, SRCCOPY);
			DeleteObject(SelectObject(canvas_device_context, brush));
			DeleteObject(SelectObject(canvas_device_context, pen));
			DeleteObject(SelectObject(canvas_device_context, bitmap));
			DeleteDC(canvas_device_context);
			EndPaint(canvas_handle, &paint_struct);
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(canvas_handle, message_id, wparam, lparam);
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
		window_class.hbrBackground = NULL;
		window_class.lpszMenuName = NULL;
		window_class.lpszClassName = "SeqerWindow";
		window_class.hIconSm = NULL;
		RegisterClassEx(&window_class);
	}

	{
		WNDCLASSEX window_class;
		window_class.cbSize = sizeof (WNDCLASSEX);
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = CanvasWindowProc;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = instance;
		window_class.hIcon = NULL;
		window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_class.hbrBackground = NULL;
		window_class.lpszMenuName =  NULL;
		window_class.lpszClassName = "SeqerCanvas";
		window_class.hIconSm = NULL;
		RegisterClassEx(&window_class);
	}

	g_midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);

	g_window_handle = CreateWindowEx(0, "SeqerWindow", "Seqer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)(NULL), (HMENU)(NULL), g_application_instance, NULL);
	g_canvas_handle = CreateWindowEx(WS_EX_CLIENTEDGE, "SeqerCanvas", "", WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, 0, 0, 100, 100, g_window_handle, (HMENU)(NULL), g_application_instance, NULL);
	g_status_bar_handle = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, g_window_handle, (HMENU)(NULL), g_application_instance, NULL);
	RedrawWindow(g_window_handle, NULL, NULL, RDW_INVALIDATE | RDW_VALIDATE | RDW_ALLCHILDREN);
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

