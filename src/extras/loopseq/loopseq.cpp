
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wx/wx.h>

class LoopSeq : public wxApp
{
	virtual bool OnInit();
};

class MainWindow : public wxFrame
{
public:
	MainWindow(const wxString& title, const wxPoint& position, const wxSize& size);
	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
private:
	DECLARE_EVENT_TABLE();
};

enum
{
	QUIT_EVENT_ID = 1,
	ABOUT_EVENT_ID
};

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(QUIT_EVENT_ID, MainWindow::OnQuit)
	EVT_MENU(ABOUT_EVENT_ID, MainWindow::OnAbout)
END_EVENT_TABLE()

IMPLEMENT_APP(LoopSeq)

bool LoopSeq::OnInit()
{
	MainWindow* main_window = new MainWindow("Div's Loop Sequencer", wxPoint(50, 50), wxSize(640, 480));
	main_window->Show(TRUE);
	this->SetTopWindow(main_window);
	return TRUE;
}

MainWindow::MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame((wxFrame*)(NULL), -1, title, pos, size)
{
	wxMenu* file_menu = new wxMenu();
	file_menu->Append(ABOUT_EVENT_ID, "&About...");
	file_menu->AppendSeparator();
	file_menu->Append(QUIT_EVENT_ID, "E&xit");

	wxMenuBar* menu_bar = new wxMenuBar();
	menu_bar->Append(file_menu, "&File");
	this->SetMenuBar(menu_bar);

	this->CreateStatusBar();
	this->SetStatusText("Div's Loop Sequencer");
}

void MainWindow::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	this->Close(TRUE);
}

void MainWindow::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("An interactive, looping step sequencer for MIDI.\nby Div Slomin, 2004", "About Div's Loop Sequencer", wxOK | wxICON_INFORMATION);
}

