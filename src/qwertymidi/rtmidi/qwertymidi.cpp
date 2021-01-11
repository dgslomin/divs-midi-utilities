
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wx/wx.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-rtmidi.h>

static wxTextCtrl* text_box;
static int startup_error = 0;
static RtMidiOutPtr midi_out = NULL;
static int channel_number = 0;
static int program_number = -1;
static int velocity = 64;
static int map[128];
static int down[128];
static int transposition = 0;

static void handle_key_down(wxKeyEvent& event)
{
	text_box->SetValue(wxString::Format("<map key=\"%d\" />", event.GetKeyCode()));
}

static void handle_key_up(wxKeyEvent& event)
{
}

static void usage(wxString program_name)
{
	text_box->SetValue(wxString::Format("Usage:  %s [ --out <port> ] [ --channel <n> ] [ --program <n> ] [ --velocity <n> ] [ --map <filename> ]\n", program_name));
	startup_error = 1;
}

class Application: public wxApp
{
public:
	bool OnInit();
};

bool Application::OnInit()
{
	wxFrame *window = new wxFrame(NULL, wxID_ANY, "Qwertymidi", wxDefaultPosition, wxSize(640, 480));
	text_box = new wxTextCtrl(window, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);

	for (int i = 1; i < this->argc; i++)
	{
		if (strcmp(this->argv[i].char_str(), "--out") == 0)
		{
			if (++i == this->argc) usage(this->argv[0]);

			if ((midi_out = rtmidi_open_out_port((char *)("qwertymidi"), this->argv[i].char_str(), (char *)("qwertymidi"))) == NULL)
			{
				text_box->SetValue(wxString::Format("Error:  Cannot open MIDI output port \"%s\".\n", this->argv[i]));
				startup_error = 1;
			}
		}
		else if (strcmp(this->argv[i].char_str(), "--channel") == 0)
		{
			if (++i == this->argc) usage(this->argv[0]);
			channel_number = atoi(this->argv[i].char_str());
		}
		else if (strcmp(this->argv[i].char_str(), "--program") == 0)
		{
			if (++i == this->argc) usage(this->argv[0]);
			program_number = atoi(this->argv[i].char_str());
		}
		else if (strcmp(this->argv[i].char_str(), "--velocity") == 0)
		{
			if (++i == this->argc) usage(this->argv[0]);
			velocity = atoi(argv[i].char_str());
		}
		else if (strcmp(this->argv[i].char_str(), "--map") == 0)
		{
			if (++i == this->argc) usage(this->argv[0]);
			// TODO
		}
		else
		{
			usage(this->argv[0]);
		}
	}

	if (!startup_error)
	{
		text_box->Bind(wxEVT_KEY_DOWN, handle_key_down);
		text_box->Bind(wxEVT_KEY_UP, handle_key_up);
	}

	this->SetTopWindow(window);
	window->Show(true);
	return true;
}

wxIMPLEMENT_APP(Application);

