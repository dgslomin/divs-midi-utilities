
#include <wx/wx.h>

class Seqer: public wxApp
{
public:
	bool OnInit();
	void OnMenuHighlight(wxMenuEvent& event);
	void OnExit(wxCommandEvent& event);
	wxFrame* main_window;
};

IMPLEMENT_APP(Seqer)

bool Seqer::OnInit()
{
	this->main_window = new wxFrame((wxFrame*)(NULL), -1, "Seqer", wxDefaultPosition, wxSize(640, 480));
	this->SetTopWindow(this->main_window);

	wxMenuBar* menu_bar = new wxMenuBar();
	this->main_window->SetMenuBar(menu_bar);
	this->main_window->Connect(wxEVT_MENU_HIGHLIGHT, wxMenuEventHandler(Seqer::OnMenuHighlight));
	wxMenu* file_menu = new wxMenu();
	menu_bar->Append(file_menu, "&File");
	file_menu->Append(wxID_NEW, "&New\tCtrl+N");
	file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O");
	file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
	file_menu->Append(wxID_SAVEAS, "Save &As...");
	file_menu->Append(wxID_REVERT, "&Revert");
	file_menu->AppendSeparator();
	file_menu->Append(wxID_EXIT, "E&xit");
	this->Connect(wxID_EXIT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seqer::OnExit));
	wxMenu* edit_menu = new wxMenu();
	menu_bar->Append(edit_menu, "&Edit");
	edit_menu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
	edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Y");
	edit_menu->AppendSeparator();
	edit_menu->Append(wxID_CUT, "Cu&t\tCtrl+X");
	edit_menu->Append(wxID_COPY, "&Copy\tCtrl+C");
	edit_menu->Append(wxID_PASTE, "&Paste\tCtrl+V");
	wxMenu* view_menu = new wxMenu();
	menu_bar->Append(view_menu, "&View");
	wxMenu* help_menu = new wxMenu();
	menu_bar->Append(help_menu, "&Help");
	help_menu->Append(wxID_ABOUT, "&About Seqer");

	this->main_window->CreateStatusBar();

	this->main_window->Show(TRUE);
	return TRUE;
}

void Seqer::OnMenuHighlight(wxMenuEvent& WXUNUSED(event))
{
	// This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
}

void Seqer::OnExit(wxCommandEvent& WXUNUSED(event))
{
	this->main_window->Close(TRUE);
}

