
#include <QAction>
#include <QBrush>
#include <QColor>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QSettings>
#include <QWidget>
#include "lane.h"
#include "midifile.h"
#include "window.h"

Lane::Lane(Window* window)
{
	this->window = window;

	QAction* edit_event_action = new QAction(tr("Edit Event"));
	this->addAction(edit_event_action);
	edit_event_action->setShortcut(QKeySequence(Qt::Key_Return));
	connect(edit_event_action, SIGNAL(triggered()), this, SLOT(editEvent()));

	QAction* select_event_action = new QAction(tr("Select Event"));
	this->addAction(select_event_action);
	select_event_action->setShortcut(QKeySequence(Qt::Key_Space));
	connect(select_event_action, SIGNAL(triggered()), this, SLOT(selectEvent()));

	QAction* toggle_event_selection_action = new QAction(tr("Toggle Event Selection"));
	this->addAction(toggle_event_selection_action);
	toggle_event_selection_action->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Space));
	connect(toggle_event_selection_action, SIGNAL(triggered()), this, SLOT(toggleEventSelection()));

	QSettings settings;
	this->background_color = settings.value("lane/background-color", QColorConstants::White).value<QColor>();
	this->white_note_background_color = settings.value("lane/white-note-background-color", QColorConstants::White).value<QColor>();
	this->black_note_background_color = settings.value("lane/black-note-background-color", QColorConstants::LightGray).value<QColor>();
	this->unselected_event_color = settings.value("lane/unselected-event-color", QColorConstants::White).value<QColor>();
	this->unselected_event_border_color = settings.value("lane/unselected-event-text-color", QColorConstants::Black).value<QColor>();
	this->unselected_event_text_color = settings.value("lane/unselected-event-border-color", QColorConstants::Black).value<QColor>();
	this->selected_event_color = settings.value("lane/selected-event-color", QColorConstants::Blue).value<QColor>();
	this->selected_event_border_color = settings.value("lane/selected-event-border-color", QColorConstants::Black).value<QColor>();
	this->selected_event_text_color = settings.value("lane/selected-event-text-color", QColorConstants::White).value<QColor>();
	this->cursor_color = settings.value("lane/cursor-color", QColorConstants::Blue).value<QColor>();
	this->selection_rect_color = settings.value("lane/selection-rect-color", QColorConstants::Blue).value<QColor>();
	this->selection_rect_border_color = settings.value("lane/selection-rect-border-color", QColorConstants::Black).value<QColor>();
	this->mouse_drag_threshold = settings.value("lane/mouse-drag-threshold", 8).toInt();
}

Lane::~Lane()
{
}

void Lane::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);

	this->paintBackground(&painter, this->width(), this->height());

	bool should_paint_selection_rect = false;
	int selected_events_x_offset = 0;
	int selected_events_y_offset = 0;

	if (this->mouse_down && (this->mouse_down_midi_event != NULL))
	{
		should_paint_selection_rect = true;
	}
	else
	{
		if (this->mouse_drag_x_allowed) selected_events_x_offset = this->mouse_drag_x - this->mouse_down_x;
		if (this->mouse_drag_y_allowed) selected_events_y_offset = this->mouse_drag_y - this->mouse_down_y;
	}

	this->paintEvents(&painter, this->width(), this->height(), selected_events_x_offset, selected_events_y_offset);

	painter.setPen(QPen(this->cursor_color));
	painter.drawLine(this->cursor_x, 0, this->cursor_x, this->height());

	if (should_paint_selection_rect)
	{
		painter.setBrush(QBrush(this->selection_rect_color));
		painter.setPen(QPen(this->selection_rect_border_color));
		painter.drawRect(this->mouse_down_x, this->mouse_down_y, this->mouse_drag_x - this->mouse_down_x, this->mouse_drag_y - this->mouse_down_y);
	}
}

void Lane::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		this->mouse_down = true;
		this->mouse_down_x = event->x();
		this->mouse_down_y = event->y();
		this->mouse_down_midi_event = this->getEventFromXY(this->mouse_down_x, this->mouse_down_y);
		this->mouse_down_midi_event_is_new = false;
		this->mouse_drag_x = this->mouse_down_x;
		this->mouse_drag_y = this->mouse_down_y;
		this->mouse_drag_x_allowed = false;
		this->mouse_drag_y_allowed = false;

		if ((this->mouse_down_midi_event == NULL) && (event->modifiers() & Qt::ShiftModifier == 0))
		{
			this->window->selectNone();

			if ((this->mouse_down_x == this->cursor_x) && (this->mouse_down_y == this->cursor_y))
			{
				this->mouse_down_midi_event = this->addEventAtXY(this->mouse_down_x, this->mouse_down_y);
				MidiFileEvent_setSelected(this->mouse_down_midi_event, 1);
				this->mouse_down_midi_event_is_new = true;
			}
		}
	}
}

void Lane::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		int mouse_x = event->x();
		int mouse_y = event->y();

		if (this->mouse_down_midi_event == NULL)
		{
			if ((mouse_x == this->mouse_down_x) && (mouse_y == this->mouse_down_y))
			{
				this->cursor_x = mouse_y;
				this->cursor_y = mouse_y;
			}
			else
			{
				this->selectEventsInRect(std::min(this->mouse_down_x, mouse_x), std::min(this->mouse_down_y, mouse_y), std::abs(mouse_x - this->mouse_down_x), std::abs(mouse_y - this->mouse_down_y));
			}
		}
		else
		{
			if ((this->mouse_drag_x_allowed && (mouse_x != this->mouse_down_x)) || (this->mouse_drag_y_allowed && (mouse_y != this->mouse_down_y)))
			{
				int x_offset = this->mouse_drag_x_allowed ? (this->mouse_drag_x - this->mouse_down_x) : 0;
				int y_offset = this->mouse_drag_y_allowed ? (this->mouse_drag_y - this->mouse_down_y) : 0;

				for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
				{
					if (MidiFileEvent_isSelected(midi_event))
					{
						this->moveEventByXY(midi_event, x_offset, y_offset);
					}
				}
			}
			else
			{
				if ((event->modifiers() & Qt::ShiftModifier == 0) && !this->mouse_down_midi_event_is_new && (mouse_x == this->cursor_x) && (mouse_y == this->cursor_y))
				{
					this->window->focusInspector();
				}
			}
		}

		this->mouse_down = false;
	}
}

void Lane::mouseMoveEvent(QMouseEvent* event)
{
	if (this->mouse_down)
	{
		this->mouse_drag_x = event->x();
		this->mouse_drag_y = event->y();
		if (abs(this->mouse_drag_x - this->mouse_down_x) > this->mouse_drag_threshold) this->mouse_drag_x_allowed = true;
		if (abs(this->mouse_drag_y - this->mouse_down_y) > this->mouse_drag_threshold) this->mouse_drag_y_allowed = true;
		this->window->refreshDisplay();
	}
}

void Lane::editEvent()
{
	bool selection_is_empty = true;

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isSelected(midi_event))
		{
			selection_is_empty = false;
			break;
		}
	}

	if (selection_is_empty)
	{
		MidiFileEvent_t cursor_midi_event = this->getEventFromXY(this->cursor_x, this->cursor_y);

		if (cursor_midi_event == NULL)
		{
			cursor_midi_event = this->addEventAtXY(this->cursor_x, this->cursor_y);
			MidiFileEvent_setSelected(cursor_midi_event, 1);
		}
		else
		{
			MidiFileEvent_setSelected(cursor_midi_event, 1);
			this->window->focusInspector();
		}
	}
	else
	{
		this->window->focusInspector();
	}
}

void Lane::selectEvent()
{
	MidiFileEvent_t cursor_midi_event = this->getEventFromXY(this->cursor_x, this->cursor_y);

	if (cursor_midi_event == NULL)
	{
		this->window->selectNone();
	}
	else
	{
		this->window->selectNone();
		MidiFileEvent_setSelected(cursor_midi_event, 1);
	}
}

void Lane::toggleEventSelection()
{
	MidiFileEvent_t cursor_midi_event = this->getEventFromXY(this->cursor_x, this->cursor_y);

	if (cursor_midi_event == NULL)
	{
		this->window->selectNone();
	}
	else
	{
		MidiFileEvent_setSelected(cursor_midi_event, !MidiFileEvent_isSelected(cursor_midi_event));
	}
}

