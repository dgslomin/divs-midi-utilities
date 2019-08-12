
#include <functional>
#include <set>
#include <wx/wx.h>
#include <wx/aboutdlg.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <midifile.h>
#include "seqer.h"

#ifndef __WXMSW__
#include "seqer.xpm"
#endif

enum
{
	SEQER_ID_NEW_WINDOW = wxID_HIGHEST + 1,
	SEQER_ID_HIGHEST
};

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	if (wxGetEnv("SEQER_DEBUG", NULL)) wxMessageBox("Attach debugger now.", "Seqer", wxOK);

	Window* window = new Window(this);
	window->Show(true);

	if (this->argc > 1)
	{
		if (!window->Load(this->argv[1]))
		{
			wxMessageBox("Cannot open the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
		}
	}

	return true;
}

TrackListWindow::TrackListWindow(Application* application, Window* existing_window): wxFrame((wxFrame*)(NULL), wxID_ANY, "Seqer", wxDefaultPosition, wxSize(640, 480))
{
	this->application = application;
	this->SetIcon(wxICON(seqer));

	wxMenuBar* menu_bar = new wxMenuBar();
		wxMenu* file_menu = new wxMenu();
			file_menu->Append(wxID_NEW, "&New\tCtrl+N");
			file_menu->AppendSeparator();
			wxMenu* new_window_menu = new wxMenu();
				new_window_menu->Append(SEQER_NEW_TRACK_LIST_WINDOW, "&Track List");
				new_window_menu->Append(SEQER_NEW_PIANO_ROLL_WINDOW, "&Piano Roll");
				new_window_menu->Append(SEQER_NEW_EVENT_LIST_WINDOW, "&Event List");
			file_menu->AppendSubMenu(new_window_menu, "New &Window");
			file_menu->AppendSeparator();
			file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O");
			file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
			file_menu->Append(wxID_SAVEAS, "Save &As...\tCtrl+Shift+S");
			file_menu->AppendSeparator();
			file_menu->Append(wxID_CLOSE, "&Close\tCtrl+W");
		menu_bar->Append(file_menu, "&File");
		wxMenu* edit_menu = new wxMenu();
			edit_menu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
			edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Shift+Z");
			edit_menu->AppendSeparator();
			edit_menu->Append(wxID_CUT, "Cu&t\tCtrl+X");
			edit_menu->Append(wxID_COPY, "&Copy\tCtrl+C");
			edit_menu->Append(wxID_PASTE, "&Paste\tCtrl+V");
			edit_menu->Append(wxID_DELETE, "&Delete\tDel");
			edit_menu->AppendSeparator();
			edit_menu->Append(wxID_SELECTALL, "Select &All\tCtrl+A");
			edit_menu->Append(SEQER_ID_SELECT_NONE, "Select &None\tCtrl+Shift+A");
		menu_bar->Append(edit_menu, "&Edit");
		wxMenu* transport_menu = new wxMenu();
			transport_menu->Append(SEQER_ID_PLAY, "&Play\tSpace");
			transport_menu->Append(SEQER_ID_STOP, "&Stop\tShift+Space");
			transport_menu->Append(SEQER_ID_RECORD, "&Record\tCtrl+R");
			transport_menu->AppendSeparator();
#ifdef __WXOSX__
			transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tAlt+Right");
			transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tAlt+Left");
#else
			transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tCtrl+Right");
			transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tCtrl+Left");
#endif
			transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go to &Marker...\tCtrl+Shift+M");
			transport_menu->AppendSeparator();
			transport_menu->Append(SEQER_ID_PORTS, "P&orts...\tCtrl+Shift+O");
		menu_bar->Append(transport_menu, "Trans&port");
		wxMenu* tools_menu = new wxMenu();
			tools_menu->Append(SEQER_ID_EXTERNAL_UTILITY, "External &Utility...\tCtrl+Shift+U");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_RECORD_MACRO, "&Record Macro...\tCtrl+Shift+R");
			tools_menu->Append(SEQER_ID_MACROS, "&Macros...");
			tools_menu->AppendSeparator();
#ifdef __WXOSX__
			tools_menu->Append(wxID_PREFERENCES, "&Preferences...\tCtrl+,");
#else
			tools_menu->Append(wxID_PREFERENCES, "&Preferences...");
#endif
		menu_bar->Append(tools_menu, "&Tools");
		wxMenu* help_menu = new wxMenu();
			help_menu->Append(wxID_HELP_CONTENTS, "&User Manual");
			help_menu->Append(wxID_ABOUT, "&About Seqer");
		menu_bar->Append(help_menu, "&Help");
	this->SetMenuBar(menu_bar);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		if (!this->SaveChangesIfNeeded()) return;
		this->sequence_editor->New();
	}, wxID_NEW);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		(new TrackListWindow(this->application, this))->Show(true);
	}, SEQER_ID_NEW_TRACK_LIST_WINDOW);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		(new PianoRollWindow(this->application, this))->Show(true);
	}, SEQER_ID_NEW_PIANO_ROLL_WINDOW);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		(new EventListWindow(this->application, this))->Show(true);
	}, SEQER_ID_NEW_EVENT_LIST_WINDOW);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		if (!this->SaveChangesIfNeeded()) return;
		wxFileDialog* file_dialog = new wxFileDialog(this, "Open File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

		if (file_dialog->ShowModal() == wxID_OK)
		{
			if (!this->Load(file_dialog->GetPath()))
			{
				wxMessageBox("Cannot open the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
			}
		}

		delete file_dialog;
	}, wxID_OPEN);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->Save();
	}, wxID_SAVE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->SaveAs();
	}, wxID_SAVEAS);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->Close();
	}, wxID_CLOSE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& event) {
		std::set<Sequence*> sequences;
		bool is_modified = false;

		for (wxWindowList::compatibility_iterator window_iterator = wxTopLevelWindows.GetFirst(); window_iterator != NULL; window_iterator = window_iterator->GetNext())
		{
			Window* window = (Window*)(window_iterator->GetData());
			sequences.insert(window->sequence);
			if (window->sequence->IsModified()) is_modified = true;
		}

		if (sequences.size() == 1)
		{
			if (!is_modified || this->SaveChanges())
			{
				event.Skip();
			}
		}
		else
		{
			if (!is_modified || (wxMessageBox("Really quit without saving changes?", "Quit", wxOK | wxCANCEL) == wxOK))
			{
				event.Skip();
			}
		}
	}, wxID_EXIT);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		wxFileName help_file = wxFileName(wxStandardPaths::Get().GetResourcesDir(), "seqer.html");

		if (!help_file.FileExists())
		{
			help_file = wxFileName(wxStandardPaths::Get().GetExecutablePath());
			help_file.SetFullName("seqer.html");
		}

		if (help_file.FileExists()) wxLaunchDefaultBrowser(help_file.GetFullPath());
	}, wxID_HELP_CONTENTS);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		wxMessageBox("Seqer\na MIDI sequencer by Div Slomin\nprovided under terms of the BSD license", "About", wxOK);
	}, wxID_ABOUT);

	this->Bind(wxEVT_CHAR_HOOK, [=](wxKeyEvent& event) {
		int keycode = event.GetKeyCode();
		int modifiers = event.GetModifiers();

		if ((keycode == WXK_UP) && (modifiers == wxMOD_NONE))
		{
		}
		else if ((keycode == WXK_DOWN) && (modifiers == wxMOD_NONE))
		{
		}
		else if ((keycode == WXK_PAGEUP) && (modifiers == wxMOD_NONE))
		{
		}
		else if ((keycode == WXK_PAGEDOWN) && (modifiers == wxMOD_NONE))
		{
		}
		else if ((keycode == WXK_HOME) && (modifiers == wxMOD_CONTROL))
		{
		}
		else if ((keycode == WXK_END) && (modifiers == wxMOD_CONTROL))
		{
		}
		else if ((keycode == WXK_LEFT) && (modifiers == wxMOD_NONE))
		{
		}
		else if ((keycode == WXK_RIGHT) && (modifiers == wxMOD_NONE))
		{
		}
		else if ((keycode == WXK_HOME) && (modifiers == wxMOD_NONE))
		{
		}
		else if ((keycode == WXK_END) && (modifiers == wxMOD_NONE))
		{
		}
		else if ((keycode == WXK_UP) && (modifiers == wxMOD_ALT))
		{
		}
		else if ((keycode == WXK_DOWN) && (modifiers == wxMOD_ALT))
		{
		}
#ifdef __WXOSX__
		else if ((keycode == WXK_UP) && (modifiers == wxMOD_CONTROL))
		{
		}
		else if ((keycode == WXK_DOWN) && (modifiers == wxMOD_CONTROL))
		{
		}
		else if ((keycode == WXK_LEFT) && (modifiers == wxMOD_CONTROL))
		{
		}
		else if ((keycode == WXK_RIGHT) && (modifiers == wxMOD_CONTROL))
		{
		}
		else if ((keycode == 'A') && (modifiers == wxMOD_RAW_CONTROL))
		{
		}
		else if ((keycode == 'E') && (modifiers == wxMOD_RAW_CONTROL))
		{
		}
		else if ((keycode == 'P') && (modifiers == wxMOD_RAW_CONTROL))
		{
		}
		else if ((keycode == 'N') && (modifiers == wxMOD_RAW_CONTROL))
		{
		}
		else if ((keycode == 'B') && (modifiers == wxMOD_RAW_CONTROL))
		{
		}
		else if ((keycode == 'F') && (modifiers == wxMOD_RAW_CONTROL))
		{
		}
		else if ((keycode == 'D') && (modifiers == wxMOD_RAW_CONTROL))
		{
		}
		else if ((keycode == 'H') && (modifiers == wxMOD_RAW_CONTROL))
		{
		}
#endif
		else if ((keycode == '=') && ((modifiers & ~wxMOD_SHIFT) == wxMOD_CONTROL))
		{
			// make ctrl+plus ignore whether shift is pressed
			this->ProcessCommand(wxID_ZOOM_IN);
		}
		else if ((keycode == '-') && ((modifiers & ~wxMOD_SHIFT) == wxMOD_CONTROL))
		{
			// make ctrl+minus ignore whether shift is pressed
			this->ProcessCommand(wxID_ZOOM_OUT);
		}
		else if ((keycode == WXK_BACK) && (modifiers == wxMOD_NONE))
		{
			// make backspace be a synonym for delete
			this->ProcessCommand(wxID_DELETE);
		}
		else if ((keycode == ']') && (modifiers == wxMOD_NONE))
		{
			// although these are listed in the menu, they don't work automatically on WXGTK
			this->ProcessCommand(SEQER_ID_SMALL_INCREASE);
		}
		else if ((keycode == '[') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_SMALL_DECREASE);
		}
		else if ((keycode == ']') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_LARGE_INCREASE);
		}
		else if ((keycode == '[') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_LARGE_DECREASE);
		}
		else
		{
			event.Skip();
		}
	});

	this->Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
		if (!this->SaveChangesIfNeeded()) return;
		event.Skip();
	});

	this->sequence_editor = new SequenceEditor(this, (existing_window == NULL) ? NULL : existing_window->sequence_editor);
}

bool Window::SaveChangesIfNeeded()
{
	if (!(this->sequence_editor->IsModified() && this->sequence_editor->IsLastEditorForSequence())) return true;
	return this->SaveChanges();
}

bool Window::SaveChanges()
{
	switch (wxMessageBox("Do you want to save changes to the current file?", "Save Changes", wxYES_NO | wxCANCEL))
	{
		case wxYES:
		{
			return this->Save();
		}
		case wxCANCEL:
		{
			return false;
		}
		default:
		{
			return true;
		}
	}
}

bool Window::Save()
{
	if (this->sequence_editor->GetFilename() == wxEmptyString)
	{
		return this->SaveAs();
	}
	else
	{
		if (this->sequence_editor->Save())
		{
			return true;
		}
		else
		{
			wxMessageBox("Cannot save the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
			return false;
		}
	}
}

bool Window::SaveAs()
{
	wxFileDialog* file_dialog = new wxFileDialog(this, "Save File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_SAVE);
	bool result;

	if (file_dialog->ShowModal() == wxID_OK)
	{
		if (this->sequence_editor->SaveAs(file_dialog->GetPath()))
		{
			result = true;
		}
		else
		{
			wxMessageBox("Cannot save the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
			result = false;
		}
	}
	else
	{
		return false;
	}

	delete file_dialog;
	return result;
}

Window::~Window()
{
	delete this->sequence_editor;
}

