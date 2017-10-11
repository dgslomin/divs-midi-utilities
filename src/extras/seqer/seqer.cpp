
#include <functional>
#include <set>
#include <wx/wx.h>
#include <wx/aboutdlg.h>
#include <wx/fontpicker.h>
#include <midifile.h>
#include "event-type.h"
#include "music-math.h"
#include "seqer.h"
#include "sequence-editor.h"

#ifndef __WXMSW__
#include "seqer.xpm"
#endif

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
	SEQER_ID_OVERWRITE,
	SEQER_ID_PLAY,
	SEQER_ID_STOP,
	SEQER_ID_RECORD,
	SEQER_ID_STEP_RECORD,
	SEQER_ID_NEXT_MARKER,
	SEQER_ID_PREVIOUS_MARKER,
	SEQER_ID_GO_TO_MARKER,
	SEQER_ID_PORTS,
	SEQER_ID_EXTERNAL_UTILITY,
	SEQER_ID_RECORD_MACRO,
	SEQER_ID_MACROS,
	SEQER_ID_HIGHEST
};

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
#ifdef __WXOSX__
	this->default_event_list_font = wxFont(wxFontInfo(10).FaceName("Lucida Grande"));
#else
	this->default_event_list_font = *wxNORMAL_FONT;
#endif
	this->default_piano_roll_first_note = 21; // standard piano range
	this->default_piano_roll_last_note = 108;
	this->default_piano_roll_key_width = 3;

	Window* window = new Window(this);
	window->Show(true);

	if (this->argc > 1)
	{
		if (!window->sequence_editor->Load(this->argv[1]))
		{
			wxMessageBox("Cannot open the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
		}
	}

	return true;
}

Window::Window(Application* application, Window* existing_window): wxFrame((wxFrame*)(NULL), wxID_ANY, "Seqer", wxDefaultPosition, wxSize(640, 480))
{
	this->application = application;
	this->SetIcon(wxICON(seqer));
	this->CreateStatusBar();

	wxMenuBar* menu_bar = new wxMenuBar();
		wxMenu* file_menu = new wxMenu();
			file_menu->Append(wxID_NEW, "&New\tCtrl+N");
			file_menu->Append(SEQER_ID_NEW_WINDOW, "New &Window\tCtrl+Shift+N");
			file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O");
			file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
			file_menu->Append(wxID_SAVEAS, "Save &As...\tCtrl+Shift+S");
			file_menu->AppendSeparator();
			file_menu->Append(wxID_CLOSE, "&Close\tCtrl+W");
		menu_bar->Append(file_menu, "&File");
		wxMenu* edit_menu = new wxMenu();
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
			wxMenu* edit_column_menu = new wxMenu();
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_1, "Column &1\tCtrl+1");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_2, "Column &2\tCtrl+2");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_3, "Column &3\tCtrl+3");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_4, "Column &4\tCtrl+4");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_5, "Column &5\tCtrl+5");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_6, "Column &6\tCtrl+6");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_7, "Column &7\tCtrl+7");
			edit_menu->AppendSubMenu(edit_column_menu, "&Columns");
		menu_bar->Append(edit_menu, "&Edit");
		wxMenu* view_menu = new wxMenu();
			view_menu->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl++");
			view_menu->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl+-");
			view_menu->Append(SEQER_ID_STEP_SIZE, "Ste&p Size...\tCtrl+Shift+P");
			view_menu->AppendSeparator();
			view_menu->Append(SEQER_ID_FILTER, "&Filter...\tCtrl+Shift+F");
		menu_bar->Append(view_menu, "&View");
		wxMenu* insert_menu = new wxMenu();
			wxMenu* insert_note_menu = new wxMenu();
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_A, "Note &A\tA");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_B, "Note &B\tB");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_C, "Note &C\tC");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_D, "Note &D\tD");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_E, "Note &E\tE");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_F, "Note &F\tF");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_G, "Note &G\tG");
			insert_menu->AppendSubMenu(insert_note_menu, "&Notes");
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
			insert_menu->AppendSeparator();
#ifdef __WXOSX__
			insert_menu->Append(SEQER_ID_OVERWRITE, "&Overwrite\tKP_Enter", "", wxITEM_CHECK);
#else
			insert_menu->Append(SEQER_ID_OVERWRITE, "&Overwrite\tInsert", "", wxITEM_CHECK);
#endif
		menu_bar->Append(insert_menu, "&Insert");
		wxMenu* transport_menu = new wxMenu();
			transport_menu->Append(SEQER_ID_PLAY, "&Play\tSpace");
			transport_menu->Append(SEQER_ID_STOP, "&Stop\tShift+Space");
			transport_menu->Append(SEQER_ID_RECORD, "&Record\tCtrl+R");
			transport_menu->Append(SEQER_ID_STEP_RECORD, "S&tep Record\tCtrl+T", "", wxITEM_CHECK);
			transport_menu->AppendSeparator();
#ifdef __WXOSX__
			transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tAlt+Right");
			transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tAlt+Left");
#else
			transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tCtrl+Right");
			transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tCtrl+Left");
#endif
			transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go to &Marker...\tCtrl+Shift+M");
			transport_menu->AppendSeparator();
			transport_menu->Append(SEQER_ID_PORTS, "P&orts...\tCtrl+Shift+O");
		menu_bar->Append(transport_menu, "Trans&port");
		wxMenu* tools_menu = new wxMenu();
			tools_menu->Append(SEQER_ID_EXTERNAL_UTILITY, "External &Utility...\tCtrl+Shift+U");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_RECORD_MACRO, "&Record Macro...\tCtrl+Shift+R");
			tools_menu->Append(SEQER_ID_MACROS, "&Macros...");
			tools_menu->AppendSeparator();
#ifdef __WXOSX__
			tools_menu->Append(wxID_PREFERENCES, "&Preferences...\tCtrl+,");
#else
			tools_menu->Append(wxID_PREFERENCES, "&Preferences...");
#endif
		menu_bar->Append(tools_menu, "&Tools");
		wxMenu* help_menu = new wxMenu();
			help_menu->Append(wxID_HELP_CONTENTS, "&User Manual");
			help_menu->Append(wxID_ABOUT, "&About Seqer");
		menu_bar->Append(help_menu, "&Help");
	this->SetMenuBar(menu_bar);

	// This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
	this->Bind(wxEVT_MENU_HIGHLIGHT, [=](wxMenuEvent&) {});

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		if (!this->SaveChangesIfNeeded()) return;
		this->sequence_editor->New();
	}, wxID_NEW);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		(new Window(this->application, this))->Show(true);
	}, SEQER_ID_NEW_WINDOW);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		if (!this->SaveChangesIfNeeded()) return;
		wxFileDialog* file_dialog = new wxFileDialog(this, "Open File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

		if (file_dialog->ShowModal() == wxID_OK)
		{
			if (!this->sequence_editor->Load(file_dialog->GetPath()))
			{
				wxMessageBox("Cannot open the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
			}
		}

		delete file_dialog;
	}, wxID_OPEN);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->Save();
	}, wxID_SAVE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->SaveAs();
	}, wxID_SAVEAS);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->Close();
	}, wxID_CLOSE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& event) {
		std::set<Sequence*> sequences;
		bool is_modified = false;

		for (wxWindowList::compatibility_iterator window_iterator = wxTopLevelWindows.GetFirst(); window_iterator != NULL; window_iterator = window_iterator->GetNext())
		{
			Window* window = (Window*)(window_iterator->GetData());
			sequences.insert(window->sequence_editor->sequence);
			if (window->sequence_editor->IsModified()) is_modified = true;
		}

		if (sequences.size() == 1)
		{
			if (!is_modified || this->SaveChanges())
			{
				event.Skip();
			}
		}
		else
		{
			if (!is_modified || (wxMessageBox("Really quit without saving changes?", "Quit", wxOK | wxCANCEL) == wxOK))
			{
				event.Skip();
			}
		}
	}, wxID_EXIT);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->sequence->undo_command_processor->Undo();
	}, wxID_UNDO);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->sequence->undo_command_processor->Redo();
	}, wxID_REDO);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->DeleteRow();
	}, wxID_DELETE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->SelectCurrent();
	}, SEQER_ID_SELECT_CURRENT);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->SelectAll();
	}, wxID_SELECTALL);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->SelectNone();
	}, SEQER_ID_SELECT_NONE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->EnterValue();
	}, SEQER_ID_ENTER_VALUE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->SmallIncrease();
	}, SEQER_ID_SMALL_INCREASE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->SmallDecrease();
	}, SEQER_ID_SMALL_DECREASE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->LargeIncrease();
	}, SEQER_ID_LARGE_INCREASE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->LargeDecrease();
	}, SEQER_ID_LARGE_DECREASE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->Quantize();
	}, SEQER_ID_QUANTIZE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToColumn(1);
	}, SEQER_ID_EDIT_COLUMN_1);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToColumn(2);
	}, SEQER_ID_EDIT_COLUMN_2);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToColumn(3);
	}, SEQER_ID_EDIT_COLUMN_3);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToColumn(4);
	}, SEQER_ID_EDIT_COLUMN_4);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToColumn(5);
	}, SEQER_ID_EDIT_COLUMN_5);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToColumn(6);
	}, SEQER_ID_EDIT_COLUMN_6);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToColumn(7);
	}, SEQER_ID_EDIT_COLUMN_7);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->ZoomIn();
	}, wxID_ZOOM_IN);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->ZoomOut();
	}, wxID_ZOOM_OUT);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		StepSizeDialog::Run(this);
	}, SEQER_ID_STEP_SIZE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		FilterDialog::Run(this);
	}, SEQER_ID_FILTER);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->InsertNote(5);
	}, SEQER_ID_INSERT_NOTE_A);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->InsertNote(6);
	}, SEQER_ID_INSERT_NOTE_B);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->InsertNote(0);
	}, SEQER_ID_INSERT_NOTE_C);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->InsertNote(1);
	}, SEQER_ID_INSERT_NOTE_D);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->InsertNote(2);
	}, SEQER_ID_INSERT_NOTE_E);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->InsertNote(3);
	}, SEQER_ID_INSERT_NOTE_F);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->InsertNote(4);
	}, SEQER_ID_INSERT_NOTE_G);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->InsertMarker();
	}, SEQER_ID_INSERT_MARKER);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& event) {
		this->sequence_editor->SetOverwriteMode(event.IsChecked());
	}, SEQER_ID_OVERWRITE);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToNextMarker();
	}, SEQER_ID_NEXT_MARKER);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		this->sequence_editor->GoToPreviousMarker();
	}, SEQER_ID_PREVIOUS_MARKER);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		wxTextEntryDialog* dialog = new wxTextEntryDialog(this, "Name", "Go to Marker");

		if (dialog->ShowModal() == wxID_OK)
		{
			this->sequence_editor->GoToMarker(dialog->GetValue());
		}

		dialog->Destroy();
	}, SEQER_ID_GO_TO_MARKER);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		PreferencesDialog::Run(this);
	}, wxID_PREFERENCES);

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& WXUNUSED(event)) {
		wxMessageBox("Seqer\na MIDI sequencer by Div Slomin\nprovided under terms of the BSD license", "About", wxOK);
	}, wxID_ABOUT);

	this->Bind(wxEVT_CHAR_HOOK, [=](wxKeyEvent& event) {
		int keycode = event.GetKeyCode();
		int modifiers = event.GetModifiers();

		if ((keycode == WXK_UP) && (modifiers == wxMOD_NONE))
		{
			this->sequence_editor->RowUp();
		}
		else if ((keycode == WXK_DOWN) && (modifiers == wxMOD_NONE))
		{
			this->sequence_editor->RowDown();
		}
		else if ((keycode == WXK_PAGEUP) && (modifiers == wxMOD_NONE))
		{
			this->sequence_editor->PageUp();
		}
		else if ((keycode == WXK_PAGEDOWN) && (modifiers == wxMOD_NONE))
		{
			this->sequence_editor->PageDown();
		}
		else if ((keycode == WXK_HOME) && (modifiers == wxMOD_CONTROL))
		{
			this->sequence_editor->GoToFirstRow();
		}
		else if ((keycode == WXK_END) && (modifiers == wxMOD_CONTROL))
		{
			this->sequence_editor->GoToLastRow();
		}
		else if ((keycode == WXK_LEFT) && (modifiers == wxMOD_NONE))
		{
			this->sequence_editor->ColumnLeft();
		}
		else if ((keycode == WXK_RIGHT) && (modifiers == wxMOD_NONE))
		{
			this->sequence_editor->ColumnRight();
		}
		else if ((keycode == WXK_HOME) && (modifiers == wxMOD_NONE))
		{
			this->sequence_editor->GoToColumn(1);
		}
		else if ((keycode == WXK_END) && (modifiers == wxMOD_NONE))
		{
			this->sequence_editor->GoToColumn(EVENT_LIST_LAST_COLUMN_NUMBER);
		}
#ifdef __WXOSX__
		else if ((keycode == WXK_UP) && (modifiers == wxMOD_ALT))
		{
			this->sequence_editor->PageUp();
		}
		else if ((keycode == WXK_DOWN) && (modifiers == wxMOD_ALT))
		{
			this->sequence_editor->PageDown();
		}
		else if ((keycode == WXK_UP) && (modifiers == wxMOD_CONTROL))
		{
			this->sequence_editor->GoToFirstRow();
		}
		else if ((keycode == WXK_DOWN) && (modifiers == wxMOD_CONTROL))
		{
			this->sequence_editor->GoToLastRow();
		}
		else if ((keycode == WXK_LEFT) && (modifiers == wxMOD_CONTROL))
		{
			this->sequence_editor->GoToColumn(1);
		}
		else if ((keycode == WXK_RIGHT) && (modifiers == wxMOD_CONTROL))
		{
			this->sequence_editor->GoToColumn(EVENT_LIST_LAST_COLUMN_NUMBER);
		}
		else if ((keycode == 'A') && (modifiers == wxMOD_RAW_CONTROL))
		{
			this->sequence_editor->GoToColumn(1);
		}
		else if ((keycode == 'E') && (modifiers == wxMOD_RAW_CONTROL))
		{
			this->sequence_editor->GoToColumn(EVENT_LIST_LAST_COLUMN_NUMBER);
		}
		else if ((keycode == 'P') && (modifiers == wxMOD_RAW_CONTROL))
		{
			this->sequence_editor->RowUp();
		}
		else if ((keycode == 'N') && (modifiers == wxMOD_RAW_CONTROL))
		{
			this->sequence_editor->RowDown();
		}
		else if ((keycode == 'B') && (modifiers == wxMOD_RAW_CONTROL))
		{
			this->sequence_editor->ColumnLeft();
		}
		else if ((keycode == 'F') && (modifiers == wxMOD_RAW_CONTROL))
		{
			this->sequence_editor->ColumnRight();
		}
		else if ((keycode == 'D') && (modifiers == wxMOD_RAW_CONTROL))
		{
			this->sequence_editor->DeleteRow();
		}
		else if ((keycode == 'H') && (modifiers == wxMOD_RAW_CONTROL))
		{
			this->sequence_editor->DeleteRow();
		}
#endif
		else if ((keycode == '=') && ((modifiers & ~wxMOD_SHIFT) == wxMOD_CONTROL))
		{
			// make ctrl+plus ignore whether shift is pressed
			this->ProcessCommand(wxID_ZOOM_IN);
		}
		else if ((keycode == '-') && ((modifiers & ~wxMOD_SHIFT) == wxMOD_CONTROL))
		{
			// make ctrl+minus ignore whether shift is pressed
			this->ProcessCommand(wxID_ZOOM_OUT);
		}
		else if ((keycode == WXK_LEFT) && (modifiers == wxMOD_SHIFT))
		{
			// make shift+left be a synonym for shift+right
			this->ProcessCommand(SEQER_ID_SELECT_CURRENT);
		}
		else if ((keycode == WXK_BACK) && (modifiers == wxMOD_NONE))
		{
			// make backspace be a synonym for delete
			this->ProcessCommand(wxID_DELETE);
		}
		else if ((keycode == ']') && (modifiers == wxMOD_NONE))
		{
			// although these are listed in the menu, they don't work automatically on WXGTK
			this->ProcessCommand(SEQER_ID_SMALL_INCREASE);
		}
		else if ((keycode == '[') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_SMALL_DECREASE);
		}
		else if ((keycode == ']') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_LARGE_INCREASE);
		}
		else if ((keycode == '[') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_LARGE_DECREASE);
		}
		else if ((keycode == '=') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_QUANTIZE);
		}
		else if ((keycode == 'A') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_INSERT_NOTE_A);
		}
		else if ((keycode == 'B') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_INSERT_NOTE_B);
		}
		else if ((keycode == 'C') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_INSERT_NOTE_C);
		}
		else if ((keycode == 'D') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_INSERT_NOTE_D);
		}
		else if ((keycode == 'E') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_INSERT_NOTE_E);
		}
		else if ((keycode == 'F') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_INSERT_NOTE_F);
		}
		else if ((keycode == 'G') && (modifiers == wxMOD_NONE))
		{
			this->ProcessCommand(SEQER_ID_INSERT_NOTE_G);
		}
		else if ((keycode == 'C') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_CONTROL_CHANGE);
		}
		else if ((keycode == 'P') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_PROGRAM_CHANGE);
		}
		else if ((keycode == 'A') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_AFTERTOUCH);
		}
		else if ((keycode == 'B') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_PITCH_BEND);
		}
		else if ((keycode == 'S') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_SYSTEM_EXCLUSIVE);
		}
		else if ((keycode == 'X') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_TEXT);
		}
		else if ((keycode == 'L') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_LYRIC);
		}
		else if ((keycode == 'M') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_MARKER);
		}
		else if ((keycode == 'O') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_PORT);
		}
		else if ((keycode == 'T') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_TEMPO);
		}
		else if ((keycode == 'I') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_TIME_SIGNATURE);
		}
		else if ((keycode == 'K') && (modifiers == wxMOD_SHIFT))
		{
			this->ProcessCommand(SEQER_ID_INSERT_KEY_SIGNATURE);
		}
		else
		{
			event.Skip();
		}
	});

	this->Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
		if (!this->SaveChangesIfNeeded()) return;
		event.Skip();
	});

	this->sequence_editor = new SequenceEditor(this, (existing_window == NULL) ? NULL : existing_window->sequence_editor);
}

bool Window::SaveChangesIfNeeded()
{
	if (!(this->sequence_editor->IsModified() && this->sequence_editor->IsLastEditorForSequence())) return true;
	return this->SaveChanges();
}

bool Window::SaveChanges()
{
	switch (wxMessageBox("Do you want to save changes to the current file?", "Save Changes", wxYES_NO | wxCANCEL))
	{
		case wxYES:
		{
			return this->Save();
		}
		case wxCANCEL:
		{
			return false;
		}
		default:
		{
			return true;
		}
	}
}

bool Window::Save()
{
	if (this->sequence_editor->GetFilename() == wxEmptyString)
	{
		return this->SaveAs();
	}
	else
	{
		if (this->sequence_editor->Save())
		{
			return true;
		}
		else
		{
			wxMessageBox("Cannot save the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
			return false;
		}
	}
}

bool Window::SaveAs()
{
	wxFileDialog* file_dialog = new wxFileDialog(this, "Save File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_SAVE);
	bool result;

	if (file_dialog->ShowModal() == wxID_OK)
	{
		if (this->sequence_editor->SaveAs(file_dialog->GetPath()))
		{
			result = true;
		}
		else
		{
			wxMessageBox("Cannot save the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
			result = false;
		}
	}
	else
	{
		return false;
	}

	delete file_dialog;
	return result;
}

Window::~Window()
{
	delete this->sequence_editor;
}

void StepSizeDialog::Run(Window* window)
{
	StepSizeDialog* dialog = new StepSizeDialog(window);

	if (dialog->ShowModal() == wxID_OK)
	{
		if (dialog->steps_per_measure_radio_button->GetValue())
		{
			int amount = atoi(dialog->amount_text_box->GetValue());

			if (amount > 0)
			{
				MidiFileEvent_t time_signature_event = window->sequence_editor->GetLatestTimeSignatureEventForRowNumber(window->sequence_editor->current_row_number);
				window->sequence_editor->SetStepSize(new StepsPerMeasureSize(window->sequence_editor, amount, MidiFileTimeSignatureEvent_getNumerator(time_signature_event), MidiFileTimeSignatureEvent_getDenominator(time_signature_event)));
			}
		}
		else if (dialog->measures_per_step_radio_button->GetValue())
		{
			int amount = atoi(dialog->amount_text_box->GetValue());

			if (amount > 0)
			{
				window->sequence_editor->SetStepSize(new MeasuresPerStepSize(window->sequence_editor, amount));
			}
		}
		else if (dialog->seconds_per_step_radio_button->GetValue())
		{
			double amount = atof(dialog->amount_text_box->GetValue());

			if (amount > 0)
			{
				window->sequence_editor->SetStepSize(new SecondsPerStepSize(window->sequence_editor, amount));
			}
		}
	}

	dialog->Destroy();
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

void FilterDialog::Run(Window* window)
{
	FilterDialog* dialog = new FilterDialog(window);

	if (dialog->ShowModal() == wxID_OK)
	{
		wxArrayInt selections;

		std::set<EventType*> filtered_event_types;
		dialog->event_type_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) filtered_event_types.insert((EventType*)(dialog->event_type_list_box->GetClientData(selections[i])));

		std::set<MidiFileTrack_t> filtered_tracks;
		dialog->track_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) filtered_tracks.insert((MidiFileTrack_t)(dialog->track_list_box->GetClientData(selections[i])));

		std::set<int> filtered_channels;
		dialog->channel_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) filtered_channels.insert(selections[i]);

		window->sequence_editor->SetFilters(filtered_event_types, filtered_tracks, filtered_channels);
	}

	dialog->Destroy();
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

	std::function<void (wxMouseEvent&)> event_type_label_click_callback = [=](wxMouseEvent& event) {
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
	};

	event_type_label->Bind(wxEVT_LEFT_DOWN, event_type_label_click_callback);
	event_type_label->Bind(wxEVT_LEFT_DCLICK, event_type_label_click_callback);

	this->event_type_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
	event_type_sizer->Add(this->event_type_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP));

	for (int i = 0; i < event_type_manager->event_types.size(); i++)
	{
		EventType* event_type = event_type_manager->event_types[i];
		this->event_type_list_box->Append(event_type->name, event_type);

		if (this->window->sequence_editor->filtered_event_types.find(event_type) != this->window->sequence_editor->filtered_event_types.end())
		{
			this->event_type_list_box->SetSelection(this->event_type_list_box->GetCount() - 1);
		}
	}

	wxBoxSizer* track_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(track_sizer, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* track_label = new wxStaticText(this, wxID_ANY, "&Track");
	track_sizer->Add(track_label, wxSizerFlags().Align(wxALIGN_CENTER).Border(wxLEFT | wxRIGHT));

	std::function<void (wxMouseEvent&)> track_label_click_callback = [=](wxMouseEvent& event) {
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
	};

	track_label->Bind(wxEVT_LEFT_DOWN, track_label_click_callback);
	track_label->Bind(wxEVT_LEFT_DCLICK, track_label_click_callback);

	this->track_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
	track_sizer->Add(this->track_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP | wxLEFT | wxRIGHT));

	for (MidiFileTrack_t track = MidiFile_getFirstTrack(this->window->sequence_editor->sequence->midi_file); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		this->track_list_box->Append(wxString::Format("%d", MidiFileTrack_getNumber(track)), track);

		if (this->window->sequence_editor->filtered_tracks.find(track) != this->window->sequence_editor->filtered_tracks.end())
		{
			this->track_list_box->SetSelection(this->track_list_box->GetCount() - 1);
		}
	}

	wxBoxSizer* channel_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(channel_sizer, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* channel_label = new wxStaticText(this, wxID_ANY, "&Channel");
	channel_sizer->Add(channel_label, wxSizerFlags().Align(wxALIGN_CENTER));

	std::function<void (wxMouseEvent&)> channel_label_click_callback = [=](wxMouseEvent& event) {
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
	};

	channel_label->Bind(wxEVT_LEFT_DOWN, channel_label_click_callback);
	channel_label->Bind(wxEVT_LEFT_DCLICK, channel_label_click_callback);

	this->channel_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
	channel_sizer->Add(this->channel_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP));

	for (int channel = 0; channel < 16; channel++)
	{
		this->channel_list_box->Append(wxString::Format("%d", channel + 1));

		if (this->window->sequence_editor->filtered_channels.find(channel) != this->window->sequence_editor->filtered_channels.end())
		{
			this->channel_list_box->SetSelection(this->channel_list_box->GetCount() - 1);
		}
	}

	wxSizer* button_sizer = this->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags().Align(wxALIGN_CENTER).Border());

	outer_sizer->Fit(this);
}

void PreferencesDialog::Run(Window* window)
{
	PreferencesDialog* dialog = new PreferencesDialog(window);

	if (dialog->ShowModal() == wxID_OK)
	{
		window->sequence_editor->event_list->font = dialog->event_list_font_picker->GetSelectedFont();

		long first_note = GetNoteNumberFromName(dialog->piano_roll_first_note_text_box->GetValue());
		long last_note = GetNoteNumberFromName(dialog->piano_roll_last_note_text_box->GetValue());

		if ((first_note >= 0) && (first_note < 128) && (last_note >= 0) && (last_note < 128) && (first_note <= last_note))
		{
			window->sequence_editor->piano_roll->first_note = first_note;
			window->sequence_editor->piano_roll->last_note = last_note;
		}

		long key_width = -1;
		dialog->piano_roll_key_width_text_box->GetValue().ToCLong(&key_width);
		if (key_width >= 0) window->sequence_editor->piano_roll->key_width = key_width;

		window->sequence_editor->RefreshData();
	}

	dialog->Destroy();
}

PreferencesDialog::PreferencesDialog(Window* window): wxDialog(NULL, wxID_ANY, "Preferences")
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

	default_event_list_font_button->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
		this->event_list_font_picker->SetSelectedFont(this->window->application->default_event_list_font);
		event.Skip();
	});

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

	default_piano_roll_first_note_button->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
		this->piano_roll_first_note_text_box->SetValue(GetNoteNameFromNumber(this->window->application->default_piano_roll_first_note));
		event.Skip();
	});

	piano_roll_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "&Last note"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->piano_roll_last_note_text_box = new wxTextCtrl(this, wxID_ANY, GetNoteNameFromNumber(this->window->sequence_editor->piano_roll->last_note));
	piano_roll_settings_sizer->Add(this->piano_roll_last_note_text_box, wxSizerFlags().Expand());
	wxButton* default_piano_roll_last_note_button = new wxButton(this, wxID_ANY, "Default");
	piano_roll_settings_sizer->Add(default_piano_roll_last_note_button, wxSizerFlags().Expand());

	default_piano_roll_last_note_button->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
		this->piano_roll_last_note_text_box->SetValue(GetNoteNameFromNumber(this->window->application->default_piano_roll_last_note));
		event.Skip();
	});

	piano_roll_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "&Key width"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->piano_roll_key_width_text_box = new wxTextCtrl(this, wxID_ANY, wxString::Format("%ld", this->window->sequence_editor->piano_roll->key_width));
	piano_roll_settings_sizer->Add(this->piano_roll_key_width_text_box, wxSizerFlags().Expand());
	wxButton* default_piano_roll_key_width_button = new wxButton(this, wxID_ANY, "Default");
	piano_roll_settings_sizer->Add(default_piano_roll_key_width_button, wxSizerFlags().Expand());

	default_piano_roll_key_width_button->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
		this->piano_roll_key_width_text_box->SetValue(wxString::Format("%ld", this->window->application->default_piano_roll_key_width));
		event.Skip();
	});

	wxSizer* button_sizer = this->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags().Align(wxALIGN_CENTER).Border());

	outer_sizer->Fit(this);
}

