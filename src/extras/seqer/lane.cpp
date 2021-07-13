
#include <QAction>
#include <QBrush>
#include <QColor>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QSettings>
#include <QWidget>
#include "lane.h"
#include "midifile.h"
#include "window.h"

static QColor themeColor(int lightness, int dark_theme_lightness)
{
	int theme_hue, theme_saturation, theme_lightness;
	QGuiApplication::palette().color(QPalette::Button).getHsl(&theme_hue, &theme_saturation, &theme_lightness);
	bool dark_theme = (theme_lightness < 127);
	return QColor::fromHsl(theme_hue, theme_saturation, dark_theme ? dark_theme_lightness : lightness);
}

Lane::Lane(Window* window)
{
	this->window = window;
	this->setFocusPolicy(Qt::StrongFocus);

	QAction* edit_event_action = new QAction(tr("Edit Event"));
	this->addAction(edit_event_action);
	edit_event_action->setShortcut(QKeySequence(Qt::Key_Return));
	connect(edit_event_action, SIGNAL(triggered()), this, SLOT(editEvent()));

	QAction* select_event_action = new QAction(tr("Select Event"));
	this->addAction(select_event_action);
	select_event_action->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return));
	connect(select_event_action, SIGNAL(triggered()), this, SLOT(selectEvent()));

	QAction* cursor_left_action = new QAction(tr("Cursor Left"));
	this->addAction(cursor_left_action);
	cursor_left_action->setShortcut(QKeySequence(Qt::Key_Left));
	connect(cursor_left_action, SIGNAL(triggered()), this, SLOT(cursorLeft()));

	QAction* cursor_right_action = new QAction(tr("Cursor Right"));
	this->addAction(cursor_right_action);
	cursor_right_action->setShortcut(QKeySequence(Qt::Key_Right));
	connect(cursor_right_action, SIGNAL(triggered()), this, SLOT(cursorRight()));

	QAction* cursor_up_action = new QAction(tr("Cursor Up"));
	this->addAction(cursor_up_action);
	cursor_up_action->setShortcut(QKeySequence(Qt::Key_Up));
	connect(cursor_up_action, SIGNAL(triggered()), this, SLOT(cursorUp()));

	QAction* cursor_down_action = new QAction(tr("Cursor Down"));
	this->addAction(cursor_down_action);
	cursor_down_action->setShortcut(QKeySequence(Qt::Key_Down));
	connect(cursor_down_action, SIGNAL(triggered()), this, SLOT(cursorDown()));

	QSettings settings;
	this->background_color = settings.value("lane/background-color", themeColor(255, 0)).value<QColor>();
	this->white_note_background_color = settings.value("lane/white-note-background-color", themeColor(255, 0)).value<QColor>();
	this->black_note_background_color = settings.value("lane/black-note-background-color", themeColor(230, 50)).value<QColor>();
	this->unselected_event_pen = QPen(settings.value("lane/unselected-event-border-color", themeColor(0, 180)).value<QColor>());
	this->unselected_event_brush = QBrush(settings.value("lane/unselected-event-color", themeColor(255, 0)).value<QColor>());
	this->unselected_event_text_pen = QPen(settings.value("lane/unselected-event-text-color", themeColor(0, 255)).value<QColor>());
	this->selected_event_pen = QPen(settings.value("lane/selected-event-border-color", themeColor(0, 180)).value<QColor>());
	this->selected_event_brush = QBrush(settings.value("lane/selected-event-color", themeColor(200, 100)).value<QColor>());
	this->selected_event_text_pen = QPen(settings.value("lane/selected-event-text-color", themeColor(255, 0)).value<QColor>());
	this->cursor_pen = QPen(settings.value("lane/cursor-color", themeColor(100, 200)).value<QColor>());
	this->cursor_brush = QBrush(settings.value("lane/cursor-color", themeColor(100, 200)).value<QColor>());
	this->selection_rect_pen = QPen(settings.value("lane/selection-rect-color", themeColor(0, 180)).value<QColor>(), 1, Qt::DashLine);
	this->mouse_drag_threshold = settings.value("lane/mouse-drag-threshold", 8).toInt();
}

void Lane::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	this->paintBackground(&painter);

	int selected_events_x_offset = 0;
	int selected_events_y_offset = 0;

	if (this->mouse_operation == LANE_MOUSE_OPERATION_RECT_SELECT)
	{
 		if ((this->mouse_drag_x != this->mouse_down_x) && (this->mouse_drag_y != this->mouse_down_y))
		{
			painter.setPen(this->selection_rect_pen);
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(this->mouse_down_x, this->mouse_down_y, this->mouse_drag_x - this->mouse_down_x, this->mouse_drag_y - this->mouse_down_y);
		}
	}
	else
	{
		if (this->mouse_drag_x_allowed) selected_events_x_offset = this->mouse_drag_x - this->mouse_down_x;
		if (this->mouse_drag_y_allowed) selected_events_y_offset = this->mouse_drag_y - this->mouse_down_y;
	}

	this->paintEvents(&painter, selected_events_x_offset, selected_events_y_offset);

	painter.setPen(this->cursor_pen);
	painter.setBrush(this->cursor_brush);
	painter.drawLine(this->window->cursor_x, 0, this->window->cursor_x, this->cursor_y);
	QPoint upper_arrowhead[] = { QPoint(this->window->cursor_x, this->cursor_y), QPoint(this->window->cursor_x - 2, this->cursor_y - 2), QPoint(this->window->cursor_x + 2, this->cursor_y - 2) };
	painter.drawConvexPolygon(upper_arrowhead, 3);
	painter.drawLine(this->window->cursor_x, cursor_y + 6, this->window->cursor_x, this->height());
	QPoint lower_arrowhead[] = { QPoint(this->window->cursor_x, this->cursor_y + 6), QPoint(this->window->cursor_x - 2, this->cursor_y + 8), QPoint(this->window->cursor_x + 2, this->cursor_y + 8) };
	painter.drawConvexPolygon(lower_arrowhead, 3);
}

void Lane::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		this->mouse_operation = LANE_MOUSE_OPERATION_NONE;
		this->mouse_down_x = event->position().x();
		this->mouse_down_y = event->position().y();
		this->mouse_drag_x = this->mouse_down_x;
		this->mouse_drag_y = this->mouse_down_y;
		this->mouse_drag_x_allowed = false;
		this->mouse_drag_y_allowed = false;

		MidiFileEvent_t midi_event = this->getEventFromXY(this->mouse_down_x, this->mouse_down_y);
		bool shift = ((event->modifiers() & Qt::ShiftModifier) != 0);

		if (midi_event == NULL)
		{
			this->mouse_operation = LANE_MOUSE_OPERATION_RECT_SELECT;

 			if (!shift)
			{
				this->window->selectNone();

				if ((this->mouse_down_x == this->window->cursor_x) && (this->mouse_down_y == this->cursor_y))
				{
					midi_event = this->addEventAtXY(this->mouse_down_x, this->mouse_down_y);
					MidiFileEvent_setSelected(midi_event, 1);
					this->mouse_operation = LANE_MOUSE_OPERATION_NONE;
				}
			}
		}
		else
		{
			if (MidiFileEvent_isSelected(midi_event))
			{
				if (shift)
				{
					MidiFileEvent_setSelected(midi_event, 0);
					this->mouse_operation = LANE_MOUSE_OPERATION_NONE;
				}
				else
				{
					this->mouse_operation = LANE_MOUSE_OPERATION_DRAG_EVENTS;
				}
			}
			else
			{
 				if (shift)
				{
					MidiFileEvent_setSelected(midi_event, 1);
					this->mouse_operation = LANE_MOUSE_OPERATION_NONE;
				}
				else
				{
					this->window->selectNone();
					MidiFileEvent_setSelected(midi_event, 1);
					this->mouse_operation = LANE_MOUSE_OPERATION_DRAG_EVENTS;
				}
			}
		}

		this->window->sequence->updateWindows();
	}
}

void Lane::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		int mouse_up_x = event->position().x();
		int mouse_up_y = event->position().y();

		if (this->mouse_operation == LANE_MOUSE_OPERATION_DRAG_EVENTS)
		{
			if ((this->mouse_drag_x_allowed && (mouse_up_x != this->mouse_down_x)) || (this->mouse_drag_y_allowed && (mouse_up_y != this->mouse_down_y)))
			{
				int x_offset = this->mouse_drag_x_allowed ? (this->mouse_drag_x - this->mouse_down_x) : 0;
				int y_offset = this->mouse_drag_y_allowed ? (this->mouse_drag_y - this->mouse_down_y) : 0;
				this->moveEventsByXY(x_offset, y_offset);
			}
			else
			{
				QPoint midi_event_position = this->getPointFromEvent(this->getEventFromXY(this->mouse_down_x, this->mouse_down_y));

				if ((this->window->cursor_x == midi_event_position.x()) && (this->cursor_y == midi_event_position.y()))
				{
					this->window->focusInspector();
				}
				else
				{
					this->window->cursor_x = midi_event_position.x();
					this->cursor_y = midi_event_position.y();
				}
			}
		}
		else if (this->mouse_operation == LANE_MOUSE_OPERATION_RECT_SELECT)
		{
			if ((mouse_up_x == this->mouse_down_x) && (mouse_up_y == this->mouse_down_y))
			{
				this->window->cursor_x = mouse_up_x;
				this->cursor_y = mouse_up_y;
			}
			else
			{
				this->selectEventsInRect(std::min(this->mouse_down_x, mouse_up_x), std::min(this->mouse_down_y, mouse_up_y), std::abs(mouse_up_x - this->mouse_down_x), std::abs(mouse_up_y - this->mouse_down_y));
			}
		}

		this->mouse_operation = LANE_MOUSE_OPERATION_NONE;
		this->mouse_drag_x_allowed = false;
		this->mouse_drag_y_allowed = false;
		this->window->sequence->updateWindows();
	}
}

void Lane::mouseMoveEvent(QMouseEvent* event)
{
	if (this->mouse_operation != LANE_MOUSE_OPERATION_NONE)
	{
		this->mouse_drag_x = event->position().x();
		this->mouse_drag_y = event->position().y();
		if (abs(this->mouse_drag_x - this->mouse_down_x) > this->mouse_drag_threshold) this->mouse_drag_x_allowed = true;
		if (abs(this->mouse_drag_y - this->mouse_down_y) > this->mouse_drag_threshold) this->mouse_drag_y_allowed = true;
		this->update();
	}
}

void Lane::wheelEvent(QWheelEvent* event)
{
	int pixel_delta_x = event->angleDelta().x() / 8;
	int pixel_delta_y = event->angleDelta().y() / 8;
	bool control = ((event->modifiers() & Qt::ControlModifier) != 0);

	if (control)
	{
		if (this->window->use_linear_time)
		{
			this->window->pixels_per_second = std::max(this->window->pixels_per_second + pixel_delta_x, 1);
		}
		else
		{
			this->window->pixels_per_beat = std::max(this->window->pixels_per_beat + pixel_delta_x, 1);
		}

		this->zoomYBy(-pixel_delta_y / 8);
	}
	else
	{
		this->window->scroll_x -= pixel_delta_x;
		this->scrollYBy(pixel_delta_y);
	}

	this->window->update();
}

void Lane::editEvent()
{
	MidiFileEvent_t cursor_midi_event = this->getEventFromXY(this->window->cursor_x, this->cursor_y);

	if (cursor_midi_event == NULL)
	{
		this->window->selectNone();
		cursor_midi_event = this->addEventAtXY(this->window->cursor_x, this->cursor_y);
		MidiFileEvent_setSelected(cursor_midi_event, 1);
		this->window->sequence->updateWindows();
	}
	else
	{
		if (MidiFileEvent_isSelected(cursor_midi_event))
		{
			this->window->focusInspector();
		}
		else
		{
			this->window->selectNone();
			MidiFileEvent_setSelected(cursor_midi_event, 1);
			this->window->sequence->updateWindows();
		}
	}
}

void Lane::selectEvent()
{
	MidiFileEvent_t cursor_midi_event = this->getEventFromXY(this->window->cursor_x, this->cursor_y);

	if (cursor_midi_event != NULL)
	{
		MidiFileEvent_setSelected(cursor_midi_event, !MidiFileEvent_isSelected(cursor_midi_event));
		this->window->sequence->updateWindows();
	}
}

void Lane::cursorLeft()
{
	this->window->cursor_x--;
	this->window->update();
}

void Lane::cursorRight()
{
	this->window->cursor_x++;
	this->window->update();
}

void Lane::cursorUp()
{
	this->cursor_y--;
	this->update();
}

void Lane::cursorDown()
{
	this->cursor_y++;
	this->update();
}

