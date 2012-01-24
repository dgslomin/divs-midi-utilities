
#include <wx/wx.h>
#include <wx/grid.h>
#include <midifile.h>

#define ID_RESIZE_GRID wxID_HIGHEST + 1

class Seqer: public wxApp
{
public:
	bool OnInit();
	void OnMenuHighlight(wxMenuEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnGridScroll(wxScrollWinEvent& event);
	void OnGridResize(wxCommandEvent& event);
	void RedrawGrid();
	MidiFile_t midi_file;
	wxFrame* main_window;
	wxGrid* main_grid;
};

IMPLEMENT_APP(Seqer)

bool Seqer::OnInit()
{
	this->midi_file = NULL;

	this->main_window = new wxFrame((wxFrame*)(NULL), -1, "Seqer", wxDefaultPosition, wxSize(640, 480));
	this->SetTopWindow(this->main_window);

	wxMenuBar* menu_bar = new wxMenuBar();
	this->main_window->SetMenuBar(menu_bar);
	this->main_window->Connect(wxEVT_MENU_HIGHLIGHT, wxMenuEventHandler(Seqer::OnMenuHighlight));
	wxMenu* file_menu = new wxMenu();
	menu_bar->Append(file_menu, "&File");
	file_menu->Append(wxID_NEW, "&New\tCtrl+N");
	file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O");
	this->Connect(wxID_OPEN, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seqer::OnFileOpen));
	file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
	file_menu->Append(wxID_SAVEAS, "Save &As...");
	file_menu->Append(wxID_REVERT, "&Revert");
	file_menu->AppendSeparator();
	file_menu->Append(wxID_EXIT, "E&xit");
	this->Connect(wxID_EXIT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seqer::OnExit));
	wxMenu* edit_menu = new wxMenu();
	menu_bar->Append(edit_menu, "&Edit");
	edit_menu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
	edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Y");
	edit_menu->AppendSeparator();
	edit_menu->Append(wxID_CUT, "Cu&t\tCtrl+X");
	edit_menu->Append(wxID_COPY, "&Copy\tCtrl+C");
	edit_menu->Append(wxID_PASTE, "&Paste\tCtrl+V");
	wxMenu* view_menu = new wxMenu();
	menu_bar->Append(view_menu, "&View");
	wxMenu* help_menu = new wxMenu();
	menu_bar->Append(help_menu, "&Help");
	help_menu->Append(wxID_ABOUT, "&About Seqer");

	this->main_grid = new wxGrid(this->main_window, -1); 
	this->main_grid->SetRowLabelSize(40);
	this->main_grid->SetDefaultColSize(40);
	this->main_grid->SetColLabelSize(this->main_grid->GetDefaultRowSize());
	this->main_grid->EnableDragRowSize(false);
	this->main_grid->EnableDragGridSize(false);
	this->main_grid->EnableEditing(false);
	this->main_grid->CreateGrid(128, 128);

	{
		wxString column_label;

		for (int column_number = 0; column_number < 128; column_number++)
		{
			column_label.Printf("%d", column_number + 1);
			this->main_grid->SetColLabelValue(column_number, column_label);
		}
	}

	this->Connect(ID_RESIZE_GRID, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seqer::OnGridResize));
	this->main_grid->Connect(wxEVT_SCROLLWIN_BOTTOM, wxScrollWinEventHandler(Seqer::OnGridScroll));
	this->main_grid->Connect(wxEVT_SCROLLWIN_LINEDOWN, wxScrollWinEventHandler(Seqer::OnGridScroll));
	this->main_grid->Connect(wxEVT_SCROLLWIN_PAGEDOWN, wxScrollWinEventHandler(Seqer::OnGridScroll));
	this->main_grid->Connect(wxEVT_SCROLLWIN_THUMBRELEASE, wxScrollWinEventHandler(Seqer::OnGridScroll));

	this->main_window->CreateStatusBar();

	this->main_window->Show(true);
	return true;
}

void Seqer::OnMenuHighlight(wxMenuEvent& WXUNUSED(event))
{
	// This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
}

void Seqer::OnFileOpen(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog* file_dialog = new wxFileDialog(this->main_window, "Open File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (file_dialog->ShowModal() == wxID_OK)
	{
		if (this->midi_file != NULL) MidiFile_free(this->midi_file);
		this->midi_file = MidiFile_load((char *)(file_dialog->GetPath().c_str()));
		this->RedrawGrid();
	}

	delete file_dialog;
}

void Seqer::OnExit(wxCommandEvent& WXUNUSED(event))
{
	this->main_window->Close(true);
}

void Seqer::OnGridScroll(wxScrollWinEvent& event)
{
	event.Skip();
	wxCommandEvent resize_grid_event(wxEVT_COMMAND_MENU_SELECTED, ID_RESIZE_GRID);
	this->AddPendingEvent(resize_grid_event);
}

void Seqer::OnGridResize(wxCommandEvent& WXUNUSED(event))
{
	if (this->main_grid->GetScrollPos(wxHORIZONTAL) + this->main_grid->GetScrollThumb(wxHORIZONTAL) == this->main_grid->GetScrollRange(wxHORIZONTAL)) this->main_grid->AppendCols(this->main_grid->GetClientSize().x / this->main_grid->GetDefaultColSize());
	if (this->main_grid->GetScrollPos(wxVERTICAL) + this->main_grid->GetScrollThumb(wxVERTICAL) == this->main_grid->GetScrollRange(wxVERTICAL)) this->main_grid->AppendRows(this->main_grid->GetClientSize().y / this->main_grid->GetDefaultRowSize());
}

void Seqer::RedrawGrid()
{
	this->main_grid->BeginBatch();
	this->main_grid->ClearGrid();

	if (this->midi_file != NULL)
	{
		int row_number = 0;

		for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
			if (MidiFileEvent_isNoteStartEvent(event))
			{
				long time = MidiFileEvent_getTick(event);
				long duration = MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(event)) - time;
				int track = MidiFileTrack_getNumber(MidiFileEvent_getTrack(event));
				int channel = MidiFileNoteStartEvent_getChannel(event);
				int note = MidiFileNoteStartEvent_getNote(event);
				int velocity = MidiFileNoteStartEvent_getVelocity(event);

				{
					wxString cell_value;
					cell_value.Printf("n %ld %ld %d %d %d %d", time, duration, track, channel, note, velocity);
					this->main_grid->SetCellValue(row_number, 0, cell_value);
					row_number++;
					if (row_number == this->main_grid->GetNumberRows()) this->main_grid->AppendRows(1);
				}
			}
		}
	}

	this->main_grid->EndBatch();
}

