
#include <wx/wx.h>
#include <wx/aboutdlg.h>
#include <wx/fontpicker.h>
#include <midifile.h>
#include "seqer.h"
#include "sequence-editor.h"
#include "step-size.h"
#include "event-type.h"

enum
{
	SEQER_ID_NEW_WINDOW = wxID_HIGHEST + 1,
	SEQER_ID_SELECT_CURRENT,
	SEQER_ID_SELECT_NONE,
	SEQER_ID_ENTER_VALUE,
	SEQER_ID_SMALL_INCREASE,
	SEQER_ID_SMALL_DECREASE,
	SEQER_ID_LARGE_INCREASE,
	SEQER_ID_LARGE_DECREASE,
	SEQER_ID_QUANTIZE,
	SEQER_ID_EDIT_COLUMN_1,
	SEQER_ID_EDIT_COLUMN_2,
	SEQER_ID_EDIT_COLUMN_3,
	SEQER_ID_EDIT_COLUMN_4,
	SEQER_ID_EDIT_COLUMN_5,
	SEQER_ID_EDIT_COLUMN_6,
	SEQER_ID_EDIT_COLUMN_7,
	SEQER_ID_STEP_SIZE,
	SEQER_ID_FILTER,
	SEQER_ID_INSERT_NOTE_A,
	SEQER_ID_INSERT_NOTE_B,
	SEQER_ID_INSERT_NOTE_C,
	SEQER_ID_INSERT_NOTE_D,
	SEQER_ID_INSERT_NOTE_E,
	SEQER_ID_INSERT_NOTE_F,
	SEQER_ID_INSERT_NOTE_G,
	SEQER_ID_INSERT_CONTROL_CHANGE,
	SEQER_ID_INSERT_PROGRAM_CHANGE,
	SEQER_ID_INSERT_AFTERTOUCH,
	SEQER_ID_INSERT_PITCH_BEND,
	SEQER_ID_INSERT_SYSTEM_EXCLUSIVE,
	SEQER_ID_INSERT_TEXT,
	SEQER_ID_INSERT_LYRIC,
	SEQER_ID_INSERT_MARKER,
	SEQER_ID_INSERT_PORT,
	SEQER_ID_INSERT_TEMPO,
	SEQER_ID_INSERT_TIME_SIGNATURE,
	SEQER_ID_INSERT_KEY_SIGNATURE,
	SEQER_ID_PLAY,
	SEQER_ID_RECORD,
	SEQER_ID_STOP,
	SEQER_ID_STEP_RECORD,
	SEQER_ID_NEXT_MARKER,
	SEQER_ID_PREVIOUS_MARKER,
	SEQER_ID_GO_TO_MARKER,
	SEQER_ID_PORTS,
	SEQER_ID_EXTERNAL_UTILITY,
	SEQER_ID_RECORD_MACRO,
	SEQER_ID_MACROS,
	SEQER_ID_SETTINGS,
	SEQER_ID_HIGHEST
};

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
#if defined(__WXOSX__)
	this->default_event_list_font = wxFont(wxFontInfo(10).FaceName("Lucida Grande"));
#else
	this->default_event_list_font = *wxNORMAL_FONT;
#endif
	this->default_piano_roll_first_note = 21; // standard piano range
	this->default_piano_roll_last_note = 108;
	this->default_piano_roll_key_width = 3;

	Window* window = new Window(this);
	this->SetTopWindow(window);
	window->Show(true);

	if (this->argc > 1)
	{
		window->sequence_editor->Load(this->argv[1]);
	}

	return true;
}

Window::Window(Application* application): wxFrame((wxFrame*)(NULL), wxID_ANY, "Seqer", wxDefaultPosition, wxSize(640, 480))
{
	this->application = application;

	wxMenuBar* menu_bar = new wxMenuBar(); this->SetMenuBar(menu_bar); this->Bind(wxEVT_MENU_HIGHLIGHT, &Window::OnMenuHighlight, this);
		wxMenu* file_menu = new wxMenu(); menu_bar->Append(file_menu, "&File");
			file_menu->Append(wxID_NEW, "&New\tCtrl+N");
			file_menu->Append(SEQER_ID_NEW_WINDOW, "New &Window\tCtrl+Shift+N");
			file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnFileOpen, this, wxID_OPEN);
			file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
			file_menu->Append(wxID_SAVEAS, "Save &As...");
			file_menu->Append(wxID_REVERT, "&Revert");
			file_menu->AppendSeparator();
			file_menu->Append(wxID_CLOSE, "&Close\tCtrl+W"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnClose, this, wxID_CLOSE);
		wxMenu* edit_menu = new wxMenu(); menu_bar->Append(edit_menu, "&Edit");
			edit_menu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
			edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Shift+Z");
			edit_menu->AppendSeparator();
			edit_menu->Append(wxID_CUT, "Cu&t\tCtrl+X");
			edit_menu->Append(wxID_COPY, "&Copy\tCtrl+C");
			edit_menu->Append(wxID_PASTE, "&Paste\tCtrl+V");
			edit_menu->Append(wxID_DELETE, "&Delete\tDel");
			edit_menu->AppendSeparator();
			edit_menu->Append(SEQER_ID_SELECT_CURRENT, "Se&lect Current\tShift+Right");
			edit_menu->Append(wxID_SELECTALL, "Select &All\tCtrl+A");
			edit_menu->Append(SEQER_ID_SELECT_NONE, "Select &None\tCtrl+Shift+A");
			edit_menu->AppendSeparator();
			edit_menu->Append(SEQER_ID_ENTER_VALUE, "Enter &Value\tEnter");
			edit_menu->Append(SEQER_ID_SMALL_INCREASE, "Small &Increase\t]");
			edit_menu->Append(SEQER_ID_SMALL_DECREASE, "Small D&ecrease\t[");
			edit_menu->Append(SEQER_ID_LARGE_INCREASE, "Large &Increase\tShift+]");
			edit_menu->Append(SEQER_ID_LARGE_DECREASE, "Large D&ecrease\tShift+[");
			edit_menu->Append(SEQER_ID_QUANTIZE, "&Quantize\t=");
			edit_menu->AppendSeparator();
			wxMenu* edit_column_menu = new wxMenu(); edit_menu->AppendSubMenu(edit_column_menu, "&Columns");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_1, "Column &1\tCtrl+1");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_2, "Column &2\tCtrl+2");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_3, "Column &3\tCtrl+3");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_4, "Column &4\tCtrl+4");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_5, "Column &5\tCtrl+5");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_6, "Column &6\tCtrl+6");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_7, "Column &7\tCtrl+7");
		wxMenu* view_menu = new wxMenu(); menu_bar->Append(view_menu, "&View");
			view_menu->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl++"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnZoomIn, this, wxID_ZOOM_IN);
			view_menu->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl+-"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnZoomOut, this, wxID_ZOOM_OUT);
			view_menu->Append(SEQER_ID_STEP_SIZE, "&Step Size...\tCtrl+Shift+S"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnStepSize, this, SEQER_ID_STEP_SIZE);
			view_menu->AppendSeparator();
			view_menu->Append(SEQER_ID_FILTER, "&Filter...\tCtrl+Shift+F"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnFilter, this, SEQER_ID_FILTER);
		wxMenu* insert_menu = new wxMenu(); menu_bar->Append(insert_menu, "&Insert");
			wxMenu* insert_note_menu = new wxMenu(); insert_menu->AppendSubMenu(insert_note_menu, "&Notes");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_A, "Note &A\tA");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_B, "Note &B\tB");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_C, "Note &C\tC");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_D, "Note &D\tD");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_E, "Note &E\tE");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_F, "Note &F\tF");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_G, "Note &G\tG");
			insert_menu->AppendSeparator();
			insert_menu->Append(SEQER_ID_INSERT_CONTROL_CHANGE, "&Control Change\tShift+C");
			insert_menu->Append(SEQER_ID_INSERT_PROGRAM_CHANGE, "&Program Change\tShift+P");
			insert_menu->Append(SEQER_ID_INSERT_AFTERTOUCH, "&Aftertouch\tShift+A");
			insert_menu->Append(SEQER_ID_INSERT_PITCH_BEND, "Pitch &Bend\tShift+B");
			insert_menu->Append(SEQER_ID_INSERT_SYSTEM_EXCLUSIVE, "&System Exclusive\tShift+S");
			insert_menu->AppendSeparator();
			insert_menu->Append(SEQER_ID_INSERT_TEXT, "Te&xt\tShift+X");
			insert_menu->Append(SEQER_ID_INSERT_LYRIC, "&Lyric\tShift+L");
			insert_menu->Append(SEQER_ID_INSERT_MARKER, "&Marker\tShift+M");
			insert_menu->Append(SEQER_ID_INSERT_PORT, "P&ort\tShift+O");
			insert_menu->Append(SEQER_ID_INSERT_TEMPO, "&Tempo\tShift+T");
			insert_menu->Append(SEQER_ID_INSERT_TIME_SIGNATURE, "T&ime Signature\tShift+I");
			insert_menu->Append(SEQER_ID_INSERT_KEY_SIGNATURE, "&Key Signature\tShift+K");
		wxMenu* transport_menu = new wxMenu(); menu_bar->Append(transport_menu, "Trans&port");
			transport_menu->Append(SEQER_ID_PLAY, "&Play\tSpace");
			transport_menu->Append(SEQER_ID_RECORD, "&Record\tCtrl+R");
			transport_menu->Append(SEQER_ID_STOP, "&Stop\tShift+Space");
			transport_menu->Append(SEQER_ID_STEP_RECORD, "S&tep Record\tCtrl+T", "", wxITEM_CHECK);
			transport_menu->AppendSeparator();
			transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tCtrl+Right");
			transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tCtrl+Left");
			transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go to &Marker...\tCtrl+Shift+M");
			transport_menu->AppendSeparator();
			transport_menu->Append(SEQER_ID_PORTS, "P&orts...\tCtrl+Shift+O");
		wxMenu* tools_menu = new wxMenu(); menu_bar->Append(tools_menu, "&Tools");
			tools_menu->Append(SEQER_ID_EXTERNAL_UTILITY, "External &Utility...\tCtrl+Shift+U");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_RECORD_MACRO, "&Record Macro...\tCtrl+M");
			tools_menu->Append(SEQER_ID_MACROS, "&Macros...");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_SETTINGS, "&Settings..."); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnSettings, this, SEQER_ID_SETTINGS);
		wxMenu* help_menu = new wxMenu(); menu_bar->Append(help_menu, "&Help");
			help_menu->Append(wxID_HELP_CONTENTS, "&User Manual");
			help_menu->Append(wxID_ABOUT, "&About"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnAbout, this, wxID_ABOUT);

	this->Bind(wxEVT_CHAR_HOOK, &Window::OnKeyPress, this);

	this->sequence_editor = new SequenceEditor(this);
	this->CreateStatusBar();
}

Window::~Window()
{
	delete this->sequence_editor;
}

void Window::OnMenuHighlight(wxMenuEvent& WXUNUSED(event))
{
	// This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
}

void Window::OnFileOpen(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog* file_dialog = new wxFileDialog(this, "Open File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (file_dialog->ShowModal() == wxID_OK)
	{
		if (!this->sequence_editor->Load(file_dialog->GetPath()))
		{
			wxMessageBox("Cannot open the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
		}
	}

	delete file_dialog;
}

void Window::OnClose(wxCommandEvent& WXUNUSED(event))
{
	this->Close(true);
}

void Window::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
	this->sequence_editor->SetStepSize(this->sequence_editor->step_size->ZoomIn());
	this->sequence_editor->Prepare();
}

void Window::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
	this->sequence_editor->SetStepSize(this->sequence_editor->step_size->ZoomOut());
	this->sequence_editor->Prepare();
}

void Window::OnStepSize(wxCommandEvent& WXUNUSED(event))
{
	StepSizeDialog* dialog = new StepSizeDialog(this);

	if (dialog->ShowModal() == wxID_OK)
	{
		if (dialog->steps_per_measure_radio_button->GetValue())
		{
			int amount = atoi(dialog->amount_text_box->GetValue());

			if (amount > 0)
			{
				MidiFileEvent_t time_signature_event = this->sequence_editor->GetLatestTimeSignatureEventForRowNumber(this->sequence_editor->current_row_number);
				this->sequence_editor->SetStepSize(new StepsPerMeasureSize(this->sequence_editor, amount, MidiFileTimeSignatureEvent_getNumerator(time_signature_event), MidiFileTimeSignatureEvent_getDenominator(time_signature_event)));
				this->sequence_editor->Prepare();
			}
		}
		else if (dialog->measures_per_step_radio_button->GetValue())
		{
			int amount = atoi(dialog->amount_text_box->GetValue());

			if (amount > 0)
			{
				this->sequence_editor->SetStepSize(new MeasuresPerStepSize(this->sequence_editor, amount));
				this->sequence_editor->Prepare();
			}
		}
		else if (dialog->seconds_per_step_radio_button->GetValue())
		{
			double amount = atof(dialog->amount_text_box->GetValue());

			if (amount > 0)
			{
				this->sequence_editor->SetStepSize(new SecondsPerStepSize(this->sequence_editor, amount));
				this->sequence_editor->Prepare();
			}
		}
	}

	dialog->Destroy();
}

void Window::OnFilter(wxCommandEvent& WXUNUSED(event))
{
	FilterDialog* dialog = new FilterDialog(this);

	if (dialog->ShowModal() == wxID_OK)
	{
		wxArrayInt selections;

		this->sequence_editor->filtered_event_types.clear();
		dialog->event_type_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) this->sequence_editor->filtered_event_types.push_back(selections[i]);

		this->sequence_editor->filtered_tracks.clear();
		dialog->track_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) this->sequence_editor->filtered_tracks.push_back(selections[i]);

		this->sequence_editor->filtered_channels.clear();
		dialog->channel_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) this->sequence_editor->filtered_channels.push_back(selections[i]);

		this->sequence_editor->Prepare();
	}

	dialog->Destroy();
}

void Window::OnSettings(wxCommandEvent& WXUNUSED(event))
{
	SettingsDialog* dialog = new SettingsDialog(this);

	if (dialog->ShowModal() == wxID_OK)
	{
		this->sequence_editor->event_list->font = dialog->event_list_font_picker->GetSelectedFont();

		long first_note = GetNoteNumberFromName(dialog->piano_roll_first_note_text_box->GetValue());
		long last_note = GetNoteNumberFromName(dialog->piano_roll_last_note_text_box->GetValue());

		if ((first_note >= 0) && (first_note < 128) && (last_note >= 0) && (last_note < 128) && (first_note <= last_note))
		{
			this->sequence_editor->piano_roll->first_note = first_note;
			this->sequence_editor->piano_roll->last_note = last_note;
		}

		long key_width = -1;
		dialog->piano_roll_key_width_text_box->GetValue().ToCLong(&key_width);
		if (key_width >= 0) this->sequence_editor->piano_roll->key_width = key_width;

		this->sequence_editor->Prepare();
		this->sequence_editor->Refresh();
	}

	dialog->Destroy();
}

void Window::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("Seqer\na MIDI sequencer\nby Div Slomin", "About", wxOK);
}

void Window::OnKeyPress(wxKeyEvent& event)
{
	if ((event.GetKeyCode() == '=') && event.ControlDown())
	{
		// make ctrl+plus ignore whether shift is pressed
		wxPostEvent(this, wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_ZOOM_IN));
	}
	else if ((event.GetKeyCode() == '-') && event.ControlDown())
	{
		// make ctrl+minus ignore whether shift is pressed
		wxPostEvent(this, wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_ZOOM_OUT));
	}
	else if ((event.GetKeyCode() == WXK_LEFT) && event.ShiftDown())
	{
		// make shift+left be a synonym for shift+right
		wxPostEvent(this, wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, SEQER_ID_SELECT_CURRENT));
	}
	else
	{
		event.Skip();
	}
}

StepSizeDialog::StepSizeDialog(Window* window): wxDialog(NULL, wxID_ANY, "Step Size")
{
	this->window = window;

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outer_sizer);

	wxBoxSizer* amount_sizer = new wxBoxSizer(wxHORIZONTAL);
	outer_sizer->Add(amount_sizer, wxSizerFlags().Expand().Border(wxTOP | wxRIGHT | wxLEFT));
	amount_sizer->Add(new wxStaticText(this, wxID_ANY, "&Amount"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border());
	this->amount_text_box = new wxTextCtrl(this, wxID_ANY);
	amount_sizer->Add(this->amount_text_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP | wxRIGHT | wxBOTTOM));

	wxBoxSizer* mode_sizer = new wxBoxSizer(wxVERTICAL);
	outer_sizer->Add(mode_sizer, wxSizerFlags().Align(wxALIGN_CENTER).Border());
	this->steps_per_measure_radio_button = new wxRadioButton(this, wxID_ANY, "Ste&ps per measure", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	mode_sizer->Add(this->steps_per_measure_radio_button, wxSizerFlags().Align(wxALIGN_LEFT).Border());
	this->measures_per_step_radio_button = new wxRadioButton(this, wxID_ANY, "&Measures per step");
	mode_sizer->Add(this->measures_per_step_radio_button, wxSizerFlags().Align(wxALIGN_LEFT).Border(wxRIGHT | wxBOTTOM | wxLEFT));
	this->seconds_per_step_radio_button = new wxRadioButton(this, wxID_ANY, "&Seconds per step");
	mode_sizer->Add(this->seconds_per_step_radio_button, wxSizerFlags().Align(wxALIGN_LEFT).Border(wxRIGHT | wxBOTTOM | wxLEFT));

	wxSizer* button_sizer = this->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags().Align(wxALIGN_CENTER).Border());

	outer_sizer->Fit(this);

	this->window->sequence_editor->step_size->PopulateDialog(this);
}

FilterDialog::FilterDialog(Window* window): wxDialog(NULL, wxID_ANY, "Filter", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	this->window = window;

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outer_sizer);

	wxBoxSizer* controls_sizer = new wxBoxSizer(wxHORIZONTAL);
	outer_sizer->Add(controls_sizer, wxSizerFlags().Proportion(1).Expand().Border());

	wxBoxSizer* event_type_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(event_type_sizer, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* event_type_label = new wxStaticText(this, wxID_ANY, "&Event type");
	event_type_sizer->Add(event_type_label, wxSizerFlags().Align(wxALIGN_CENTER));
	event_type_label->Bind(wxEVT_LEFT_DOWN, &FilterDialog::OnEventTypeLabelClick, this);
	event_type_label->Bind(wxEVT_LEFT_DCLICK, &FilterDialog::OnEventTypeLabelClick, this);

	wxArrayString event_type_names;
	for (int i = 0; event_types[i] != NULL; i++) event_type_names.Add(event_types[i]->GetName());
	this->event_type_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, event_type_names, wxLB_MULTIPLE);
	for (int i = 0; i < this->window->sequence_editor->filtered_event_types.size(); i++) this->event_type_list_box->SetSelection(this->window->sequence_editor->filtered_event_types[i]);
	event_type_sizer->Add(this->event_type_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP));

	wxBoxSizer* track_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(track_sizer, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* track_label = new wxStaticText(this, wxID_ANY, "&Track");
	track_sizer->Add(track_label, wxSizerFlags().Align(wxALIGN_CENTER).Border(wxLEFT | wxRIGHT));
	track_label->Bind(wxEVT_LEFT_DOWN, &FilterDialog::OnTrackLabelClick, this);
	track_label->Bind(wxEVT_LEFT_DCLICK, &FilterDialog::OnTrackLabelClick, this);

	wxArrayString tracks;
	for (int i = 0; i < MidiFile_getNumberOfTracks(this->window->sequence_editor->sequence->midi_file); i++) tracks.Add(wxString::Format("%d", i));
	this->track_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, tracks, wxLB_MULTIPLE);
	for (int i = 0; i < this->window->sequence_editor->filtered_tracks.size(); i++) this->track_list_box->SetSelection(this->window->sequence_editor->filtered_tracks[i]);
	track_sizer->Add(this->track_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP | wxLEFT | wxRIGHT));

	wxBoxSizer* channel_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(channel_sizer, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* channel_label = new wxStaticText(this, wxID_ANY, "&Channel");
	channel_sizer->Add(channel_label, wxSizerFlags().Align(wxALIGN_CENTER));
	channel_label->Bind(wxEVT_LEFT_DOWN, &FilterDialog::OnChannelLabelClick, this);
	channel_label->Bind(wxEVT_LEFT_DCLICK, &FilterDialog::OnChannelLabelClick, this);

	wxArrayString channels;
	for (int i = 1; i <= 16; i++) channels.Add(wxString::Format("%d", i));
	this->channel_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, channels, wxLB_MULTIPLE);
	for (int i = 0; i < this->window->sequence_editor->filtered_channels.size(); i++) this->channel_list_box->SetSelection(this->window->sequence_editor->filtered_channels[i]);
	channel_sizer->Add(this->channel_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP));

	wxSizer* button_sizer = this->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags().Align(wxALIGN_CENTER).Border());

	outer_sizer->Fit(this);
}

void FilterDialog::OnEventTypeLabelClick(wxMouseEvent& event)
{
	int count = this->event_type_list_box->GetCount();
	wxArrayInt selections;

	if (this->event_type_list_box->GetSelections(selections) == count)
	{
		for (int i = 0; i < count; i++)
		{
			this->event_type_list_box->Deselect(i);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			this->event_type_list_box->SetSelection(i);
		}
	}

	this->event_type_list_box->SetFocus();
	event.Skip();
}

void FilterDialog::OnTrackLabelClick(wxMouseEvent& event)
{
	int count = this->track_list_box->GetCount();
	wxArrayInt selections;

	if (this->track_list_box->GetSelections(selections) == count)
	{
		for (int i = 0; i < count; i++)
		{
			this->track_list_box->Deselect(i);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			this->track_list_box->SetSelection(i);
		}
	}

	this->track_list_box->SetFocus();
	event.Skip();
}

void FilterDialog::OnChannelLabelClick(wxMouseEvent& event)
{
	int count = this->channel_list_box->GetCount();
	wxArrayInt selections;

	if (this->channel_list_box->GetSelections(selections) == count)
	{
		for (int i = 0; i < count; i++)
		{
			this->channel_list_box->Deselect(i);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			this->channel_list_box->SetSelection(i);
		}
	}

	this->channel_list_box->SetFocus();
	event.Skip();
}

SettingsDialog::SettingsDialog(Window* window): wxDialog(NULL, wxID_ANY, "Settings")
{
	this->window = window;

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outer_sizer);

	wxStaticBoxSizer* event_list_settings_section = new wxStaticBoxSizer(wxVERTICAL, this, "Event list");
	outer_sizer->Add(event_list_settings_section, wxSizerFlags().Expand().Border());
	wxFlexGridSizer* event_list_settings_sizer = new wxFlexGridSizer(3, wxSizerFlags::GetDefaultBorder(), wxSizerFlags::GetDefaultBorder());
	event_list_settings_section->Add(event_list_settings_sizer, wxSizerFlags().Expand().Border());
	event_list_settings_sizer->AddGrowableCol(1, 1);

	event_list_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "Fo&nt"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->event_list_font_picker = new wxFontPickerCtrl(this, wxID_ANY, this->window->sequence_editor->event_list->font, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL);
	event_list_settings_sizer->Add(this->event_list_font_picker, wxSizerFlags().Expand());
	wxButton* default_event_list_font_button = new wxButton(this, wxID_ANY, "Default");
	event_list_settings_sizer->Add(default_event_list_font_button, wxSizerFlags().Expand());
	default_event_list_font_button->Bind(wxEVT_BUTTON, &SettingsDialog::OnDefaultEventListFontButtonClick, this);

	wxStaticBoxSizer* piano_roll_settings_section = new wxStaticBoxSizer(wxVERTICAL, this, "Piano roll");
	outer_sizer->Add(piano_roll_settings_section, wxSizerFlags().Expand().Border(wxRIGHT | wxBOTTOM | wxLEFT));
	wxFlexGridSizer* piano_roll_settings_sizer = new wxFlexGridSizer(3, wxSizerFlags::GetDefaultBorder(), wxSizerFlags::GetDefaultBorder());
	piano_roll_settings_section->Add(piano_roll_settings_sizer, wxSizerFlags().Expand().Border());
	piano_roll_settings_sizer->AddGrowableCol(1, 1);

	piano_roll_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "&First note"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->piano_roll_first_note_text_box = new wxTextCtrl(this, wxID_ANY, GetNoteNameFromNumber(this->window->sequence_editor->piano_roll->first_note));
	piano_roll_settings_sizer->Add(this->piano_roll_first_note_text_box, wxSizerFlags().Expand());
	wxButton* default_piano_roll_first_note_button = new wxButton(this, wxID_ANY, "Default");
	piano_roll_settings_sizer->Add(default_piano_roll_first_note_button, wxSizerFlags().Expand());
	default_piano_roll_first_note_button->Bind(wxEVT_BUTTON, &SettingsDialog::OnDefaultPianoRollFirstNoteButtonClick, this);

	piano_roll_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "&Last note"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->piano_roll_last_note_text_box = new wxTextCtrl(this, wxID_ANY, GetNoteNameFromNumber(this->window->sequence_editor->piano_roll->last_note));
	piano_roll_settings_sizer->Add(this->piano_roll_last_note_text_box, wxSizerFlags().Expand());
	wxButton* default_piano_roll_last_note_button = new wxButton(this, wxID_ANY, "Default");
	piano_roll_settings_sizer->Add(default_piano_roll_last_note_button, wxSizerFlags().Expand());
	default_piano_roll_last_note_button->Bind(wxEVT_BUTTON, &SettingsDialog::OnDefaultPianoRollLastNoteButtonClick, this);

	piano_roll_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "&Key width"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->piano_roll_key_width_text_box = new wxTextCtrl(this, wxID_ANY, wxString::Format("%ld", this->window->sequence_editor->piano_roll->key_width));
	piano_roll_settings_sizer->Add(this->piano_roll_key_width_text_box, wxSizerFlags().Expand());
	wxButton* default_piano_roll_key_width_button = new wxButton(this, wxID_ANY, "Default");
	piano_roll_settings_sizer->Add(default_piano_roll_key_width_button, wxSizerFlags().Expand());
	default_piano_roll_key_width_button->Bind(wxEVT_BUTTON, &SettingsDialog::OnDefaultPianoRollKeyWidthButtonClick, this);

	wxSizer* button_sizer = this->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags().Align(wxALIGN_CENTER).Border());

	outer_sizer->Fit(this);
}

void SettingsDialog::OnDefaultEventListFontButtonClick(wxCommandEvent& event)
{
	this->event_list_font_picker->SetSelectedFont(this->window->application->default_event_list_font);
	event.Skip();
}

void SettingsDialog::OnDefaultPianoRollFirstNoteButtonClick(wxCommandEvent& event)
{
	this->piano_roll_first_note_text_box->SetValue(GetNoteNameFromNumber(this->window->application->default_piano_roll_first_note));
	event.Skip();
}

void SettingsDialog::OnDefaultPianoRollLastNoteButtonClick(wxCommandEvent& event)
{
	this->piano_roll_last_note_text_box->SetValue(GetNoteNameFromNumber(this->window->application->default_piano_roll_last_note));
	event.Skip();
}

void SettingsDialog::OnDefaultPianoRollKeyWidthButtonClick(wxCommandEvent& event)
{
	this->piano_roll_key_width_text_box->SetValue(wxString::Format("%ld", this->window->application->default_piano_roll_key_width));
	event.Skip();
}

