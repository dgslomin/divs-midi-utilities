
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wx/wx.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-rtmidi.h>

static wxTextCtrl* text_box;

static void handle_key_down(wxKeyEvent& event)
{
	text_box->SetValue(wxString::Format("<map key=\"%d\" />", event.GetKeyCode()));
}

static void handle_key_up(wxKeyEvent& event)
{
}

static void usage(char *program_name)
{
	text_box->SetValue(wxString::Format("Usage:  %s --out <port> [ --channel <n> ] [ --program <n> ] [ --velocity <n> ] [ --map <filename> ]\n", program_name));
}

class Application: public wxApp
{
public:
	bool OnInit();
};

bool Application::OnInit()
{
	wxFrame *window = new wxFrame(NULL, wxID_ANY, "Qwertymidi", wxDefaultPosition, wxSize(640, 480));
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	text_box = new wxTextCtrl(window, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	sizer->Add(text_box, wxSizerFlags().Proportion(1).Expand().Border());
	window->SetSizer(sizer);
	text_box->Bind(wxEVT_KEY_DOWN, handle_key_down);
	text_box->Bind(wxEVT_KEY_UP, handle_key_up);
	this->SetTopWindow(window);
	window->Show(true);

	for (int i = 1; i < this->argc; i++)
	{
		else if (strcmp(this->argv[i], "--out") == 0)
		{
			if (++i == this->argc)
			{
				usage(this->argv[0]);
			}
			else
			{
				if ((midi_out = rtmidi_open_out_port("notemap", this->argv[i], "notemap")) == NULL)
				{
					fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", this->argv[i]);
					exit(1);
				}
			}
		}
		else if (strcmp(this->argv[i], "--map") == 0)
		{
		}
		else
		{
			usage(this->argv[0]);
		}
	}

	return true;
}

wxIMPLEMENT_APP(Application);

