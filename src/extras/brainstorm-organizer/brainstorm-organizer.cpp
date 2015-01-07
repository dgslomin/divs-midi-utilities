
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
	void OnRefresh(wxCommandEvent& event);
	void OnUp(wxCommandEvent& event);
	void OnDown(wxCommandEvent& event);
	void OnPlay(wxCommandEvent& event);
	void OnStop(wxCommandEvent& event);
	void OnBack(wxCommandEvent& event);
	void OnForward(wxCommandEvent& event);
	void OnRename(wxCommandEvent& event);
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
	this->dirTextBox->Bind(wxEVT_TEXT_ENTER, &Application::OnRefresh, this);
	outerSizer->Add(this->dirTextBox, 0, wxEXPAND | wxALL, 4);

	wxButton* refreshButton = new wxButton(window, wxID_ANY, "&Refresh");
	refreshButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnRefresh, this);
	outerSizer->Add(refreshButton, 0, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM, 4);

	this->filesListBox = new wxListBox(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_ALWAYS_SB);
	outerSizer->Add(this->filesListBox, 1, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 4);

	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	outerSizer->Add(rightSizer, 0, wxEXPAND);

	rightSizer->AddStretchSpacer();

	wxButton* upButton = new wxButton(window, wxID_ANY, "&Up");
	upButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnUp, this);
	rightSizer->Add(upButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* downButton = new wxButton(window, wxID_ANY, "&Down");
	downButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnDown, this);
	rightSizer->Add(downButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddStretchSpacer();

	wxButton* playButton = new wxButton(window, wxID_ANY, "&Play");
	playButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnPlay, this);
	rightSizer->Add(playButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* stopButton = new wxButton(window, wxID_ANY, "&Stop");
	stopButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnStop, this);
	rightSizer->Add(stopButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* backButton = new wxButton(window, wxID_ANY, "&Back");
	backButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnBack, this);
	rightSizer->Add(backButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* forwardButton = new wxButton(window, wxID_ANY, "&Forward");
	forwardButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnForward, this);
	rightSizer->Add(forwardButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddSpacer(4);
	this->timeLabel = new wxStaticText(window, wxID_ANY, "0 of 0 s");
	rightSizer->Add(this->timeLabel, 0, wxALIGN_CENTER | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddStretchSpacer();

	this->nameTextBox = new wxTextCtrl(window, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	this->nameTextBox->Bind(wxEVT_TEXT_ENTER, &Application::OnRename, this);
	outerSizer->Add(this->nameTextBox, 1, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 4);

	wxButton* renameButton = new wxButton(window, wxID_ANY, "Re&name");
	renameButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnRename, this);
	outerSizer->Add(renameButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	window->Show(true);
	return true;
}

void Application::OnRefresh(wxCommandEvent& WXUNUSED(event))
{
	this->timeLabel->SetLabelText("refresh");
	this->timeLabel->SendSizeEventToParent();
}

void Application::OnUp(wxCommandEvent& WXUNUSED(event))
{
	this->timeLabel->SetLabelText("up");
	this->timeLabel->SendSizeEventToParent();
}

void Application::OnDown(wxCommandEvent& WXUNUSED(event))
{
	this->timeLabel->SetLabelText("down");
	this->timeLabel->SendSizeEventToParent();
}

void Application::OnPlay(wxCommandEvent& WXUNUSED(event))
{
	this->timeLabel->SetLabelText("play");
	this->timeLabel->SendSizeEventToParent();
}

void Application::OnStop(wxCommandEvent& WXUNUSED(event))
{
	this->timeLabel->SetLabelText("stop");
	this->timeLabel->SendSizeEventToParent();
}

void Application::OnBack(wxCommandEvent& WXUNUSED(event))
{
	this->timeLabel->SetLabelText("back");
	this->timeLabel->SendSizeEventToParent();
}

void Application::OnForward(wxCommandEvent& WXUNUSED(event))
{
	this->timeLabel->SetLabelText("forward");
	this->timeLabel->SendSizeEventToParent();
}

void Application::OnRename(wxCommandEvent& WXUNUSED(event))
{
	this->timeLabel->SetLabelText("rename");
	this->timeLabel->SendSizeEventToParent();
}

