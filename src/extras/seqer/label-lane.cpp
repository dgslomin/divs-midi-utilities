
#include <QPainter>
#include "label-lane.h"
#include "lane.h"
#include "midifile.h"
#include "window.h"

LabelLane::LabelLane(Window* window): Lane(window)
{
}

void LabelLane::paintBackground(QPainter* painter)
{
	painter->fillRect(0, 0, this->width(), this->height(), this->background_color);
}

void LabelLane::paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset)
{
	this->populateLabels();
	this->layoutLabels(selected_events_x_offset);

	int row_height = this->fontMetrics().lineSpacing() + 4;

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		bool is_selected = MidiFileEvent_isSelected(label.midi_event);

		painter->setPen(is_selected ? this->selected_event_pen : this->unselected_event_pen);
		painter->setBrush(is_selected ? this->selected_event_brush : this->unselected_event_brush);
		painter->drawRect(label.x, label.row * row_height, label.width, row_height);

		painter->setPen(is_selected ? this->selected_event_text_pen : this->unselected_event_text_pen);
		painter->setBrush(Qt::NoBrush);
		painter->drawText(label.text, label.x + 2, (label.row * row_height) + 2);
	}
}

MidiFileEvent_t LabelLane::getEventFromXY(int x, int y)
{
	int row_height = this->fontMetrics().lineSpacing() + 4;

	for (int label_number = this->labels.size() - 1; label_number >= 0; label_number--)
	{
		Label& label = this->labels[label_number];
		QRect rect(label.x, label.row * row_height, label.width, row_height);
		if (rect.contains(x, y, false)) return label.midi_event;
	}

	return NULL;
}

QPoint LabelLane::getPointFromEvent(MidiFileEvent_t midi_event)
{
	int row_height = this->fontMetrics().lineSpacing() + 4;

	for (int label_number = this->labels.size() - 1; label_number >= 0; label_number--)
	{
		if (label.midi_event == midi_event) return QPoint(label.x, label.row * row_height);
	}

	return NULL;
}

MidiFileEvent_t LabelLane::addEventAtXY(int x, int y)
{
	return this->addEventAtTick(this->window->getTickFromX(x));
}

void LabelLane::moveEventsByXY(int x_offset, int y_offset)
{
	// not supported
}

void LabelLane::selectEventsInRect(int x, int y, int width, int height)
{
	QRect bounds(x, y, width, height);
	int row_height = this->fontMetrics().lineSpacing() + 4;

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		QRect rect(label.x, label.row * row_height, label.width, row_height);
		if (rect.intersects(bounds)) MidiFileEvent_setSelected(label.midi_event, 1);
	}
}

void LabelLane::scrollYBy(int y_offset)
{
	// not supported
}

void LabelLane::zoomYBy(int y_offset)
{
	// not supported
}

void LabelLane::layoutLabels(int selected_events_x_offset)
{
	QFontMetrics font_metrics = this->fontMetrics();
	int row_height = font_metrics.lineSpacing() + 4;
	int number_of_rows = this->height() / row_height;
	int cluster_start = 0;
	int end_x = 0;

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		int preferred_x = this->window->getXFromTick(MidiFileEvent_getTick(label.midi_event));
		int width = font_metrics.boundingRect(label.text).width() + 4;

		if (preferred_x >= end_x)
		{
			this->labels[label_number].x = preferred_x;
			this->labels[label_number].row = 0;
			cluster_start = label_number;
			end_x = this->labels[label_number].x + width;
		}
		else if (label_number - cluster_start < number_of_rows)
		{
			this->labels[label_number].x = this->labels[cluster_start].x;
			this->labels[label_number].row = this->labels[label_number - 1].row + 1;
			end_x = std::max(end_x, this->labels[label_number].x + width);
		}
		else
		{
			this->labels[label_number].x = end_x;
			this->labels[label_number].row = 0;
			cluster_start = label_number;
			end_x = this->labels[label_number].x + width;
		}
	}
}

