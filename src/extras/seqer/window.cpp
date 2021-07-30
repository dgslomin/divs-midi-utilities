
#include <QtWidgets>
#include "all-events-lane.h"
#include "colors.h"
#include "controller-lane.h"
#include "inspector-sidebar.h"
#include "lane.h"
#include "marker-lane.h"
#include "menu.h"
#include "midifile.h"
#include "note-lane.h"
#include "splitter.h"
#include "tempo-lane.h"
#include "time-ruler.h"
#include "velocity-lane.h"
#include "window.h"

QVector<Window*> Window::windows;

bool Window::confirm_on_close = true;

Window::Window(Window* existing_window)
{
	Window::windows.append(this);
	this->sequence = (existing_window == NULL) ? new Sequence(MidiFile_newFromTemplate(NULL)) : existing_window->sequence;
	this->sequence->addWindow(this);
	this->statusBar();

	this->sidebar_splitter = new Splitter(Qt::Horizontal);
	this->setCentralWidget(sidebar_splitter);
	this->sidebar_splitter->setChildrenCollapsible(false);

	QWidget* lane_vbox = new QWidget();
	this->sidebar_splitter->addWidget(lane_vbox);
	QVBoxLayout* lane_vbox_layout = new QVBoxLayout(lane_vbox);
	lane_vbox_layout->setSpacing(0);
	lane_vbox_layout->setContentsMargins(0, 0, 0, 0);

	lane_vbox_layout->addWidget(new TimeRuler(this));

	this->lane_splitter = new Splitter(Qt::Vertical);
	lane_vbox_layout->addWidget(this->lane_splitter);
	this->lane_splitter->setChildrenCollapsible(false);

	this->lane_splitter->addWidget(new NoteLane(this));

	this->sidebar_tab_widget = new QTabWidget();
	this->sidebar_splitter->addWidget(this->sidebar_tab_widget);
	this->sidebar_tab_widget->setTabPosition(QTabWidget::North);
	this->sidebar_tab_widget->setDocumentMode(true);
	this->sidebar_tab_widget->setElideMode(Qt::ElideNone);
	this->sidebar_tab_widget->setUsesScrollButtons(false);

	this->inspector_sidebar = new InspectorSidebar(this);
	this->sidebar_tab_widget->addTab(inspector_sidebar, tr("Inspector"));

	this->sidebar_tab_widget->addTab(new QTextEdit("tracks placeholder"), tr("Tracks"));
	this->sidebar_tab_widget->addTab(new QTextEdit("channels placeholder"), tr("Channels"));

	QSettings settings;
	this->restoreGeometry(settings.value("window/window-geometry").toByteArray());
	this->restoreState(settings.value("window/window-state").toByteArray());
	this->lane_splitter->restoreState(settings.value("window/lane-splitter-state").toByteArray());
	this->sidebar_splitter->restoreState(settings.value("window/sidebar-splitter-state").toByteArray());
	this->sidebar_tab_widget->setCurrentIndex(settings.value("window/sidebar-tab-index", 0).toInt());
	this->use_linear_time = settings.value("window/use-linear-time", false).toBool();
	this->pixels_per_beat = settings.value("window/pixels-per-beat", 40.0).toFloat();
	this->pixels_per_second = settings.value("window/pixels-per-second", 40.0).toFloat();

	Menu::createMenuBar(this);
}

Window::~Window()
{
	this->sequence->removeWindow(this);
	Window::windows.removeOne(this);
}

void Window::closeEvent(QCloseEvent* event)
{
	if (Window::confirm_on_close && this->sequence->is_modified && (this->sequence->number_of_windows == 1) && !this->save(true, false, ""))
	{
		event->ignore();
		return;
	}

	QSettings settings;
	settings.setValue("window/window-geometry", this->saveGeometry());
	settings.setValue("window/window-state", this->saveState());
	settings.setValue("window/lane-splitter-state", this->lane_splitter->saveState());
	settings.setValue("window/sidebar-splitter-state", this->sidebar_splitter->saveState());
	settings.setValue("window/sidebar-tab-index", this->sidebar_tab_widget->currentIndex());
	settings.setValue("window/use-linear-time", this->use_linear_time);
}

bool Window::save(bool ask_first, bool save_as, QString filename)
{
	if (ask_first)
	{
		switch (QMessageBox::question(this, tr("Save Changes"), tr("Do you want to save changes to the current file?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
		{
			case QMessageBox::Yes:
			{
				break;
			}
			case QMessageBox::No:
			{
				return true;
			}
			default:
			{
				return false;
			}
		}
	}

	if (this->sequence->filename.isEmpty() || save_as)
	{
		if (filename.isEmpty()) filename = QFileDialog::getSaveFileName(this, tr("Save File"), QSettings().value("window/directory", "").toString(), tr("MIDI Files (*.mid)"));

		if (filename.isEmpty())
		{
			return false;
		}
		else
		{
			if (this->sequence->saveAs(filename))
			{
				QSettings().setValue("window/directory", QFileInfo(filename).absolutePath());
				return true;
			}
		}
	}
	else
	{
		if (this->sequence->save())
		{
			return true;
		}
	}

	QMessageBox::warning(this, tr("Error"), tr("Cannot save the specified MIDI file."));
	return false;
}

int Window::getXFromTick(long tick)
{
	if (this->use_linear_time)
	{
		return (int)(MidiFile_getTimeFromTick(this->sequence->midi_file, tick) * this->pixels_per_second - this->scroll_x);
	}
	else
	{
		return (int)(MidiFile_getBeatFromTick(this->sequence->midi_file, tick) * this->pixels_per_beat - this->scroll_x);
	}
}

long Window::getTickFromX(int x)
{
	if (this->use_linear_time)
	{
		return MidiFile_getTickFromTime(this->sequence->midi_file, (x + this->scroll_x) / this->pixels_per_second);
	}
	else
	{
		return MidiFile_getTickFromBeat(this->sequence->midi_file, (x + this->scroll_x) / this->pixels_per_beat);
	}
}

void Window::scrollXBy(float offset)
{
	float old_scroll_x = this->scroll_x;
	this->scroll_x = std::max(old_scroll_x - offset, 0.0f);
	this->cursor_x += (int)(old_scroll_x - this->scroll_x);
	this->update();
}

void Window::zoomXBy(float factor)
{
	long cursor_tick = this->getTickFromX(this->cursor_x);

	if (this->use_linear_time)
	{
		this->pixels_per_second *= factor;
	}
	else
	{
		this->pixels_per_beat *= factor;
	}

	this->cursor_x = this->getXFromTick(cursor_tick);
	this->update();
}

Lane* Window::getFocusedLane()
{
	for (int lane_number = 0; lane_number < this->lane_splitter->count(); lane_number++)
	{
		Lane* lane = qobject_cast<Lane*>(this->lane_splitter->widget(lane_number));
		if ((lane != NULL) && lane->hasFocus()) return lane;
	}

	return NULL;
}

void Window::scrollCursorIntoView()
{
	Lane* lane = this->getFocusedLane();
	if (lane == NULL) return;
	if (this->cursor_x < this->scroll_x) this->scroll_x = this->cursor_x;
	if (this->cursor_x > this->scroll_x + lane->width()) this->scroll_x = this->cursor_x - lane->width();
}

void Window::newSequence()
{
	if (this->sequence->is_modified && (this->sequence->number_of_windows == 1) && !this->save(true, false, "")) return;
	this->sequence->removeWindow(this);
	this->sequence = new Sequence(MidiFile_newFromTemplate(NULL));
	this->sequence->addWindow(this);
	this->sequence->update(false);
}

void Window::newWindow()
{
	(new Window(this))->show();
}

void Window::open()
{
	if (this->sequence->is_modified && (this->sequence->number_of_windows == 1) && !this->save(true, false, "")) return;
	QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QSettings().value("window/directory", "").toString(), tr("MIDI Files (*.mid)"));
	if (!filename.isEmpty()) this->open(filename);
}

void Window::open(QString filename)
{
	MidiFile_t new_midi_file = Sequence::loadMidiFile(filename);

	if (new_midi_file == NULL)
	{
		QMessageBox::warning(this, tr("Error"), tr("Cannot open the specified MIDI file."));
		return;
	}

	this->sequence->removeWindow(this);
	this->sequence = new Sequence(new_midi_file);
	this->sequence->addWindow(this);
	this->sequence->filename = filename;
	this->sequence->update(false);
	QSettings().setValue("window/directory", QFileInfo(filename).absolutePath());
}

void Window::save()
{
	this->save(false, false, "");
}

void Window::saveAs()
{
	this->save(false, true, "");
}

void Window::saveAs(QString filename)
{
	this->save(false, true, filename);
}

void Window::quit()
{
	bool other_sequences_are_modified = false;

	for (Window* window: Window::windows)
	{
		if ((window->sequence != this->sequence) && (window->sequence->is_modified))
		{
			other_sequences_are_modified = true;
			break;
		}
	}

	if (other_sequences_are_modified)
	{
		if (QMessageBox::question(this, tr("Quit"), tr("Really quit without saving changes?"), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) return;
	}
	else
	{
		if (this->sequence->is_modified && !this->save(true, false, "")) return;
	}

	Window::confirm_on_close = false;
	QGuiApplication::quit();
}

void Window::undo()
{
	this->sequence->undo_stack->undo();
}

void Window::redo()
{
	this->sequence->undo_stack->redo();
}

void Window::cut()
{
	Lane* lane = this->getFocusedLane();
	if (lane != NULL) lane->cut();
}

void Window::copy_()
{
	Lane* lane = this->getFocusedLane();
	if (lane != NULL) lane->copy_();
}

void Window::paste()
{
	Lane* lane = this->getFocusedLane();
	if (lane != NULL) lane->paste();
}

void Window::delete_()
{
	for (MidiFileEvent_t midi_event = MidiFile_iterateEvents(this->sequence->midi_file); midi_event != NULL; midi_event = MidiFile_iterateEvents(this->sequence->midi_file))
	{
		if (MidiFileEvent_isSelected(midi_event)) MidiFileEvent_delete(midi_event);
	}

	this->sequence->update(true);
}

void Window::selectAll()
{
	Sequence::midiFileSelectAll(this->sequence->midi_file);
	this->sequence->update(false);
}

void Window::selectNone()
{
	Sequence::midiFileSelectNone(this->sequence->midi_file);
	this->sequence->update(false);
}

void Window::zoomInTime()
{
	this->zoomXBy(1.05f);
}

void Window::zoomOutTime()
{
	this->zoomXBy(1 / 1.05f);
}

void Window::zoomInLane()
{
	Lane* lane = this->getFocusedLane();
	if (lane != NULL) lane->zoomIn();
}

void Window::zoomOutLane()
{
	Lane* lane = this->getFocusedLane();
	if (lane != NULL) lane->zoomOut();
}

void Window::addNoteLane()
{
	this->lane_splitter->addWidget(new NoteLane(this));
}

void Window::addVelocityLane()
{
	this->lane_splitter->addWidget(new VelocityLane(this));
}

void Window::addControllerLane()
{
	this->lane_splitter->addWidget(new ControllerLane(this));
}

void Window::addTempoLane()
{
	this->lane_splitter->addWidget(new TempoLane(this));
}

void Window::addMarkerLane()
{
	this->lane_splitter->addWidget(new MarkerLane(this));
}

void Window::addAllEventsLane()
{
	this->lane_splitter->addWidget(new AllEventsLane(this));
}

void Window::removeLane()
{
	Lane* lane = getFocusedLane();
	if (lane != NULL) delete lane;
}

void Window::moveLaneUp()
{
	Lane* lane = getFocusedLane();
	int index = this->lane_splitter->indexOf(lane);
	if (index > 0) this->lane_splitter->insertWidget(index - 1, lane);
}

void Window::moveLaneDown()
{
	Lane* lane = getFocusedLane();
	int index = this->lane_splitter->indexOf(lane);
	this->lane_splitter->insertWidget(index + 1, lane);
}

void Window::setUseLinearTime(bool use_linear_time)
{
	this->use_linear_time = use_linear_time;
	this->update();
}

void Window::nextMarker()
{
	long cursor_tick = this->getTickFromX(this->cursor_x);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEventForTick(this->sequence->midi_file, cursor_tick); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isMarkerEvent(midi_event))
		{
			long event_tick = MidiFileEvent_getTick(midi_event);

			if (event_tick != cursor_tick)
			{
				this->cursor_x = this->getXFromTick(event_tick);
				this->scrollCursorIntoView();
				this->update();
				break;
			}
		}
	}
}

void Window::previousMarker()
{
	long cursor_tick = this->getTickFromX(this->cursor_x);

	for (MidiFileEvent_t midi_event = MidiFile_getLastEventForTick(this->sequence->midi_file, cursor_tick); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if (MidiFileEvent_isMarkerEvent(midi_event))
		{
			long event_tick = MidiFileEvent_getTick(midi_event);

			if (event_tick != cursor_tick)
			{
				this->cursor_x = this->getXFromTick(event_tick);
				this->scrollCursorIntoView();
				this->update();
				break;
			}
		}
	}

	this->cursor_x = this->getXFromTick(0);
	this->scrollCursorIntoView();
	this->update();
}

void Window::goToMarker()
{
	QString marker = QInputDialog::getText(this, tr("Go to Marker"), tr("Marker name:"));
	if (marker.isNull()) return;
	long marker_tick = MidiFile_getTickFromMarker(this->sequence->midi_file, marker.toUtf8().data());

	if (marker_tick < 0)
	{
		QMessageBox::warning(this, tr("Error"), tr("Cannot find the specified marker."));
	}
	else
	{
		this->cursor_x = this->getXFromTick(marker_tick);
		this->scrollCursorIntoView();
		this->update();
	}
}

void Window::aboutSeqer()
{
	QMessageBox::information(this, tr("About"), tr("Seqer\na MIDI sequencer by Div Slomin\nprovided under terms of the BSD license"));
}

void Window::sequenceUpdated()
{
	this->setWindowModified(this->sequence->is_modified);
	this->setWindowTitle(QString("%1[*] - Seqer").arg(this->sequence->filename.isEmpty() ? tr("Untitled") : QFileInfo(this->sequence->filename).fileName()));

	for (int lane_number = 0; lane_number < this->lane_splitter->count(); lane_number++)
	{
		Lane* lane = qobject_cast<Lane*>(this->lane_splitter->widget(lane_number));
		if (lane != NULL) lane->sequenceUpdated();
	}
}

void Window::focusInspector()
{
	this->inspector_sidebar->setFocus(Qt::OtherFocusReason);
}

