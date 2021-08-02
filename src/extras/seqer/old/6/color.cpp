
#include <wx/wx.h>

// Adapted from public domain code by John Ratcliff
// http://codesuppository.blogspot.com/2012/06/hsl2rgb-convert-rgb-colors-to-hsl-or.html

void RgbToHsl(unsigned int r, unsigned int g, unsigned int b, unsigned int& h, unsigned int& s, unsigned int& l)
{
	float r_percent = ((float)r)/255;
	float g_percent = ((float)g)/255;
	float b_percent = ((float)b)/255;

	float max_color = 0;
	if((r_percent >= g_percent) && (r_percent >= b_percent))
	{
		max_color = r_percent;
	}
	if((g_percent >= r_percent) && (g_percent >= b_percent))
		max_color = g_percent;
	if((b_percent >= r_percent) && (b_percent >= g_percent))
		max_color = b_percent;

	float min_color = 0;
	if((r_percent <= g_percent) && (r_percent <= b_percent))
		min_color = r_percent;
	if((g_percent <= r_percent) && (g_percent <= b_percent))
		min_color = g_percent;
	if((b_percent <= r_percent) && (b_percent <= g_percent))
		min_color = b_percent;

	float L = 0;
	float S = 0;
	float H = 0;

	L = (max_color + min_color)/2;

	if(max_color == min_color)
	{
		S = 0;
		H = 0;
	}
	else
	{
		if(L < .50)
		{
			S = (max_color - min_color)/(max_color + min_color);
		}
		else
		{
			S = (max_color - min_color)/(2 - max_color - min_color);
		}
		if(max_color == r_percent)
		{
			H = (g_percent - b_percent)/(max_color - min_color);
		}
		if(max_color == g_percent)
		{
			H = 2 + (b_percent - r_percent)/(max_color - min_color);
		}
		if(max_color == b_percent)
		{
			H = 4 + (r_percent - g_percent)/(max_color - min_color);
		}
	}
	s = (unsigned int)(S*100);
	l = (unsigned int)(L*100);
	H = H*60;
	if(H < 0)
		H += 360;
	h = (unsigned int)H;
}

static void HslToRgbHelper(unsigned int& c, float& temp1, float& temp2, float& temp3)
{
	if((temp3 * 6) < 1)
		c = (unsigned int)((temp2 + (temp1 - temp2)*6*temp3)*100);
	else
		if((temp3 * 2) < 1)
			c = (unsigned int)(temp1*100);
		else
			if((temp3 * 3) < 2)
				c = (unsigned int)((temp2 + (temp1 - temp2)*(.66666 - temp3)*6)*100);
			else
				c = (unsigned int)(temp2*100);
	return;
}

void HslToRgb(unsigned int h, unsigned int s, unsigned int l, unsigned int& r, unsigned int& g, unsigned int& b)
{
	float L = ((float)l)/100;
	float S = ((float)s)/100;
	float H = ((float)h)/360;

	if(s == 0)
	{
		r = l;
		g = l;
		b = l;
	}
	else
	{
		float temp1 = 0;
		if(L < .50)
		{
			temp1 = L*(1 + S);
		}
		else
		{
			temp1 = L + S - (L*S);
		}

		float temp2 = 2*L - temp1;

		float temp3 = 0;
		for(int i = 0 ; i < 3 ; i++)
		{
			switch(i)
			{
			case 0: // red
				{
					temp3 = H + .33333f;
					if(temp3 > 1)
						temp3 -= 1;
					HslToRgbHelper(r,temp1,temp2,temp3);
					break;
				}
			case 1: // green
				{
					temp3 = H;
					HslToRgbHelper(g,temp1,temp2,temp3);
					break;
				}
			case 2: // blue
				{
					temp3 = H - .33333f;
					if(temp3 < 0)
						temp3 += 1;
					HslToRgbHelper(b,temp1,temp2,temp3);
					break;
				}
			default:
				{

				}
			}
		}
	}
	r = (unsigned int)((((float)r)/100)*255);
	g = (unsigned int)((((float)g)/100)*255);
	b = (unsigned int)((((float)b)/100)*255);
}

wxColour ColorShade(wxColour input, unsigned int luminance)
{
	unsigned int h = 0;
	unsigned int s = 0;
	unsigned int l = 0;
	RgbToHsl(input.Red(), input.Green(), input.Blue(), h, s, l);
	unsigned int r = 0;
	unsigned int g = 0;
	unsigned int b = 0;
	HslToRgb(h, s, luminance, r, g, b);
	return wxColour(r, g, b);
}

