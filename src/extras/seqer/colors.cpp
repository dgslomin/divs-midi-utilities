
#include <QColor>
#include <QGuiApplication>
#include <QPalette>
#include "colors.h"

QColor Colors::shade(QColor base_color, int light_theme_lightness, int dark_theme_lightness)
{
	int base_hue, base_saturation, base_lightness;
	base_color.getHsl(&base_hue, &base_saturation, &base_lightness);
	bool dark_theme = (QGuiApplication::palette().color(QPalette::Button).lightness() < 127);
	return QColor::fromHsl(base_hue, base_saturation, dark_theme ? dark_theme_lightness : light_theme_lightness);
}

QColor Colors::buttonShade(int light_theme_lightness, int dark_theme_lightness)
{
	return Colors::shade(QGuiApplication::palette().color(QPalette::Button), light_theme_lightness, dark_theme_lightness);
}

QColor Colors::textShade(int light_theme_lightness, int dark_theme_lightness)
{
	return Colors::shade(QGuiApplication::palette().color(QPalette::ButtonText), light_theme_lightness, dark_theme_lightness);
}

