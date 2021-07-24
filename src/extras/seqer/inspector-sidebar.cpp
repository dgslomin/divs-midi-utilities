
#include <QtWidgets>
#include "inspector-sidebar.h"
#include "separator.h"
#include "window.h"

InspectorSidebar::InspectorSidebar(Window* window)
{
	this->window = window;

	QVBoxLayout* outer_layout = new QVBoxLayout(this);
	outer_layout->setSpacing(0);
	outer_layout->setContentsMargins(0, 0, 0, 0);

	QWidget* lane_panel = new QWidget();
	outer_layout->addWidget(lane_panel);
	QFormLayout* lane_layout = new QFormLayout(lane_panel);
	lane_layout->setSpacing(4);
	lane_layout->setContentsMargins(8, 8, 8, 8);

	QLineEdit* lane_type_line_edit = new QLineEdit();
	lane_layout->addRow(tr("Lane Type"), lane_type_line_edit);

	QLineEdit* lane_controller_line_edit = new QLineEdit();
	lane_layout->addRow(tr("Controller"), lane_controller_line_edit);

	outer_layout->addWidget(new Separator(Qt::Horizontal));

	QWidget* event_panel = new QWidget();
	outer_layout->addWidget(event_panel, 1);
	QFormLayout* event_layout = new QFormLayout(event_panel);
	event_layout->setSpacing(4);
	event_layout->setContentsMargins(8, 8, 8, 8);

	QLineEdit* event_type_line_edit = new QLineEdit();
	event_layout->addRow(tr("Event Type"), event_type_line_edit);

	QLineEdit* event_time_line_edit = new QLineEdit();
	event_layout->addRow(tr("Time"), event_time_line_edit);
}

