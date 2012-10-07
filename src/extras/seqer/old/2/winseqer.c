
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <commctrl.h>

#include <simple-collections.h>
#include <midifile.h>
#include <seqer.h>

#define WIN_SEQER_ID_ICON 1000

#define WIN_SEQER_ID_MENU_FILE_NEW_WINDOW 2000
#define WIN_SEQER_ID_MENU_FILE_NEW 2001
#define WIN_SEQER_ID_MENU_FILE_OPEN 2002
#define WIN_SEQER_ID_MENU_FILE_SAVE 2003
#define WIN_SEQER_ID_MENU_FILE_SAVE_AS 2004
#define WIN_SEQER_ID_MENU_FILE_REVERT 2005
#define WIN_SEQER_ID_MENU_FILE_EXIT 2006
#define WIN_SEQER_ID_MENU_EDIT_UNDO 2007
#define WIN_SEQER_ID_MENU_EDIT_REDO 2008
#define WIN_SEQER_ID_MENU_EDIT_CUT 2009
#define WIN_SEQER_ID_MENU_EDIT_COPY 2010
#define WIN_SEQER_ID_MENU_EDIT_PASTE 2011
#define WIN_SEQER_ID_MENU_EDIT_DELETE 2012
#define WIN_SEQER_ID_MENU_VIEW_PIANO_ROLL 2013
#define WIN_SEQER_ID_MENU_VIEW_EVENT_LIST 2014
#define WIN_SEQER_ID_MENU_VIEW_PIANO_ROLL_MODE_SINGLE 2015
#define WIN_SEQER_ID_MENU_VIEW_PIANO_ROLL_MODE_OVERLAID 2016
#define WIN_SEQER_ID_MENU_VIEW_PIANO_ROLL_MODE_STACKED 2017
#define WIN_SEQER_ID_MENU_VIEW_PREVIOUS_TRACK 2018
#define WIN_SEQER_ID_MENU_VIEW_NEXT_TRACK 2019
#define WIN_SEQER_ID_MENU_VIEW_TRACKS 2020
#define WIN_SEQER_ID_MENU_VIEW_VIEW_SETTINGS 2021
#define WIN_SEQER_ID_MENU_VIEW_GLOBAL_SETTINGS 2022
#define WIN_SEQER_ID_MENU_TRANSPORT_PLAY 2023
#define WIN_SEQER_ID_MENU_TRANSPORT_RECORD 2024
#define WIN_SEQER_ID_MENU_TRANSPORT_STOP 2025
#define WIN_SEQER_ID_MENU_TRANSPORT_PORTS 2026
#define WIN_SEQER_ID_MENU_HELP_ABOUT 2027

typedef struct WinSeqerApplication *WinSeqerApplication_t;
typedef struct WinSeqerWindow *WinSeqerWindow_t;

static WinSeqerApplication_t WinSeqerApplication_new(HINSTANCE instance);
static void WinSeqerApplication_free(WinSeqerApplication_t application);
static void WinSeqerApplication_processMessages(WinSeqerApplication_t application);

static WinSeqerWindow_t WinSeqerWindow_new(WinSeqerApplication_t application, WinSeqerWindow_t base_window);
static void WinSeqerWindow_free(WinSeqerWindow_t window);
static int WinSeqerWindow_save(WinSeqerWindow_t window, int prompt_for_filename);
static LRESULT CALLBACK WinSeqerWindow_processMessage(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam);
static LRESULT CALLBACK WinSeqerCanvas_processMessage(HWND canvas_handle, UINT message_id, WPARAM wparam, LPARAM lparam);
static void WinSeqerWindow_drawLine(SeqerWindow_t window, int x1, int y1, int x2, int y2, int color);
static void WinSeqerWindow_drawRectangle(SeqerWindow_t window, int x, int y, int width, int height, int color, int fill_color);
static void WinSeqerWindow_drawCircle(SeqerWindow_t window, int x, int y, int diameter, int color, int fill_color);
static void WinSeqerWindow_measureString(SeqerWindow_t window, char *string, int *width_out, int *height_out);
static void WinSeqerWindow_drawString(SeqerWindow_t window, int x, int y, char *string, int color);
static void WinSeqerWindow_filenameChange(SeqerWindow_t underlying_window);
static void WinSeqerWindow_redraw(SeqerWindow_t underlying_window);
static void WinSeqerWindow_configureHorizontalScrollbar(SeqerWindow_t underlying_window, int max, int line, int page, int current);
static void WinSeqerWindow_configureVerticalScrollbar(SeqerWindow_t underlying_window, int max, int line, int page, int current);

struct WinSeqerApplication
{
	SeqerApplication_t application;
	HINSTANCE instance;
};

struct WinSeqerWindow
{
	SeqerWindow_t window;
	HWND window_handle;
	HWND canvas_handle;
	HDC canvas_device_context;
	int color;
	int fill_color;
	HWND status_bar_handle;
	int horizontal_line_scroll_amount;
	int vertical_line_scroll_amount;
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show)
{
	WinSeqerApplication_t application;
	WinSeqerWindow_t window;

	InitCommonControls();

	application = WinSeqerApplication_new(instance);
	window = WinSeqerWindow_new(application, NULL);

	if (command_line[0] != '\0')
	{
		/* TODO: handle multiple arguments by opening multiple windows */

		if (SeqerWindow_loadMidiFile(window->window, command_line) < 0)
		{
			String_t error_message = String_new(256);
			String_set(error_message, (unsigned char *)("Cannot load MIDI file \""));
			String_addString(error_message, (unsigned char *)(command_line));
			String_addString(error_message, (unsigned char *)("\"."));
			MessageBox(NULL, (char *)(String_get(error_message)), "Seqer", MB_OK | MB_ICONERROR | MB_APPLMODAL);
			String_free(error_message);
			WinSeqerApplication_free(application);
			return 1;
		}
	}

	ShowWindow(window->window_handle, show);

 	WinSeqerApplication_processMessages(application);

	WinSeqerApplication_free(application);
	return 0;
}

static WinSeqerApplication_t WinSeqerApplication_new(HINSTANCE instance)
{
	WinSeqerApplication_t application = (WinSeqerApplication_t)(malloc(sizeof (struct WinSeqerApplication)));

	application->application = SeqerApplication_new();
	SeqerApplication_setDrawLineCallback(application->application, WinSeqerWindow_drawLine);
	SeqerApplication_setDrawRectangleCallback(application->application, WinSeqerWindow_drawRectangle);
	SeqerApplication_setDrawCircleCallback(application->application, WinSeqerWindow_drawCircle);
	SeqerApplication_setMeasureStringCallback(application->application, WinSeqerWindow_measureString);
	SeqerApplication_setDrawStringCallback(application->application, WinSeqerWindow_drawString);
	SeqerApplication_setFilenameChangeCallback(application->application, WinSeqerWindow_filenameChange);
	SeqerApplication_setRedrawCallback(application->application, WinSeqerWindow_redraw);
	SeqerApplication_setConfigureHorizontalScrollbarCallback(application->application, WinSeqerWindow_configureHorizontalScrollbar);
	SeqerApplication_setConfigureVerticalScrollbarCallback(application->application, WinSeqerWindow_configureVerticalScrollbar);
	SeqerApplication_setUserData(application->application, application);

	application->instance = instance;

	{
		WNDCLASSEX window_class;
		window_class.cbSize = sizeof (WNDCLASSEX);
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = WinSeqerWindow_processMessage;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = instance;
		window_class.hIcon = LoadIcon(instance, MAKEINTRESOURCE(WIN_SEQER_ID_ICON));
		window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_class.hbrBackground = NULL;
		window_class.lpszMenuName =  NULL;
		window_class.lpszClassName = "SeqerWindow";
		window_class.hIconSm = NULL;
		RegisterClassEx(&window_class);
	}

	{
		WNDCLASSEX window_class;
		window_class.cbSize = sizeof (WNDCLASSEX);
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = WinSeqerCanvas_processMessage;
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

	return application;
}

static void WinSeqerApplication_free(WinSeqerApplication_t application)
{
	SeqerApplication_free(application->application);
	free(application);
}

static void WinSeqerApplication_processMessages(WinSeqerApplication_t application)
{
	MSG message;

	while (GetMessage(&message, (HWND)(NULL), 0, 0) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

static WinSeqerWindow_t WinSeqerWindow_new(WinSeqerApplication_t application, WinSeqerWindow_t base_window)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(malloc(sizeof (struct WinSeqerWindow)));
	window->window = SeqerWindow_new(application->application, (base_window == NULL) ? NULL : base_window->window);
	SeqerWindow_setUserData(window->window, window);

	if (base_window == NULL)
	{
		window->window_handle = CreateWindowEx(0, "SeqerWindow", "Seqer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)(NULL), (HMENU)(NULL), application->instance, window);
	}
	else
	{
		RECT base_window_rect;
		GetWindowRect(base_window->window_handle, &base_window_rect);
		window->window_handle = CreateWindowEx(0, "SeqerWindow", "Seqer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, base_window_rect.right - base_window_rect.left, base_window_rect.bottom - base_window_rect.top, (HWND)(NULL), (HMENU)(NULL), application->instance, window);
	}

	{
		HMENU menubar_handle = CreateMenu();

		{
			HMENU file_menu_handle = CreatePopupMenu();
			AppendMenu(file_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_FILE_NEW_WINDOW, "New &Window");
			AppendMenu(file_menu_handle, MF_SEPARATOR, 0, NULL);
			AppendMenu(file_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_FILE_NEW, "&New");
			AppendMenu(file_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_FILE_OPEN, "&Open...");
			AppendMenu(file_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_FILE_SAVE, "&Save\tCtrl+S");
			AppendMenu(file_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_FILE_SAVE_AS, "Save &As...");
			AppendMenu(file_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_FILE_REVERT, "&Revert");
			AppendMenu(file_menu_handle, MF_SEPARATOR, 0, NULL);
			AppendMenu(file_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_FILE_EXIT, "E&xit");
			AppendMenu(menubar_handle, MF_STRING | MF_POPUP, (UINT)(file_menu_handle), "&File");
		}

		{
			HMENU edit_menu_handle = CreatePopupMenu();
			AppendMenu(edit_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_EDIT_UNDO, "&Undo\tCtrl+Z");
			AppendMenu(edit_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_EDIT_REDO, "&Redo\tCtrl+Y");
			AppendMenu(edit_menu_handle, MF_SEPARATOR, 0, NULL);
			AppendMenu(edit_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_EDIT_CUT, "Cu&t\tCtrl+X");
			AppendMenu(edit_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_EDIT_COPY, "&Copy\tCtrl+C");
			AppendMenu(edit_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_EDIT_PASTE, "&Paste\tCtrl+V");
			AppendMenu(edit_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_EDIT_DELETE, "&Delete\tDel");
			AppendMenu(menubar_handle, MF_STRING | MF_POPUP, (UINT)(edit_menu_handle), "&Edit");
		}

		{
			HMENU view_menu_handle = CreatePopupMenu();
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_PIANO_ROLL, "Piano &Roll");
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_EVENT_LIST, "Event &List");
			AppendMenu(view_menu_handle, MF_SEPARATOR, 0, NULL);
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_PIANO_ROLL_MODE_SINGLE, "&Single");
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_PIANO_ROLL_MODE_OVERLAID, "&Overlaid");
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_PIANO_ROLL_MODE_STACKED, "Stac&ked");
			AppendMenu(view_menu_handle, MF_SEPARATOR, 0, NULL);
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_PREVIOUS_TRACK, "&Previous Track");
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_NEXT_TRACK, "&Next Track");
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_NEXT_TRACK, "&Tracks...");
			AppendMenu(view_menu_handle, MF_SEPARATOR, 0, NULL);
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_VIEW_SETTINGS, "&View Settings...");
			AppendMenu(view_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_VIEW_GLOBAL_SETTINGS, "&Global Settings...");
			AppendMenu(menubar_handle, MF_STRING | MF_POPUP, (UINT)(view_menu_handle), "&View");
		}

		{
			HMENU transport_menu_handle = CreatePopupMenu();
			AppendMenu(transport_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_TRANSPORT_PLAY, "&Play");
			AppendMenu(transport_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_TRANSPORT_RECORD, "&Record");
			AppendMenu(transport_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_TRANSPORT_STOP, "&Stop");
			AppendMenu(transport_menu_handle, MF_SEPARATOR, 0, NULL);
			AppendMenu(transport_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_TRANSPORT_PORTS, "&Ports...");
			AppendMenu(menubar_handle, MF_STRING | MF_POPUP, (UINT)(transport_menu_handle), "&Transport");
		}

		{
			HMENU help_menu_handle = CreatePopupMenu();
			AppendMenu(help_menu_handle, MF_STRING, WIN_SEQER_ID_MENU_HELP_ABOUT, "&About Seqer");
			AppendMenu(menubar_handle, MF_STRING | MF_POPUP, (UINT)(help_menu_handle), "&Help");
		}

		SetMenu(window->window_handle, menubar_handle);
	}

	window->canvas_handle = CreateWindowEx(WS_EX_CLIENTEDGE, "SeqerCanvas", "", WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, 0, 0, 100, 100, window->window_handle, (HMENU)(NULL), application->instance, window);
	window->status_bar_handle = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, window->window_handle, (HMENU)(NULL), application->instance, NULL);
	WinSeqerWindow_redraw(window->window);
	WinSeqerWindow_filenameChange(window->window);
	return window;
}

static void WinSeqerWindow_free(WinSeqerWindow_t window)
{
	SeqerWindow_free(window->window);
	free(window);
}

static int WinSeqerWindow_save(WinSeqerWindow_t window, int prompt_for_filename)
{
	if (prompt_for_filename || SeqerWindow_shouldPromptForFilename(window->window))
	{
		OPENFILENAME open_file_name;
		char filename[1024];

		memset(&open_file_name, 0, sizeof (OPENFILENAME));
		open_file_name.lStructSize = sizeof (OPENFILENAME);
		open_file_name.hwndOwner = window->window_handle;
		open_file_name.lpstrFilter = "MIDI Files (*.mid, *.rmi)\0*.mid;*.rmi\0All Files\0*.*\0";
		open_file_name.lpstrFile = filename;
		open_file_name.nMaxFile = 1024;
		open_file_name.Flags = OFN_EXPLORER;
		open_file_name.lpstrDefExt = "mid";
		filename[0] = '\0';

		if (GetSaveFileName(&open_file_name))
		{
			if (SeqerWindow_saveMidiFile(window->window, filename) < 0)
			{
				String_t error_message = String_new(256);
				String_set(error_message, (unsigned char *)("Cannot save MIDI file \""));
				String_addString(error_message, (unsigned char *)(filename));
				String_addString(error_message, (unsigned char *)("\"."));
				MessageBox(NULL, (char *)(String_get(error_message)), "Seqer", MB_OK | MB_ICONERROR | MB_APPLMODAL);
				String_free(error_message);
				return -1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		if (SeqerWindow_saveMidiFile(window->window, NULL) < 0)
		{
			String_t error_message = String_new(256);
			String_set(error_message, (unsigned char *)("Cannot save MIDI file \""));
			String_addString(error_message, (unsigned char *)(SeqerWindow_getFilename(window->window)));
			String_addString(error_message, (unsigned char *)("\"."));
			MessageBox(NULL, (char *)(String_get(error_message)), "Seqer", MB_OK | MB_ICONERROR | MB_APPLMODAL);
			String_free(error_message);
			return -1;
		}
		else
		{
			return 0;
		}
	}
}

static LRESULT CALLBACK WinSeqerWindow_processMessage(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(GetWindowLongPtr(window_handle, GWLP_USERDATA));

	switch (message_id)
	{
		case WM_CREATE:
		{
			SetWindowLongPtr(window_handle, GWLP_USERDATA, (long)((WinSeqerWindow_t)(((LPCREATESTRUCT)(lparam))->lpCreateParams)));
			break;
		}
		case WM_SIZE:
		{
			SendMessage(window->status_bar_handle, WM_SIZE, 0, 0);

			{
				RECT client_rect;
				RECT status_bar_rect;
				GetClientRect(window_handle, &client_rect);
				GetWindowRect(window->status_bar_handle, &status_bar_rect);
				SetWindowPos(window->canvas_handle, NULL, 0, 0, client_rect.right, client_rect.bottom - (status_bar_rect.bottom - status_bar_rect.top), SWP_NOZORDER);
			}

			break;
		}
		case WM_CLOSE:
		{
			int cancelled = 0;

			if (SeqerWindow_shouldPromptForSave(window->window))
			{
				switch (MessageBox(window->window_handle, "Save changes to file?", "Seqer", MB_YESNOCANCEL | MB_ICONWARNING | MB_APPLMODAL))
				{
					case IDYES:
					{
						if (WinSeqerWindow_save(window, 0) < 0) cancelled = 1;
						break;
					}
					case IDNO:
					{
						break;
					}
					case IDCANCEL:
					{
						cancelled = 1;
						break;
					}
				}
			}

			if (cancelled)
			{
				return 0;
			}
			else
			{
				SeqerApplication_t application = SeqerWindow_getApplication(window->window);
				WinSeqerWindow_free(window);
				if (SeqerApplication_getNumberOfWindows(application) == 0) PostQuitMessage(0);
			}

			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wparam))
			{
				case WIN_SEQER_ID_MENU_FILE_NEW_WINDOW:
				{
					WinSeqerApplication_t application = (WinSeqerApplication_t)(SeqerApplication_getUserData(SeqerWindow_getApplication(window->window)));
					ShowWindow(WinSeqerWindow_new(application, window)->window_handle, SW_SHOWNORMAL);
					break;
				}
				case WIN_SEQER_ID_MENU_FILE_NEW:
				{
					int cancelled = 0;

					if (SeqerWindow_shouldPromptForSave(window->window))
					{
						switch (MessageBox(window->window_handle, "Save changes to file?", "Seqer", MB_YESNOCANCEL | MB_ICONWARNING | MB_APPLMODAL))
						{
							case IDYES:
							{
								if (WinSeqerWindow_save(window, 0) < 0) cancelled = 1;
								break;
							}
							case IDNO:
							{
								break;
							}
							case IDCANCEL:
							{
								cancelled = 1;
								break;
							}
						}
					}

					if (!cancelled)
					{
						SeqerWindow_newMidiFile(window->window);
					}

					break;
				}
				case WIN_SEQER_ID_MENU_FILE_OPEN:
				{
					int cancelled = 0;

					if (SeqerWindow_shouldPromptForSave(window->window))
					{
						switch (MessageBox(window->window_handle, "Save changes to file?", "Seqer", MB_YESNOCANCEL | MB_ICONWARNING | MB_APPLMODAL))
						{
							case IDYES:
							{
								if (WinSeqerWindow_save(window, 0) < 0) cancelled = 1;
								break;
							}
							case IDNO:
							{
								break;
							}
							case IDCANCEL:
							{
								cancelled = 1;
								break;
							}
						}
					}

					if (!cancelled)
					{
						OPENFILENAME open_file_name;
						char filename[1024];

						memset(&open_file_name, 0, sizeof (OPENFILENAME));
						open_file_name.lStructSize = sizeof (OPENFILENAME);
						open_file_name.hwndOwner = window_handle;
						open_file_name.lpstrFilter = "MIDI Files (*.mid, *.rmi)\0*.mid;*.rmi\0All Files\0*.*\0";
						open_file_name.lpstrFile = filename;
						open_file_name.nMaxFile = 1024;
						open_file_name.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
						open_file_name.lpstrDefExt = "mid";
						filename[0] = '\0';

						if (GetOpenFileName(&open_file_name))
						{
							if (SeqerWindow_loadMidiFile(window->window, filename) < 0)
							{
								String_t error_message = String_new(256);
								String_set(error_message, (unsigned char *)("Cannot load MIDI file \""));
								String_addString(error_message, (unsigned char *)(filename));
								String_addString(error_message, (unsigned char *)("\"."));
								MessageBox(NULL, (char *)(String_get(error_message)), "Seqer", MB_OK | MB_ICONERROR | MB_APPLMODAL);
								String_free(error_message);
							}
						}
					}

					break;
				}
				case WIN_SEQER_ID_MENU_FILE_SAVE:
				{
					WinSeqerWindow_save(window, 0);
					break;
				}
				case WIN_SEQER_ID_MENU_FILE_SAVE_AS:
				{
					WinSeqerWindow_save(window, 1);
					break;
				}
				case WIN_SEQER_ID_MENU_FILE_EXIT:
				{
					int cancelled = 0;

					if (SeqerWindow_shouldPromptForSave(window->window))
					{
						switch (MessageBox(window->window_handle, "Save changes to file?", "Seqer", MB_YESNOCANCEL | MB_ICONWARNING | MB_APPLMODAL))
						{
							case IDYES:
							{
								if (WinSeqerWindow_save(window, 0) < 0) cancelled = 1;
								break;
							}
							case IDNO:
							{
								break;
							}
							case IDCANCEL:
							{
								cancelled = 1;
								break;
							}
						}
					}

					if (cancelled)
					{
						return 0;
					}
					else
					{
						SeqerApplication_t application = SeqerWindow_getApplication(window->window);
						WinSeqerWindow_free(window);
						if (SeqerApplication_getNumberOfWindows(application) == 0) PostQuitMessage(0);
					}

					break;
				}
				default:
				{
					break;
				}
			}

			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(window_handle, message_id, wparam, lparam);
}

static LRESULT CALLBACK WinSeqerCanvas_processMessage(HWND canvas_handle, UINT message_id, WPARAM wparam, LPARAM lparam)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(GetWindowLongPtr(canvas_handle, GWLP_USERDATA));

	switch (message_id)
	{
		case WM_CREATE:
		{
			SetWindowLongPtr(canvas_handle, GWLP_USERDATA, (long)((WinSeqerWindow_t)(((LPCREATESTRUCT)(lparam))->lpCreateParams)));
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint_struct;
			HDC device_context;
			HBITMAP bitmap;
			HPEN pen;
			HBRUSH brush;
			device_context = BeginPaint(canvas_handle, &paint_struct);
			window->canvas_device_context = CreateCompatibleDC(device_context);
			bitmap = SelectObject(window->canvas_device_context, CreateCompatibleBitmap(device_context, SeqerWindow_getWidth(window->window), SeqerWindow_getHeight(window->window)));
			pen = SelectObject(window->canvas_device_context, CreatePen(PS_SOLID, 0, 0));
			brush = SelectObject(window->canvas_device_context, CreateSolidBrush(0));
			window->color = 0x000000;
			window->fill_color = 0x000000;
			SeqerWindow_paint(window->window);
			BitBlt(device_context, 0, 0, SeqerWindow_getWidth(window->window), SeqerWindow_getHeight(window->window), window->canvas_device_context, 0, 0, SRCCOPY);
			DeleteObject(SelectObject(window->canvas_device_context, brush));
			DeleteObject(SelectObject(window->canvas_device_context, pen));
			DeleteObject(SelectObject(window->canvas_device_context, bitmap));
			DeleteDC(window->canvas_device_context);
			EndPaint(canvas_handle, &paint_struct);
			break;
		}
		case WM_SIZE:
		{
			RECT client_rect;
			GetClientRect(canvas_handle, &client_rect);
			SeqerWindow_setSize(window->window, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top);
			break;
		}
		case WM_HSCROLL:
		{
			SCROLLINFO scroll_info;
			scroll_info.cbSize = sizeof (SCROLLINFO);
			scroll_info.fMask = SIF_ALL;
			GetScrollInfo(canvas_handle, SB_HORZ, &scroll_info);

			switch LOWORD(wparam)
			{
				case SB_LINELEFT:
				{
					scroll_info.nPos -= window->horizontal_line_scroll_amount;
					break;
				}
				case SB_LINERIGHT:
				{
					scroll_info.nPos += window->horizontal_line_scroll_amount;
					break;
				}
				case SB_PAGELEFT:
				{
					scroll_info.nPos -= scroll_info.nPage;
					break;
				}
				case SB_PAGERIGHT:
				{
					scroll_info.nPos += scroll_info.nPage;
					break;
				}
				case SB_THUMBTRACK:
				{
					scroll_info.nPos = scroll_info.nTrackPos;
					break;
				}
				default:
				{
					break;
				}
			}

			scroll_info.fMask = SIF_POS;
			SetScrollInfo(canvas_handle, SB_HORZ, &scroll_info, TRUE);
			GetScrollInfo(canvas_handle, SB_HORZ, &scroll_info);
			SeqerWindow_scrollHorizontal(window->window, scroll_info.nPos);
			break;
		}
		case WM_VSCROLL:
		{
			SCROLLINFO scroll_info;
			scroll_info.cbSize = sizeof (SCROLLINFO);
			scroll_info.fMask = SIF_ALL;
			GetScrollInfo(canvas_handle, SB_VERT, &scroll_info);

			switch LOWORD(wparam)
			{
				case SB_LINELEFT:
				{
					scroll_info.nPos -= window->vertical_line_scroll_amount;
					break;
				}
				case SB_LINERIGHT:
				{
					scroll_info.nPos += window->vertical_line_scroll_amount;
					break;
				}
				case SB_PAGELEFT:
				{
					scroll_info.nPos -= scroll_info.nPage;
					break;
				}
				case SB_PAGERIGHT:
				{
					scroll_info.nPos += scroll_info.nPage;
					break;
				}
				case SB_THUMBTRACK:
				{
					scroll_info.nPos = scroll_info.nTrackPos;
					break;
				}
				default:
				{
					break;
				}
			}

			scroll_info.fMask = SIF_POS;
			SetScrollInfo(canvas_handle, SB_VERT, &scroll_info, TRUE);
			GetScrollInfo(canvas_handle, SB_VERT, &scroll_info);
			SeqerWindow_scrollVertical(window->window, scroll_info.nPos);
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(canvas_handle, message_id, wparam, lparam);
}

static void WinSeqerWindow_setColor(WinSeqerWindow_t window, int color)
{
	if (window->color != color)
	{
		switch (color)
		{
			case SEQER_COLOR_SELECTED_FOREGROUND:
			{
				DeleteObject(SelectObject(window->canvas_device_context, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_HIGHLIGHTTEXT))));
				break;
			}
			case SEQER_COLOR_SELECTED_BACKGROUND:
			{
				DeleteObject(SelectObject(window->canvas_device_context, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_HIGHLIGHT))));
				break;
			}
			default:
			{
				DeleteObject(SelectObject(window->canvas_device_context, CreatePen(PS_SOLID, 0, RGB((color & 0xFF0000) >> 16, (color & 0x00FF00) >> 8, color & 0x0000FF))));
				break;
			}
		}

		window->color = color;
	}
}

static void WinSeqerWindow_setFillColor(WinSeqerWindow_t window, int fill_color)
{
	if (window->fill_color != fill_color)
	{
		switch (fill_color)
		{
			case SEQER_COLOR_SELECTED_FOREGROUND:
			{
				DeleteObject(SelectObject(window->canvas_device_context, CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHTTEXT))));
				break;
			}
			case SEQER_COLOR_SELECTED_BACKGROUND:
			{
				DeleteObject(SelectObject(window->canvas_device_context, CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT))));
				break;
			}
			default:
			{
				DeleteObject(SelectObject(window->canvas_device_context, CreateSolidBrush(RGB((fill_color & 0xFF0000) >> 16, (fill_color & 0x00FF00) >> 8, fill_color & 0x0000FF))));
				break;
			}
		}

		window->fill_color = fill_color;
	}
}

static void WinSeqerWindow_drawLine(SeqerWindow_t underlying_window, int x1, int y1, int x2, int y2, int color)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	WinSeqerWindow_setColor(window, color);
	MoveToEx(window->canvas_device_context, x1, y1, NULL);
	LineTo(window->canvas_device_context, x2, y2);
}

static void WinSeqerWindow_drawRectangle(SeqerWindow_t underlying_window, int x, int y, int width, int height, int color, int fill_color)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	WinSeqerWindow_setColor(window, color);
	WinSeqerWindow_setFillColor(window, fill_color);
	Rectangle(window->canvas_device_context, x, y, x + width, y + height);
}

static void WinSeqerWindow_drawCircle(SeqerWindow_t underlying_window, int x, int y, int diameter, int color, int fill_color)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	WinSeqerWindow_setColor(window, color);
	WinSeqerWindow_setFillColor(window, fill_color);
	Ellipse(window->canvas_device_context, x - (diameter / 2), y - (diameter / 2), x + (diameter / 2), y + (diameter / 2));
}

static void WinSeqerWindow_measureString(SeqerWindow_t underlying_window, char *string, int *width_out, int *height_out)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	SIZE size;
	GetTextExtentPoint32(window->canvas_device_context, string, strlen(string), &size);
	if (width_out != NULL) *width_out = size.cx;
	if (height_out != NULL) *height_out = size.cy;
}

static void WinSeqerWindow_drawString(SeqerWindow_t underlying_window, int x, int y, char *string, int color)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	WinSeqerWindow_setColor(window, color);
	TextOut(window->canvas_device_context, x, y, string, strlen(string));
}

static void WinSeqerWindow_filenameChange(SeqerWindow_t underlying_window)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	char *filename = SeqerWindow_getFilename(window->window);
	int modified = SeqerWindow_getModified(window->window);

	if (filename[0] == '\0')
	{
		if (modified)
		{
			SendMessage(window->window_handle, WM_SETTEXT, 0, (LPARAM)("Untitled + Seqer"));
		}
		else
		{
			SendMessage(window->window_handle, WM_SETTEXT, 0, (LPARAM)("Untitled - Seqer"));
		}
	}
	else
	{
		char *basename;
		String_t title;

		basename = strrchr(filename, '\\');

		if (basename == NULL)
		{
			basename = strrchr(filename, '/');

			if (basename == NULL)
			{
				basename = filename;
			}
			else
			{
				basename++;
			}
		}
		else
		{
			basename++;
		}

		title = String_new(64);
		String_set(title, basename);

		if (modified)
		{
			String_addString(title, (unsigned char *)(" + Seqer"));
		}
		else
		{
			String_addString(title, (unsigned char *)(" - Seqer"));
		}

		SendMessage(window->window_handle, WM_SETTEXT, 0, (LPARAM)(String_get(title)));
		String_free(title);
	}
}

static void WinSeqerWindow_redraw(SeqerWindow_t underlying_window)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	RedrawWindow(window->window_handle, NULL, NULL, RDW_INVALIDATE | RDW_VALIDATE | RDW_ALLCHILDREN);
}

static void WinSeqerWindow_configureHorizontalScrollbar(SeqerWindow_t underlying_window, int max, int line, int page, int current)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	SCROLLINFO scroll_info;
	scroll_info.cbSize = sizeof (SCROLLINFO);
	scroll_info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	scroll_info.nMin = 0;
	scroll_info.nMax = max;
	scroll_info.nPage = page;
	scroll_info.nPos = current;
	SetScrollInfo(window->canvas_handle, SB_HORZ, &scroll_info, TRUE);
	window->horizontal_line_scroll_amount = line;
}

static void WinSeqerWindow_configureVerticalScrollbar(SeqerWindow_t underlying_window, int max, int line, int page, int current)
{
	WinSeqerWindow_t window = (WinSeqerWindow_t)(SeqerWindow_getUserData(underlying_window));
	SCROLLINFO scroll_info;
	scroll_info.cbSize = sizeof (SCROLLINFO);
	scroll_info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	scroll_info.nMin = 0;
	scroll_info.nMax = max;
	scroll_info.nPage = page;
	scroll_info.nPos = current;
	SetScrollInfo(window->canvas_handle, SB_VERT, &scroll_info, TRUE);
	window->vertical_line_scroll_amount = line;
}

