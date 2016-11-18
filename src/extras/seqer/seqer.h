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

	Window(Application* application);
	~Window();
	void OnMenuHighlight(wxMenuEvent& event);
	void OnFileNew(wxCommandEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnColumn1(wxCommandEvent& event);
	void OnColumn2(wxCommandEvent& event);
	void OnColumn3(wxCommandEvent& event);
	void OnColumn4(wxCommandEvent& event);
	void OnColumn5(wxCommandEvent& event);
	void OnColumn6(wxCommandEvent& event);
	void OnColumn7(wxCommandEvent& event);
	void OnZoomIn(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	void OnStepSize(wxCommandEvent& event);
	void OnFilter(wxCommandEvent& event);
	void OnInsertNoteA(wxCommandEvent& event);
	void OnInsertNoteB(wxCommandEvent& event);
	void OnInsertNoteC(wxCommandEvent& event);
	void OnInsertNoteD(wxCommandEvent& event);
	void OnInsertNoteE(wxCommandEvent& event);
	void OnInsertNoteF(wxCommandEvent& event);
	void OnInsertNoteG(wxCommandEvent& event);
	void OnNextMarker(wxCommandEvent& event);
	void OnPreviousMarker(wxCommandEvent& event);
	void OnPreferences(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnKeyPress(wxKeyEvent& event);
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
	void OnEventTypeLabelClick(wxMouseEvent& event);
	void OnTrackLabelClick(wxMouseEvent& event);
	void OnChannelLabelClick(wxMouseEvent& event);
};

class PreferencesDialog: public wxDialog
{
public:
	Window* window;
	wxFontPickerCtrl* event_list_font_picker;
	wxTextCtrl* piano_roll_first_note_text_box;
	wxTextCtrl* piano_roll_last_note_text_box;
	wxTextCtrl* piano_roll_key_width_text_box;

	static void Run(Window* window);
	PreferencesDialog(Window* window);
	void OnDefaultEventListFontButtonClick(wxCommandEvent& event);
	void OnDefaultPianoRollFirstNoteButtonClick(wxCommandEvent& event);
	void OnDefaultPianoRollLastNoteButtonClick(wxCommandEvent& event);
	void OnDefaultPianoRollKeyWidthButtonClick(wxCommandEvent& event);
};

#endif
