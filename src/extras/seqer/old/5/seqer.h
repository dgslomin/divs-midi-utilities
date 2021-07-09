#ifndef SEQER_INCLUDED
#define SEQER_INCLUDED

class Application;
class Window;
class StepSizeDialog;
class FilterDialog;
class PreferencesDialog;

#include <wx/wx.h>
#include <wx/fontpicker.h>
#include "sequence-editor.h"

class Application: public wxApp
{
public:
	wxFont default_event_list_font;
	long default_piano_roll_first_note;
	long default_piano_roll_last_note;
	long default_piano_roll_key_width;

	bool OnInit();
};

class Window: public wxFrame
{
public:
	Application* application;
	SequenceEditor* sequence_editor;

	Window(Application* application, Window* existing_window = NULL);
	~Window();
	bool SaveChangesIfNeeded();
	bool SaveChanges();
	bool Save();
	bool SaveAs();
};

class StepSizeDialog: public wxDialog
{
public:
	Window* window;
	wxTextCtrl* amount_text_box;
	wxRadioButton* steps_per_measure_radio_button;
	wxRadioButton* measures_per_step_radio_button;
	wxRadioButton* seconds_per_step_radio_button;

	static void Run(Window* window);
	StepSizeDialog(Window* window);
};

class FilterDialog: public wxDialog
{
public:
	Window* window;
	wxListBox* event_type_list_box;
	wxListBox* track_list_box;
	wxListBox* channel_list_box;

	static void Run(Window* window);
	FilterDialog(Window* window);
};

class PreferencesDialog: public wxDialog
{
public:
	Window* window;
	wxTextCtrl* piano_roll_first_note_text_box;
	wxTextCtrl* piano_roll_last_note_text_box;
	wxTextCtrl* piano_roll_key_width_text_box;
	wxFontPickerCtrl* event_list_font_picker;

	static void Run(Window* window);
	PreferencesDialog(Window* window);
};

#endif
