
#include <wx/wx.h>
#include <midifile.h>

class Application: public wxApp
{
private:
	wxTextCtrl* dirTextBox;
	wxListBox* filesListBox;
	wxStaticText* timeLabel;
	wxTextCtrl* nameTextBox;

public:
	bool OnInit();
};

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	wxFrame* window = new wxFrame((wxFrame*)(NULL), wxID_ANY, "Brainstorm Organizer", wxDefaultPosition, wxSize(640, 480));
	this->SetTopWindow(window);

	wxFlexGridSizer* outerSizer = new wxFlexGridSizer(2);
	outerSizer->AddGrowableCol(0);
	outerSizer->AddGrowableRow(1);
	window->SetSizer(outerSizer);

	this->dirTextBox = new wxTextCtrl(window, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	outerSizer->Add(this->dirTextBox, 0, wxEXPAND | wxALL, 4);

	wxButton* refreshButton = new wxButton(window, wxID_ANY, "&Refresh");
	outerSizer->Add(refreshButton, 0, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM, 4);

	this->filesListBox = new wxListBox(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_ALWAYS_SB);
	outerSizer->Add(this->filesListBox, 1, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 4);

	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	outerSizer->Add(rightSizer, 0, wxEXPAND);

	rightSizer->AddStretchSpacer();

	wxButton* upButton = new wxButton(window, wxID_ANY, "&Up");
	rightSizer->Add(upButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* downButton = new wxButton(window, wxID_ANY, "&Down");
	rightSizer->Add(downButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddStretchSpacer();

	wxButton* playButton = new wxButton(window, wxID_ANY, "&Play");
	rightSizer->Add(playButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* stopButton = new wxButton(window, wxID_ANY, "&Stop");
	rightSizer->Add(stopButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* backButton = new wxButton(window, wxID_ANY, "&Back");
	rightSizer->Add(backButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* forwardButton = new wxButton(window, wxID_ANY, "&Forward");
	rightSizer->Add(forwardButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddSpacer(4);
	this->timeLabel = new wxStaticText(window, wxID_ANY, "0 of 0 s");
	rightSizer->Add(this->timeLabel, 0, wxALIGN_CENTER | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddStretchSpacer();

	this->nameTextBox = new wxTextCtrl(window, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	outerSizer->Add(this->nameTextBox, 1, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 4);

	wxButton* renameButton = new wxButton(window, wxID_ANY, "Re&name");
	outerSizer->Add(renameButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	window->Show(true);
	return true;
}

