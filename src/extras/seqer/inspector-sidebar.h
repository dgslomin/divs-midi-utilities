#ifndef INSPECTOR_SIDEBAR_INCLUDED
#define INSPECTOR_SIDEBAR_INCLUDED

class InspectorSidebar;

#include <QtWidgets>
#include "window.h"

class InspectorSidebar: public QWidget
{
	Q_OBJECT

public:
	Window* window;
	QLineEdit* lane_type_line_edit;
	QLineEdit* lane_controller_line_edit;
	QLineEdit* event_type_line_edit;
	QLineEdit* time_line_edit;
	QLineEdit* end_time_line_edit;

	InspectorSidebar(Window* window);
	void paintEvent(QPaintEvent* event);
	void sequenceUpdated();
};

#endif
