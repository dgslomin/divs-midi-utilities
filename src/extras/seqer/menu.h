#ifndef MENU_INCLUDED
#define MENU_INCLUDED

class Menu;

#include <QtWidgets>
#include "window.h"

class Menu: public QObject
{
	Q_OBJECT

public:
	static void createMenuBar(Window* window);

private:
	Menu() {}
	static void createFileMenu(Window* window);
	static void createEditMenu(Window* window);
	static void createViewMenu(Window* window);
	static void createTransportMenu(Window* window);
	static void createToolsMenu(Window* window);
	static void createHelpMenu(Window* window);
};

#endif
