
#include <QWidget>
#include <QFormLayout>
#include <QFrame>
#include <QLineEdit>
#include "inspector-sidebar.h"
#include "window.h"

InspectorSidebar::InspectorSidebar(Window* window)
{
	this->window = window;
	QFormLayout* layout = new QFormLayout(this);

	QLineEdit* lane_type_line_edit = new QLineEdit();
	layout->addRow(tr("Lane Type"), lane_type_line_edit);

	QLineEdit* lane_controller_line_edit = new QLineEdit();
	layout->addRow(tr("Controller"), lane_controller_line_edit);

	QFrame* separator = new QFrame();
	layout->addRow(separator);
	separator->setFrameShape(QFrame::HLine);
	separator->setFrameShadow(QFrame::Sunken);
	separator->setMinimumHeight(16);

	QLineEdit* event_type_line_edit = new QLineEdit();
	layout->addRow(tr("Event Type"), event_type_line_edit);

	QLineEdit* event_time_line_edit = new QLineEdit();
	layout->addRow(tr("Time"), event_time_line_edit);
}

