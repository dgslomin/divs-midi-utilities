
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <wx/dir.h>
#include <wx/regex.h>
#include <wx/wx.h>
#include <midifile.h>
#include <midifile-player.h>

#define SKIP_TIME 10

class Application: public wxApp
{
private:
	wxFrame* window;
	wxListBox* filesListBox;
	wxStaticText* timeLabel;
	wxTextCtrl* nameTextBox;
	std::string dir;
	wxString oldName;
	MidiFilePlayer_t midiFilePlayer;
	MidiFile_t midiFile;
	int midiFileLength;
	int midiFileCurrentTime;

public:
	bool OnInit();
	void OnSelect(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnRefresh(wxCommandEvent& event);
	void OnUp(wxCommandEvent& event);
	void OnDown(wxCommandEvent& event);
	void OnPlay(wxCommandEvent& event);
	void OnStop(wxCommandEvent& event);
	void OnBack(wxCommandEvent& event);
	void OnForward(wxCommandEvent& event);
	void OnRename(wxCommandEvent& event);
	void OnNameTextBoxClick(wxMouseEvent& event);
	void OnUpdateTimeLabel(wxThreadEvent& event);
	static void OnMidiFileEvent(MidiFileEvent_t event, void *user_data);
	void ListFiles();
	void SelectFile();
	void UpdateTimeLabel();
};

class File
{
public:
	wxString filename;
	long timestamp;

	File(wxString filename, long timestamp)
	{
		this->filename = filename;
		this->timestamp = timestamp;
	}
};

wxDEFINE_EVENT(wxEVT_COMMAND_UPDATE_TIME_LABEL, wxThreadEvent);
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
	this->filesListBox->Bind(wxEVT_LISTBOX, &Application::OnSelect, this);
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
	this->nameTextBox->Bind(wxEVT_LEFT_DOWN, &Application::OnNameTextBoxClick, this);
	outerSizer->Add(this->nameTextBox, 0, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 4);

	wxButton* renameButton = new wxButton(this->window, wxID_ANY, "Re&name");
	renameButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Application::OnRename, this);
	outerSizer->Add(renameButton, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 4);

	this->Bind(wxEVT_COMMAND_UPDATE_TIME_LABEL, &Application::OnUpdateTimeLabel, this);

	this->midiFilePlayer = MidiFilePlayer_new(&Application::OnMidiFileEvent, this);
	this->midiFile = NULL;

	this->dir = ".";
	this->ListFiles();

	this->window->Show(true);
	return true;
}

void Application::OnSelect(wxCommandEvent& WXUNUSED(event))
{
	this->SelectFile();
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
		selection = this->filesListBox->GetCount() - 1;
	}
	else
	{
		selection--;
	}

	if ((selection >= 0) && (this->filesListBox->GetCount() > 0))
	{
		this->filesListBox->SetSelection(selection);
		this->SelectFile();
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
		this->SelectFile();
	}
}

void Application::OnPlay(wxCommandEvent& WXUNUSED(event))
{
	if (MidiFilePlayer_isRunning(this->midiFilePlayer))
	{
		MidiFilePlayer_pause(this->midiFilePlayer);
	}
	else
	{
		MidiFilePlayer_play(this->midiFilePlayer);
	}
}

void Application::OnStop(wxCommandEvent& WXUNUSED(event))
{
	MidiFilePlayer_pause(this->midiFilePlayer);
	MidiFilePlayer_setTick(this->midiFilePlayer, 0);
    this->midiFileCurrentTime = 0;
    this->UpdateTimeLabel();
}

void Application::OnBack(wxCommandEvent& WXUNUSED(event))
{
	float currentTime = MidiFile_getTimeFromTick(this->midiFile, MidiFilePlayer_getTick(this->midiFilePlayer));
	float newTime = currentTime - SKIP_TIME;
	if (newTime < 0) newTime = 0;
	MidiFilePlayer_setTick(this->midiFilePlayer, MidiFile_getTickFromTime(this->midiFile, newTime));
	this->midiFileCurrentTime = (int)(newTime);
	this->UpdateTimeLabel();
}

void Application::OnForward(wxCommandEvent& WXUNUSED(event))
{
	float currentTime = MidiFile_getTimeFromTick(this->midiFile, MidiFilePlayer_getTick(this->midiFilePlayer));
	float newTime = currentTime + SKIP_TIME;
	MidiFilePlayer_setTick(this->midiFilePlayer, MidiFile_getTickFromTime(this->midiFile, newTime));
	this->midiFileCurrentTime = (int)(newTime);
	this->UpdateTimeLabel();
}

void Application::OnRename(wxCommandEvent& WXUNUSED(event))
{
	this->nameTextBox->SetFocus();
	wxRegEx regex("\\d{14}.mid$", wxRE_ADVANCED);

	if (regex.Matches(this->oldName))
	{
		wxString newName = this->nameTextBox->GetValue() + regex.GetMatch(this->oldName, 0);
		wxRenameFile(this->dir + "/" + this->oldName, this->dir + "/" + newName);
		this->filesListBox->SetString(this->filesListBox->GetSelection(), newName);
		this->SelectFile();
	}
}

void Application::OnNameTextBoxClick(wxMouseEvent& event)
{
	if (this->nameTextBox->HasFocus())
	{
		event.Skip();
	}
	else
	{
		this->nameTextBox->SetFocus();
		this->nameTextBox->SelectAll();
	}
}

void Application::OnUpdateTimeLabel(wxThreadEvent& WXUNUSED(event))
{
	this->UpdateTimeLabel();
}

void Application::OnMidiFileEvent(MidiFileEvent_t event, void* userData)
{
	Application* application = static_cast<Application*>(userData);
	int eventTime = (int)(MidiFile_getTimeFromTick(application->midiFile, MidiFileEvent_getTick(event)));

	if (eventTime != application->midiFileCurrentTime)
	{
		application->midiFileCurrentTime = eventTime;
		wxQueueEvent(application, new wxThreadEvent(wxEVT_COMMAND_UPDATE_TIME_LABEL));
	}
}

bool CompareFiles(File* first, File* second)
{
	return first->timestamp < second->timestamp;
}

void Application::ListFiles()
{
   wxDir dir(this->dir);

	if (dir.IsOpened())
	{
		std::vector<File*> files;
		wxRegEx regex("(\\d{14})\\.mid$", wxRE_ADVANCED);
		wxString filename;

		for (bool hasMoreFiles = dir.GetFirst(&filename, "", wxDIR_FILES); hasMoreFiles; hasMoreFiles = dir.GetNext(&filename))
		{
			if (regex.Matches(filename))
			{
				files.push_back(new File(filename, atol(regex.GetMatch(filename, 1))));
			}
		}

		std::sort(files.begin(), files.end(), CompareFiles);
		this->filesListBox->Clear();

		for (std::vector<File*>::iterator file = files.begin(); file != files.end(); file++)
		{
			this->filesListBox->Append((*file)->filename);
		}

		int count = this->filesListBox->GetCount();

		if (count > 0)
		{
			this->filesListBox->SetSelection(count - 1);
			this->SelectFile();
		}
	}
}

void Application::SelectFile()
{
	int selectedFileNumber = this->filesListBox->GetSelection();

	if (selectedFileNumber != wxNOT_FOUND)
	{
		wxString selectedFile = this->filesListBox->GetString(selectedFileNumber);
		wxRegEx regex("^(.*)\\d{14}\\.mid$", wxRE_ADVANCED);

		if (regex.Matches(selectedFile))
		{
			this->nameTextBox->SetValue(regex.GetMatch(selectedFile, 1));
			this->nameTextBox->SelectAll();
			this->oldName = selectedFile;

			if (this->midiFile != NULL) MidiFile_free(this->midiFile);
			this->midiFile = MidiFile_load((char *)(static_cast<const char *>((this->dir + "/" + this->oldName).c_str())));
			MidiFilePlayer_setMidiFile(this->midiFilePlayer, this->midiFile);
			this->midiFileLength = (int)(MidiFile_getTimeFromTick(this->midiFile, MidiFileEvent_getTick(MidiFile_getLastEvent(this->midiFile))));
			this->midiFileCurrentTime = 0;
			this->UpdateTimeLabel();
		}
	}
}

void Application::UpdateTimeLabel()
{
	wxString timeLabelText;
	timeLabelText.Printf("%d of %d s", this->midiFileCurrentTime, this->midiFileLength);
	this->timeLabel->SetLabelText(timeLabelText);
	this->timeLabel->SendSizeEventToParent();
}

