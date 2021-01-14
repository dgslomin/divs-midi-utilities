
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/joystick.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-rtmidi.h>
#include <midiutil-wx.h>

static wxTextCtrl* text_box;
static wxTimer* timer;
static int startup_error = 0;
static RtMidiOutPtr midi_out = NULL;
static int channel_number = 0;
static int number_of_joysticks = 0;
static wxJoystick* joystick_array[2];
static int axis_controller_number_array[2][6];
static int button_controller_number_array[2][64];
static int axis_last_value_array[2][6];
static int button_last_value_array[2][64];

static void send_control_change(int channel, int number, int value)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE];
		MidiUtilMessage_setControlChange(message, channel, number, value);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE);
	}
}

static int get_joystick_axis_min(wxJoystick* joystick, int axis_number)
{
	switch (axis_number) {
		case 0: return joystick->GetXMin();
		case 1: return joystick->GetYMin();
		case 2: return joystick->GetZMin();
		case 3: return joystick->GetRudderMin();
		case 4: return joystick->GetUMin();
		case 5: return joystick->GetVMin();
		default: return 0;
	}
}

static int get_joystick_axis_max(wxJoystick* joystick, int axis_number)
{
	switch (axis_number) {
		case 0: return joystick->GetXMax();
		case 1: return joystick->GetYMax();
		case 2: return joystick->GetZMax();
		case 3: return joystick->GetRudderMax();
		case 4: return joystick->GetUMax();
		case 5: return joystick->GetVMax();
		default: return 0;
	}
}

static void handle_timer(wxTimerEvent& event)
{
	wxString status_message = wxEmptyString;
	int joystick_number;

	for (joystick_number = 0; joystick_number < number_of_joysticks; joystick_number++)
	{
		wxJoystick* joystick = joystick_array[joystick_number];
		status_message.append(wxString::Format("joystick %d:\n\naxes:", joystick_number));

		{
			int number_of_axes = joystick->GetMaxAxes();
			int axis_number;

			for (axis_number = 0; axis_number < number_of_axes; axis_number++)
			{
				int position = joystick->GetPosition(axis_number);
				int min = get_joystick_axis_min(joystick, axis_number);
				int max = get_joystick_axis_max(joystick, axis_number);
				int value = (position - min) * 128 / (max - min);

				if (value != axis_last_value_array[joystick_number][axis_number])
				{
					int controller_number = axis_controller_number_array[joystick_number][axis_number];
					if (controller_number >= 0) send_control_change(channel_number, controller_number, value);
					axis_last_value_array[joystick_number][axis_number] = value;
				}

				status_message.append(wxString::Format("%s %d=%d", (axis_number == 0) ? "" : ",", axis_number, value));
			}
		}

		status_message.append("\n\nbuttons:");

		{
			int number_of_buttons = joystick->GetMaxButtons();
			int button_number;

			for (button_number = 0; button_number < number_of_buttons; button_number++)
			{
				int value = joystick->GetButtonState(button_number) ? 127 : 0;

				if (value != button_last_value_array[joystick_number][button_number])
				{
					int controller_number = button_controller_number_array[joystick_number][button_number];
					if (controller_number >= 0) send_control_change(channel_number, controller_number, value);
					button_last_value_array[joystick_number][button_number] = value;
				}

				status_message.append(wxString::Format("%s %d=%d", (button_number == 0) ? "" : ",", button_number, value));
			}
		}

		status_message.append("\n\n");
	}

	text_box->SetValue(status_message);
}

static void usage(wxString program_name)
{
	text_box->SetValue(wxString::Format("Usage: %s [ --out <n> ] [ --channel <n> ] [ --axis <joystick number> <axis number> <controller number> ] ... [ --button <joystick number> <button number> <controller number> ] ...\n", program_name));
	startup_error = 1;
}

class Application: public wxApp
{
public:
	bool OnInit();
	int OnExit();
};

bool Application::OnInit()
{
	int i;
	wxFrame *window = new wxFrame(NULL, wxID_ANY, "Joycc", wxDefaultPosition, wxSize(640, 480));
	text_box = new wxTextCtrl(window, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);
	timer = new wxTimer(text_box);
	text_box->Bind(wxEVT_TIMER, handle_timer);

	for (i = 1; i < this->argc; i++)
	{
		if (this->argv[i] == "--out")
		{
			if (++i == this->argc) usage(this->argv[0]);

			if ((midi_out = rtmidi_open_out_port((char *)("joycc"), this->argv[i].char_str(), (char *)("joycc"))) == NULL)
			{
				text_box->SetValue(wxString::Format("Error:  Cannot open MIDI output port \"%s\".\n", this->argv[i]));
				startup_error = 1;
			}
		}
		else if (this->argv[i] == "--channel")
		{
			if (++i == this->argc) usage(this->argv[0]);
			channel_number = wxAtoi(this->argv[i]);
		}
		else if (this->argv[i] == "--axis")
		{
			int joystick_number, axis_number, controller_number;
			if (++i == this->argc) usage(this->argv[0]);
			joystick_number = wxAtoi(this->argv[i]);
			if (++i == this->argc) usage(this->argv[0]);
			axis_number = wxAtoi(this->argv[i]);
			if (++i == this->argc) usage(this->argv[0]);
			controller_number = wxAtoi(this->argv[i]);
			axis_controller_number_array[joystick_number][axis_number] = controller_number;
		}
		else if (this->argv[i] == "--button")
		{
			int joystick_number, button_number, controller_number;
			if (++i == this->argc) usage(this->argv[0]);
			joystick_number = wxAtoi(this->argv[i]);
			if (++i == this->argc) usage(this->argv[0]);
			button_number = wxAtoi(this->argv[i]);
			if (++i == this->argc) usage(this->argv[0]);
			controller_number = wxAtoi(this->argv[i]);
			button_controller_number_array[joystick_number][button_number] = controller_number;
		}
		else
		{
			usage(this->argv[0]);
		}
	}

	if (!startup_error)
	{
		int joystick_number, axis_number, button_number;
		number_of_joysticks = wxJoystick::GetNumberJoysticks();

		for (joystick_number = 0; joystick_number < number_of_joysticks; joystick_number++)
		{
			wxJoystick* joystick = new wxJoystick(joystick_number);
			joystick_array[joystick_number] = joystick;
			for (axis_number = 0; axis_number < 6; axis_number++) axis_last_value_array[joystick_number][axis_number] = 0;
			for (button_number = 0; button_number < 64; button_number++) button_last_value_array[joystick_number][button_number] = 0;
		}

		timer->Start(100);
	}

	this->SetTopWindow(window);
	window->Show(true);
	return true;
}

int Application::OnExit()
{
	if (midi_out != NULL) rtmidi_close_port(midi_out);
	delete timer;

	while (number_of_joysticks > 0)
	{
		wxJoystick* joystick = joystick_array[--number_of_joysticks];
		delete joystick;
	}

	return 0;
}

wxIMPLEMENT_APP(Application);

