
#include <QtWidgets>
#include "touchmidi.h"

TouchPanel::TouchPanel()
{
	this->setAttribute(Qt::WA_AcceptTouchEvents, true);
}

bool TouchPanel::event(QEvent* event)
{
	switch (event->type())
	{
		case QEvent::TouchBegin:
		case QEvent::TouchUpdate:
		case QEvent::TouchEnd:
		{
			this->touchEvent(static_cast<QTouchEvent*>(event));
			return true;
		}
		default:
		{
			return QWidget::event(event);
		}
	}
}

void TouchPanel::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)

	QPainter painter(this);
}

void TouchPanel::touchEvent(QTouchEvent* event)
{
	for (int touch_point_number = 0; touch_point_number < event->touchPoints().size(); touch_point_number++)
	{
		QTouchEvent::TouchPoint touch_point = event->touchPoints()[touch_point_number];
		QString state;

		switch (touch_point.state())
		{
			case Qt::TouchPointPressed:
			{
				state = "Pressed";
				break;
			}
			case Qt::TouchPointMoved:
			{
				state = "Moved";
				break;
			}
			case Qt::TouchPointStationary:
			{
				state = "Stationary";
				break;
			}
			case Qt::TouchPointReleased:
			{
				state = "Released";
				break;
			}
			default:
			{
				state = "???";
				break;
			}
		}

		QPointF start_pos = touch_point.startPos();
		QPointF pos = touch_point.pos();
		qDebug("touch %d: start(%f, %f), current(%f, %f) -> %s", touch_point_number, start_pos.x(), start_pos.y(), pos.x(), pos.y(), state.toStdString().c_str());
	}
}

int main(int argc, char** argv)
{
	QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

	QApplication application(argc, argv);
	application.setOrganizationName("Sreal");
	application.setOrganizationDomain("sreal.com");
	application.setApplicationName("TouchMIDI");

	QMainWindow* window = new QMainWindow();
	QWidget* panel = new QWidget();
	window->setCentralWidget(panel);
	QVBoxLayout* layout = new QVBoxLayout(panel);

	TouchPanel* upper = new TouchPanel();
	layout->addWidget(upper, 1);
	upper->setAutoFillBackground(true);
	upper->setBackgroundRole(QPalette::Button);

	TouchPanel* lower = new TouchPanel();
	layout->addWidget(lower, 1);
	lower->setAutoFillBackground(true);
	lower->setBackgroundRole(QPalette::Dark);

	window->show();

	return application.exec();
}

