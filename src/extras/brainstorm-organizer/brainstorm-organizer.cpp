
#include <stdlib.h>
#include <wx/dir.h>
#include <wx/regex.h>
#include <wx/wx.h>
#include <midifile.h>

class Application: public wxApp
{
private:
	wxFrame* window;
	wxListBox* filesListBox;
	wxStaticText* timeLabel;
	wxTextCtrl* nameTextBox;
	std::string dir;

public:
	bool OnInit();
	void OnOpen(wxCommandEvent& event);
	void OnRefresh(wxCommandEvent& event);
	void OnUp(wxCommandEvent& event);
	void OnDown(wxCommandEvent& event);
	void OnPlay(wxCommandEvent& event);
	void OnStop(wxCommandEvent& event);
	void OnBack(wxCommandEvent& event);
	void OnForward(wxCommandEvent& event);
	void OnRename(wxCommandEvent& event);
	void ListFiles();
};

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	this->window = new wxFrame((wxFrame*)(NULL), wxID_ANY, "Brainstorm Organizer", wxDefaultPosition, wxSize(640, 480));
	this->SetTopWindow(this->window);

	wxFlexGridSizer* outerSizer = new wxFlexGridSizer(2);
	outerSizer->AddGrowableCol(0);
	outerSizer->AddGrowableRow(0);
	this->window->SetSizer(outerSizer);

	this->filesListBox = new wxListBox(this->window, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_ALWAYS_SB);
	outerSizer->Add(this->filesListBox, 1, wxEXPAND | wxALL, 4);

	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	outerSizer->Add(rightSizer, 0, wxEXPAND);

	wxButton* openButton = new wxButton(this->window, wxID_ANY, "&Open");
	openButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnOpen, this);
	rightSizer->Add(openButton, 0, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM, 4);

	wxButton* refreshButton = new wxButton(this->window, wxID_ANY, "&Refresh");
	refreshButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnRefresh, this);
	rightSizer->Add(refreshButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddSpacer(32);

	wxButton* upButton = new wxButton(this->window, wxID_ANY, "&Up");
	upButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnUp, this);
	rightSizer->Add(upButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* downButton = new wxButton(this->window, wxID_ANY, "&Down");
	downButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnDown, this);
	rightSizer->Add(downButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddSpacer(32);

	wxButton* playButton = new wxButton(this->window, wxID_ANY, "&Play");
	playButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnPlay, this);
	rightSizer->Add(playButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* stopButton = new wxButton(this->window, wxID_ANY, "&Stop");
	stopButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnStop, this);
	rightSizer->Add(stopButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* backButton = new wxButton(this->window, wxID_ANY, "&Back");
	backButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnBack, this);
	rightSizer->Add(backButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	wxButton* forwardButton = new wxButton(this->window, wxID_ANY, "&Forward");
	forwardButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnForward, this);
	rightSizer->Add(forwardButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	rightSizer->AddSpacer(4);
	this->timeLabel = new wxStaticText(this->window, wxID_ANY, "0 of 0 s");
	rightSizer->Add(this->timeLabel, 0, wxALIGN_CENTER | wxRIGHT | wxBOTTOM, 4);

	this->nameTextBox = new wxTextCtrl(this->window, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	this->nameTextBox->Bind(wxEVT_TEXT_ENTER, &Application::OnRename, this);
	outerSizer->Add(this->nameTextBox, 1, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 4);

	wxButton* renameButton = new wxButton(this->window, wxID_ANY, "Re&name");
	renameButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnRename, this);
	outerSizer->Add(renameButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	this->dir = ".";
	this->ListFiles();

	this->window->Show(true);
	return true;
}

void Application::OnOpen(wxCommandEvent& WXUNUSED(event))
{
	wxDirDialog dialog(this->window, wxDirSelectorPromptStr, "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (dialog.ShowModal() == wxID_OK)
	{
		this->dir = dialog.GetPath();
		this->ListFiles();
	}
}

void Application::OnRefresh(wxCommandEvent& WXUNUSED(event))
{
	this->ListFiles();
}

void Application::OnUp(wxCommandEvent& WXUNUSED(event))
{
	int selection = this->filesListBox->GetSelection();

	if (selection == wxNOT_FOUND)
	{
		selection = 0;
	}
	else
	{
		selection--;
	}

	if ((selection >= 0) && (this->filesListBox->GetCount() > 0))
	{
		this->filesListBox->SetSelection(selection);
	}
}

void Application::OnDown(wxCommandEvent& WXUNUSED(event))
{
	int selection = this->filesListBox->GetSelection();

	if (selection == wxNOT_FOUND)
	{
		selection = 0;
	}
	else
	{
		selection++;
	}

	if (selection < this->filesListBox->GetCount())
	{
		this->filesListBox->SetSelection(selection);
	}
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

int CompareFilenames(const wxString& first, const wxString& second)
{
	wxRegEx regex("(\\d{14})\\.mid$", wxRE_ADVANCED);
    long firstDate = 0;
    long secondDate = 0;

    if (regex.Matches(first))
    {
        firstDate = atol(regex.GetMatch(first, 1));
    }

    if (regex.Matches(second))
    {
        secondDate = atol(regex.GetMatch(second, 1));
    }

    return firstDate < secondDate ? -1 : firstDate > secondDate ? 1 : 0;
}

void Application::ListFiles()
{
   wxDir dir(this->dir);

	if (dir.IsOpened())
	{
        wxArrayString filenames;
		wxRegEx regex("\\d{14}\\.mid$", wxRE_ADVANCED);
		wxString filename;

		for (bool hasMoreFiles = dir.GetFirst(&filename, "", wxDIR_FILES); hasMoreFiles; hasMoreFiles = dir.GetNext(&filename))
		{
			if (regex.Matches(filename))
			{
				filenames.Add(filename);
			}
		}

        filenames.Sort(CompareFilenames);
		this->filesListBox->Clear();
		this->filesListBox->Append(filenames);

        int count = this->filesListBox->GetCount();

        if (count > 0)
		{
			this->filesListBox->SetSelection(count - 1);
		}
	}
}

