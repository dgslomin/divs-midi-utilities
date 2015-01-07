
#include <wx/wx.h>
#include <midifile.h>

class Application: public wxApp
{
public:
	wxListBox* filesListBox;
	wxTextCtrl* nameTextBox;

	bool OnInit();
};

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	wxFrame* window = new wxFrame((wxFrame*)(NULL), wxID_ANY, "Brainstorm Organizer", wxDefaultPosition, wxSize(640, 480));
	this->SetTopWindow(window);

	wxFlexGridSizer* outerSizer = new wxFlexGridSizer(2);
	outerSizer->AddGrowableCol(0);
	outerSizer->AddGrowableRow(0);
	window->SetSizer(outerSizer);

	this->filesListBox = new wxListBox(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_ALWAYS_SB);
	outerSizer->Add(this->filesListBox, 1, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 4);

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
	outerSizer->Add(buttonsSizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM, 4);

	wxButton* previousButton = new wxButton(window, wxID_ANY, "&Previous");
	buttonsSizer->Add(previousButton, 0, wxEXPAND);

	wxButton* nextButton = new wxButton(window, wxID_ANY, "&Next");
	buttonsSizer->Add(nextButton, 0, wxEXPAND | wxTOP, 4);

	this->nameTextBox = new wxTextCtrl(window, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	outerSizer->Add(this->nameTextBox, 1, wxEXPAND | wxALL, 4);

	wxButton* renameButton = new wxButton(window, wxID_ANY, "&Rename");
	outerSizer->Add(renameButton, 0, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM, 4);

	window->Show(true);
	return true;
}

