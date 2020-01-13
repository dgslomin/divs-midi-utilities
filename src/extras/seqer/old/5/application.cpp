
#include <wx/wx.h>
#include "application.h"
#include "window.h"

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	if (wxGetEnv("SEQER_DEBUG", NULL)) wxMessageBox("Attach debugger now.", "Seqer", wxOK);

	Window* window = new Window(this);
	window->Show(true);

	if (this->argc > 1)
	{
		if (!window->Load(this->argv[1]))
		{
			wxMessageBox("Cannot open the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
		}
	}

	return true;
}

