#ifndef MIDIUTIL_WX_INCLUDED
#define MIDIUTIL_WX_INCLUDED

/* Common helpers that work with wx. */

#include <wx/wx.h>

wxString MidiUtil_getNameFromWxKeyCode(wxKeyCode key_code);
wxKeyCode MidiUtil_getWxKeyCodeFromName(wxString key_name);

#endif
