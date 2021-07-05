#ifndef INSPECTOR_SIDEBAR_INCLUDED
#define INSPECTOR_SIDEBAR_INCLUDED

class InspectorSidebar;

#include <QWidget>
#include "window.h"

class InspectorSidebar: public QWidget
{
	Q_OBJECT

public:
	Window* window;

	InspectorSidebar(Window* window);
};

#endif
