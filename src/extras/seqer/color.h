#ifndef COLOR_INCLUDED
#define COLOR_INCLUDED

#include <wx/wx.h>

// RGB = [0, 256); H = [0, 360); S = [0, 100); L = [0, 100)
void RgbToHsl(const unsigned int r, const unsigned int g, const unsigned int b, unsigned int& h, unsigned int& s, unsigned int& l);
void HslToRgb(const unsigned int h, const unsigned int s, const unsigned int l, unsigned int& r, unsigned int& g, unsigned int& b);

wxColour ColorShade(const wxColour input, const unsigned int luminance);

#endif
