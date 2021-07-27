
#include <algorithm>
#include <QtWidgets>
#include "label-lane.h"
#include "lane.h"
#include "midifile.h"
#include "window.h"

LabelLane::LabelLane(Window* window, QString type): Lane(window, type)
{
	QSettings settings;
	float font_scaling_factor = settings.value("label-lane/font-scaling-factor", 0.8).toFloat();

	QFont font = this->font();
	font.setPointSizeF(font.pointSizeF() * font_scaling_factor);
	this->setFont(font);

	this->row_height = this->fontMetrics().lineSpacing() + 4;
}

void LabelLane::paintBackground(QPainter* painter)
{
	painter->fillRect(0, 0, this->width(), this->height(), this->background_color);
}

void LabelLane::paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset)
{
	Q_UNUSED(selected_events_y_offset)

	if (this->sequence_updated)
	{
		this->populateLabels();
		this->computeLabelWidths();
	}

	this->layoutLabels(selected_events_x_offset);

	QFontMetrics font_metrics = this->fontMetrics();
	int baseline = font_metrics.leading() + font_metrics.ascent();
	MidiFileTrack_t current_track = MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 0);

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		bool is_background = (MidiFileEvent_getTrack(label.midi_event) != current_track);
		bool is_selected = MidiFileEvent_isSelected(label.midi_event);

		painter->setPen(is_background ? (is_selected ? this->selected_background_event_pen : this->unselected_background_event_pen) : (is_selected ? this->selected_event_pen : this->unselected_event_pen));
		painter->setBrush(is_background ? (is_selected ? this->selected_background_event_brush : this->unselected_background_event_brush) : (is_selected ? this->selected_event_brush : this->unselected_event_brush));
		painter->drawRect(label.x, label.row * this->row_height, label.width, this->row_height);

		painter->setPen(is_background ? (is_selected ? this->selected_background_event_text_pen : this->unselected_background_event_text_pen) : (is_selected ? this->selected_event_text_pen : this->unselected_event_text_pen));
		painter->setBrush(Qt::NoBrush);
		painter->drawText(label.x + 2, label.row * this->row_height + baseline + 2, label.text);
	}
}

MidiFileEvent_t LabelLane::getEventFromXY(int x, int y)
{
	for (int label_number = this->labels.size() - 1; label_number >= 0; label_number--)
	{
		Label& label = this->labels[label_number];
		QRect rect(label.x, label.row * this->row_height, label.width, this->row_height);
		if (rect.contains(x, y, false)) return label.midi_event;
	}

	return NULL;
}

QPoint LabelLane::getPointFromEvent(MidiFileEvent_t midi_event)
{
	for (int label_number = this->labels.size() - 1; label_number >= 0; label_number--)
	{
		Label& label = this->labels[label_number];
		if (label.midi_event == midi_event) return QPoint(label.x, label.row * this->row_height);
	}

	return QPoint();
}

void LabelLane::selectEventsInRect(int x, int y, int width, int height)
{
	QRect bounds(x, y, width, height);

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		QRect rect(label.x, label.row * this->row_height, label.width, this->row_height);
		if (rect.intersects(bounds)) MidiFileEvent_setSelected(label.midi_event, 1);
	}
}

void LabelLane::scrollYBy(int offset)
{
	// not supported
	Q_UNUSED(offset)
}

void LabelLane::zoomYBy(float factor)
{
	// not supported
	Q_UNUSED(factor)
}

int LabelLane::getCursorGap()
{
	return this->row_height;
}

void LabelLane::computeLabelWidths()
{
	QFontMetrics font_metrics = this->fontMetrics();

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		label.width = font_metrics.boundingRect(label.text).width() + 4;
	}
}

void LabelLane::layoutLabels(int selected_events_x_offset)
{
	int number_of_rows = this->height() / this->row_height;
	int cluster_start = 0;
	int end_x = INT_MIN;

	// We need to break this into two phases because selected_events_x_offset
	// can change the order of events.

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		label.preferred_x = this->window->getXFromTick(MidiFileEvent_getTick(label.midi_event)) + (MidiFileEvent_isSelected(label.midi_event) ? selected_events_x_offset : 0);
	}

	if (selected_events_x_offset != 0)
	{
		std::sort(this->labels.begin(), this->labels.end(), [](const Label& label1, const Label& label2) { return label1.preferred_x < label2.preferred_x; });
	}

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];

		if (label.preferred_x >= end_x)
		{
			label.x = label.preferred_x;
			label.row = 0;
			cluster_start = label_number;
			end_x = label.x + label.width;
		}
		else if (label_number - cluster_start < number_of_rows)
		{
			label.x = this->labels[cluster_start].x;
			label.row = this->labels[label_number - 1].row + 1;
			end_x = std::max(end_x, label.x + label.width);
		}
		else
		{
			label.x = end_x;
			label.row = 0;
			cluster_start = label_number;
			end_x = label.x + label.width;
		}
	}
}

Label::Label(MidiFileEvent_t midi_event, QString text)
{
	this->midi_event = midi_event;
	this->text = text;
}

