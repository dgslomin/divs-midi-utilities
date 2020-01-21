
#include <set>
#include <wx/wx.h>
#include <midifile.h>
#include "application.h"
#include "ids.h"
#include "sequence.h"
#include "window.h"

#ifndef __WXMSW__
#include "seqer.xpm"
#endif

Window::Window(Application* application, Window* existing_window): wxFrame((wxFrame*)(NULL), wxID_ANY, "Seqer", wxDefaultPosition, wxSize(640, 480))
{
	this->application = application;
	this->sequence = (existing_window == NULL) ? new Sequence() : existing_window->sequence;
	this->sequence->AddWindow(this);
	this->SetIcon(wxICON(seqer));
	this->CreateStatusBar();
	this->CreateMenuBar();
}

Window::~Window()
{
	this->sequence->RemoveWindow(this);
}

void Window::CreateMenuBar()
{
	wxMenuBar* menu_bar = new wxMenuBar();
	CreateFileMenu(menu_bar);
	CreateEditMenu(menu_bar);
	CreateTransportMenu(menu_bar);
	CreateToolsMenu(menu_bar);
	CreateHelpMenu(menu_bar);
	this->SetMenuBar(menu_bar);

	// This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
	this->Bind(wxEVT_MENU_HIGHLIGHT, [=](wxMenuEvent&) {});
}

void Window::CreateFileMenu(wxMenuBar* menu_bar)
{
	wxMenu* file_menu = new wxMenu();

	file_menu->Append(wxID_NEW, "&New\tCtrl+N");
	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->New(); }, wxID_NEW);

	file_menu->Append(SEQER_ID_NEW_WINDOW, "New &Window\tCtrl+Shift+N");
	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->NewWindow(); }, SEQER_ID_NEW_WINDOW);

	file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O");
	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->Open(); }, wxID_OPEN);

	file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->Save(); }, wxID_SAVE);

	file_menu->Append(wxID_SAVEAS, "Save &As...\tCtrl+Shift+S");
	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->SaveAs(); }, wxID_SAVEAS);

	file_menu->AppendSeparator();

	file_menu->Append(wxID_CLOSE, "&Close\tCtrl+W");
	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->Close(); }, wxID_CLOSE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->Quit(); }, wxID_EXIT);

	menu_bar->Append(file_menu, "&File");
}

void Window::CreateEditMenu(wxMenuBar* menu_bar)
{
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
	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->SelectAll(); }, wxID_SELECTALL);

	edit_menu->Append(SEQER_ID_SELECT_NONE, "Select &None\tCtrl+Shift+A");
	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) { this->SelectNone(); }, SEQER_ID_SELECT_NONE);

	edit_menu->Append(SEQER_ID_SELECT_TIME_RANGE, "Select Ti&me Range");
	menu_bar->Append(edit_menu, "&Edit");
}

void Window::CreateTransportMenu(wxMenuBar* menu_bar)
{
	wxMenu* transport_menu = new wxMenu();
	transport_menu->Append(SEQER_ID_PLAY, "&Play\tSpace");
	transport_menu->Append(SEQER_ID_STOP, "&Stop\tShift+Space");
	transport_menu->Append(SEQER_ID_RECORD, "&Record\tCtrl+R");
	transport_menu->AppendSeparator();
	transport_menu->Append(SEQER_ID_SET_PLAYBACK_POSITION, "Set Playback Position");
	transport_menu->Append(SEQER_ID_GO_TO_PLAYBACK_POSITION, "Go to Playback Position");
	transport_menu->AppendSeparator();
#ifdef __WXOSX__
	transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tAlt+Right");
	transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tAlt+Left");
#else
	transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tCtrl+Right");
	transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tCtrl+Left");
#endif
	transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go to &Marker...\tCtrl+Shift+M");
	menu_bar->Append(transport_menu, "Trans&port");
}

void Window::CreateToolsMenu(wxMenuBar* menu_bar)
{
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
}

void Window::CreateHelpMenu(wxMenuBar* menu_bar)
{
	wxMenu* help_menu = new wxMenu();
	help_menu->Append(wxID_HELP_CONTENTS, "&User Manual");
	help_menu->Append(wxID_ABOUT, "&About Seqer");
	menu_bar->Append(help_menu, "&Help");
}

bool Window::IsModified()
{
	return this->sequence->is_modified;
}

bool Window::IsLastWindowForSequence()
{
	return (this->sequence->windows.size() == 1);
}

wxString Window::GetFilename()
{
	return this->sequence->filename;
}

void Window::New()
{
	if (!this->SaveChangesIfNeeded()) return;
	this->sequence->RemoveWindow(this);
	this->sequence = new Sequence();
	this->sequence->AddWindow(this);
	this->RefreshData();
}

void Window::NewWindow()
{
	(new Window(this->application, this))->Show(true);
}

void Window::Open()
{
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
}

bool Window::Load(wxString filename)
{
	MidiFile_t new_midi_file = MidiFile_load((char *)(filename.ToStdString().c_str()));
	if (new_midi_file == NULL) return false;
	this->sequence->RemoveWindow(this);
	this->sequence = new Sequence();
	this->sequence->AddWindow(this);
	this->sequence->filename = filename;
	MidiFile_free(this->sequence->midi_file);
	this->sequence->midi_file = new_midi_file;
	this->RefreshData();
	return true;
}

bool Window::SaveChangesIfNeeded()
{
	if (!(this->IsModified() && this->IsLastWindowForSequence())) return true;
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
	if (this->GetFilename() == wxEmptyString)
	{
		return this->SaveAs();
	}
	else
	{
		if (this->sequence->Save())
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
		if (this->sequence->SaveAs(file_dialog->GetPath()))
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

void Window::Quit()
{
	std::set<Sequence*> sequences;
	bool is_modified = false;

	for (wxWindowList::compatibility_iterator window_iterator = wxTopLevelWindows.GetFirst(); window_iterator != NULL; window_iterator = window_iterator->GetNext())
	{
		Window* window = (Window*)(window_iterator->GetData());
		sequences.insert(window->sequence);
		if (window->IsModified()) is_modified = true;
	}

	if (sequences.size() == 1)
	{
		if (!is_modified || this->SaveChanges()) wxExit();
	}
	else
	{
		if (!is_modified || (wxMessageBox("Really quit without saving changes?", "Quit", wxOK | wxCANCEL) == wxOK)) wxExit();
	}
}

void Window::SelectAll()
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		MidiFileEvent_setSelected(midi_event, 1);
	}
}

void Window::SelectNone()
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		MidiFileEvent_setSelected(midi_event, 0);
	}
}

void Window::FocusPropertyEditor()
{
}

void Window::RefreshData()
{
}

void Window::RefreshDisplay()
{
}

int Window::GetXFromTick(long tick)
{
	if (this->use_linear_time)
	{
		return (int)(MidiFile_getTimeFromTick(this->sequence->midi_file, tick) * this->pixels_per_second) - this->scroll_x;
	}
	else
	{
		return (int)(MidiFile_getBeatFromTick(this->sequence->midi_file, tick) * this->pixels_per_beat) - this->scroll_x;
	}
}

long Window::GetTickFromX(int x)
{
	if (this->use_linear_time)
	{
		return MidiFile_getTickFromTime(this->sequence->midi_file, (float)(x + this->scroll_x) / this->pixels_per_second);
	}
	else
	{
		return MidiFile_getTickFromBeat(this->sequence->midi_file, (float)(x + this->scroll_x) / this->pixels_per_beat);
	}
}

