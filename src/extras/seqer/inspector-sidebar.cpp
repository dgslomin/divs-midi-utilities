
#include <QtWidgets>
#include "colors.h"
#include "inspector-sidebar.h"
#include "lane.h"
#include "midifile.h"
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

	this->lane_type_line_edit = new QLineEdit();
	lane_layout->addRow(tr("Lane Type"), this->lane_type_line_edit);

	this->lane_controller_line_edit = new QLineEdit();
	lane_layout->addRow(tr("Controller"), this->lane_controller_line_edit);

	outer_layout->addWidget(new Separator(Qt::Horizontal));

	QWidget* event_panel = new QWidget();
	outer_layout->addWidget(event_panel, 1);
	QFormLayout* event_layout = new QFormLayout(event_panel);
	event_layout->setSpacing(4);
	event_layout->setContentsMargins(8, 8, 8, 8);

	this->event_type_line_edit = new QLineEdit();
	event_layout->addRow(tr("Event Type"), this->event_type_line_edit);

	this->event_time_line_edit = new QLineEdit();
	event_layout->addRow(tr("Time"), this->event_time_line_edit);
}

void InspectorSidebar::paintEvent(QPaintEvent* event)
{
	Lane* lane = this->window->getFocusedLane();
	if (lane != NULL) this->lane_type_line_edit->setText(lane->type);
	QWidget::paintEvent(event);
}

void InspectorSidebar::sequenceUpdated()
{
	MidiFileEvent_t first_selected_event = NULL;
	bool multiple_event_types_are_selected = false;

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isSelected(midi_event))
		{
			if (first_selected_event == NULL)
			{
				first_selected_event = midi_event;
			}
			else
			{
				if (MidiFileEvent_getType(midi_event) != MidiFileEvent_getType(first_selected_event)) multiple_event_types_are_selected = true;
			}
		}
	}

	if (first_selected_event == NULL)
	{
		this->event_type_line_edit->setText("");
		this->event_time_line_edit->setText("");
	}
	else
	{
		if (multiple_event_types_are_selected)
		{
			this->event_type_line_edit->setText(tr("Multiple"));
		}
		else
		{
			switch (MidiFileEvent_getType(first_selected_event))
			{
				case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
				{
					this->event_type_line_edit->setText(tr("Note Off"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_NOTE_ON:
				{
					this->event_type_line_edit->setText(tr("Note On"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
				{
					this->event_type_line_edit->setText(tr("Key Pressure"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
				{
					this->event_type_line_edit->setText(tr("Control Change"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
				{
					this->event_type_line_edit->setText(tr("Program Change"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
				{
					this->event_type_line_edit->setText(tr("Channel Pressure"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
				{
					this->event_type_line_edit->setText(tr("Pitch Wheel"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_SYSEX:
				{
					this->event_type_line_edit->setText(tr("System Exclusive"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_META:
				{
					this->event_type_line_edit->setText(tr("Meta"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_NOTE:
				{
					this->event_type_line_edit->setText(tr("Note"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_FINE_CONTROL_CHANGE:
				{
					this->event_type_line_edit->setText(tr("Fine Control Change"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_RPN:
				{
					this->event_type_line_edit->setText(tr("RPN"));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_NRPN:
				{
					this->event_type_line_edit->setText(tr("NRPN"));
					break;
				}
				default:
				{
					break;
				}
			}
		}

		this->event_time_line_edit->setText(MidiFile_getMeasureBeatStringFromTick(this->window->sequence->midi_file, MidiFileEvent_getTick(first_selected_event)));
	}
}

