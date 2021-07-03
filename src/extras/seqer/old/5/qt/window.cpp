
#include <set>
#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QWindow>
#include "midifile.h"
#include "window.h"

Window::Window(Window* existing_window)
{
	this->sequence = (existing_window == NULL) ? new Sequence() : existing_window->sequence;
	this->sequence->addWindow(this);

	this->setWindowTitle("Seqer");
	this->createMenuBar();
	this->statusBar();

	this->sidebar_splitter = new QSplitter(Qt::Horizontal);
	this->setCentralWidget(sidebar_splitter);
	this->sidebar_splitter->setChildrenCollapsible(false);
	this->sidebar_splitter->setHandleWidth(0);

	this->lane_splitter = new QSplitter(Qt::Vertical);
	this->sidebar_splitter->addWidget(this->lane_splitter);
	this->lane_splitter->setChildrenCollapsible(false);
	this->lane_splitter->setHandleWidth(0);

	this->lane_splitter->addWidget(new QTextEdit("lane placeholder 1"));
	this->lane_splitter->addWidget(new QTextEdit("lane placeholder 2"));
	this->lane_splitter->addWidget(new QTextEdit("lane placeholder 3"));

	this->sidebar_tab_widget = new QTabWidget();
	this->sidebar_splitter->addWidget(this->sidebar_tab_widget);
	this->sidebar_tab_widget->setTabPosition(QTabWidget::North);
	this->sidebar_tab_widget->setDocumentMode(true);
	this->sidebar_tab_widget->setElideMode(Qt::ElideNone);
	this->sidebar_tab_widget->setUsesScrollButtons(false);

	this->sidebar_tab_widget->addTab(new QTextEdit("inspector placeholder"), tr("Inspector"));
	this->sidebar_tab_widget->addTab(new QTextEdit("tracks placeholder"), tr("Tracks"));
	this->sidebar_tab_widget->addTab(new QTextEdit("channels placeholder"), tr("Channels"));

	QSettings settings;
	this->restoreGeometry(settings.value("state/window-geometry").toByteArray());
	this->restoreState(settings.value("state/window-state").toByteArray());
	this->lane_splitter->restoreState(settings.value("state/lane-splitter-state").toByteArray());
	this->sidebar_splitter->restoreState(settings.value("state/sidebar-splitter-state").toByteArray());
	this->sidebar_tab_widget->setCurrentIndex(settings.value("state/sidebar-tab-index", 0).toInt());
	this->use_linear_time = settings.value("view/use-linear-time", false).toBool();
	this->pixels_per_beat = settings.value("view/pixels-per-beat", 40).toInt();
	this->pixels_per_second = settings.value("view/pixels-per-second", 40).toInt();
}

Window::~Window()
{
	this->sequence->removeWindow(this);
}

void Window::closeEvent(QCloseEvent* event)
{
	QSettings settings;
	settings.setValue("state/window-geometry", this->saveGeometry());
	settings.setValue("state/window-state", this->saveState());
	settings.setValue("state/lane-splitter-state", this->lane_splitter->saveState());
	settings.setValue("state/sidebar-splitter-state", this->sidebar_splitter->saveState());
	settings.setValue("state/sidebar-tab-index", this->sidebar_tab_widget->currentIndex());
	QMainWindow::closeEvent(event);
}

bool Window::isModified()
{
	return this->sequence->is_modified;
}

bool Window::isLastWindowForSequence()
{
	return (this->sequence->windows.size() == 1);
}

QString Window::getFilename()
{
	return this->sequence->filename;
}

void Window::new_()
{
	if (!this->saveChangesIfNeeded()) return;
	this->sequence->removeWindow(this);
	this->sequence = new Sequence();
	this->sequence->addWindow(this);
	this->refreshData();
}

void Window::newWindow()
{
	(new Window(this))->show();
}

void Window::open()
{
	if (!this->saveChangesIfNeeded()) return;

	QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("MIDI Files (*.mid)"));

	if (!filename.isEmpty())
	{
		if (!this->load(filename))
		{
			QMessageBox::warning(this, tr("Error"), tr("Cannot open the specified MIDI file."));
		}
	}
}

bool Window::load(QString filename)
{
	MidiFile_t new_midi_file = MidiFile_load(filename.toUtf8().data());
	if (new_midi_file == NULL) return false;
	this->sequence->removeWindow(this);
	this->sequence = new Sequence();
	this->sequence->addWindow(this);
	this->sequence->filename = filename;
	MidiFile_free(this->sequence->midi_file);
	this->sequence->midi_file = new_midi_file;
	this->refreshData();
	return true;
}

bool Window::saveChangesIfNeeded()
{
	if (!(this->isModified() && this->isLastWindowForSequence())) return true;
	return this->saveChanges();
}

bool Window::saveChanges()
{
	switch (QMessageBox::question(this, tr("Save Changes"), tr("Do you want to save changes to the current file?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
	{
		case QMessageBox::Yes:
		{
			return this->save();
		}
		case QMessageBox::Cancel:
		{
			return false;
		}
		default:
		{
			return true;
		}
	}
}

bool Window::save()
{
	if (this->getFilename().isEmpty())
	{
		return this->saveAs();
	}
	else
	{
		if (this->sequence->save())
		{
			return true;
		}
		else
		{
			QMessageBox::warning(this, tr("Error"), tr("Cannot save the specified MIDI file."));
			return false;
		}
	}
}

bool Window::saveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("MIDI Files (*.mid)"));

	if (filename.isEmpty())
	{
		return false;
	}
	else
	{
		if (this->sequence->saveAs(filename))
		{
			return true;
		}
		else
		{
			QMessageBox::warning(this, tr("Error"), tr("Cannot save the specified MIDI file."));
			return false;
		}
	}
}

void Window::quit()
{
	std::set<Sequence*> sequences;
	bool is_modified = false;

	for (QWindow* top_level_window: QGuiApplication::topLevelWindows())
	{
		Window* window = qobject_cast<Window*>(top_level_window);

		if (window != NULL)
		{
			sequences.insert(window->sequence);
			if (window->isModified()) is_modified = true;
		}
	}

	if (sequences.size() == 1)
	{
		if (!is_modified || this->saveChanges()) QGuiApplication::quit();
	}
	else
	{
		if (!is_modified || QMessageBox::question(this, tr("Quit"), tr("Really quit without saving changes?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) QGuiApplication::quit();
	}
}

void Window::selectAll()
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		MidiFileEvent_setSelected(midi_event, 1);
	}
}

void Window::selectNone()
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		MidiFileEvent_setSelected(midi_event, 0);
	}
}

void Window::addLane()
{
}

void Window::removeLane()
{
}

void Window::focusInspector()
{
}

void Window::refreshData()
{
}

void Window::refreshDisplay()
{
}

int Window::getXFromTick(long tick)
{
	if (this->use_linear_time)
	{
		return (int)(MidiFile_getTimeFromTick(this->sequence->midi_file, tick) * this->pixels_per_second) - this->scroll_x;
	}
	else
	{
		return (int)(MidiFile_getBeatFromTick(this->sequence->midi_file, tick) * this->pixels_per_beat) - this->scroll_x;
	}
}

long Window::getTickFromX(int x)
{
	if (this->use_linear_time)
	{
		return MidiFile_getTickFromTime(this->sequence->midi_file, (float)(x + this->scroll_x) / this->pixels_per_second);
	}
	else
	{
		return MidiFile_getTickFromBeat(this->sequence->midi_file, (float)(x + this->scroll_x) / this->pixels_per_beat);
	}
}

void Window::createMenuBar()
{
	this->createFileMenu();
	this->createEditMenu();
	this->createViewMenu();
	this->createTransportMenu();
	this->createToolsMenu();
	this->createHelpMenu();
}

void Window::createFileMenu()
{
	QMenu* file_menu = this->menuBar()->addMenu(tr("&File"));

	QAction* new_action = new QAction(tr("&New"));
	this->addAction(new_action);
	file_menu->addAction(new_action);
	new_action->setShortcut(QKeySequence(QKeySequence::New));

	QAction* new_window_action = new QAction(tr("New &Window"));
	this->addAction(new_window_action);
	new_window_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
	file_menu->addAction(new_window_action);

	QAction* open_action = new QAction(tr("&Open..."));
	this->addAction(open_action);
	file_menu->addAction(open_action);
	open_action->setShortcut(QKeySequence(QKeySequence::Open));

	QAction* save_action = new QAction(tr("&Save"));
	this->addAction(save_action);
	file_menu->addAction(save_action);
	save_action->setShortcut(QKeySequence(QKeySequence::Save));

	QAction* save_as_action = new QAction(tr("Save &As..."));
	this->addAction(save_as_action);
	save_as_action->setShortcut(QKeySequence(QKeySequence::SaveAs));
	file_menu->addAction(save_as_action);

	file_menu->addSeparator();

	QAction* close_action = new QAction(tr("&Close"));
	this->addAction(close_action);
	file_menu->addAction(close_action);
	close_action->setShortcut(QKeySequence(QKeySequence::Close));
	connect(close_action, SIGNAL(triggered()), this, SLOT(close()));

	QAction* quit_action = new QAction(tr("&Quit"));
	this->addAction(quit_action);
	file_menu->addAction(quit_action);
	quit_action->setShortcut(QKeySequence(QKeySequence::Quit));
}

void Window::createEditMenu()
{
	QMenu* edit_menu = this->menuBar()->addMenu(tr("&Edit"));

	QAction* undo_action = new QAction(tr("&Undo"));
	this->addAction(undo_action);
	edit_menu->addAction(undo_action);
	undo_action->setShortcut(QKeySequence(QKeySequence::Undo));

	QAction* redo_action = new QAction(tr("&Redo"));
	this->addAction(redo_action);
	edit_menu->addAction(redo_action);
	redo_action->setShortcut(QKeySequence(QKeySequence::Redo));

	edit_menu->addSeparator();

	QAction* cut_action = new QAction(tr("Cu&t"));
	this->addAction(cut_action);
	edit_menu->addAction(cut_action);
	cut_action->setShortcut(QKeySequence(QKeySequence::Cut));

	QAction* copy_action = new QAction(tr("&Copy"));
	this->addAction(copy_action);
	edit_menu->addAction(copy_action);
	copy_action->setShortcut(QKeySequence(QKeySequence::Copy));

	QAction* paste_action = new QAction(tr("&Paste"));
	this->addAction(paste_action);
	edit_menu->addAction(paste_action);
	paste_action->setShortcut(QKeySequence(QKeySequence::Paste));

	QAction* delete_action = new QAction(tr("&Delete"));
	this->addAction(delete_action);
	edit_menu->addAction(delete_action);
	delete_action->setShortcut(QKeySequence(QKeySequence::Delete));

	edit_menu->addSeparator();

	QAction* select_all_action = new QAction(tr("Select &All"));
	this->addAction(select_all_action);
	edit_menu->addAction(select_all_action);
	select_all_action->setShortcut(QKeySequence(QKeySequence::SelectAll));

	QAction* select_none_action = new QAction(tr("Select &None"));
	this->addAction(select_none_action);
	edit_menu->addAction(select_none_action);
	select_none_action->setShortcut(QKeySequence(QKeySequence::Deselect));

	QAction* select_time_range_action = new QAction(tr("Select Ti&me Range"));
	this->addAction(select_time_range_action);
	edit_menu->addAction(select_time_range_action);
}

void Window::createViewMenu()
{
	QMenu* view_menu = this->menuBar()->addMenu(tr("&View"));

	QAction* next_lane_action = new QAction(tr("&Next Lane"));
	this->addAction(next_lane_action);
	view_menu->addAction(next_lane_action);
	next_lane_action->setShortcut(QKeySequence(QKeySequence::NextChild));

	QAction* previous_lane_action = new QAction(tr("Pre&vious Lane"));
	this->addAction(previous_lane_action);
	view_menu->addAction(previous_lane_action);
	previous_lane_action->setShortcut(QKeySequence(QKeySequence::PreviousChild));

	view_menu->addSeparator();

	QAction* add_lane_action = new QAction(tr("&Add Lane"));
	this->addAction(add_lane_action);
	view_menu->addAction(add_lane_action);

	QAction* remove_lane_action = new QAction(tr("&Remove Lane"));
	this->addAction(remove_lane_action);
	view_menu->addAction(remove_lane_action);

	QAction* move_lane_up_action = new QAction(tr("Move Lane &Up"));
	this->addAction(move_lane_up_action);
	view_menu->addAction(move_lane_up_action);

	QAction* move_lane_down_action = new QAction(tr("Move Lane &Down"));
	this->addAction(move_lane_down_action);
	view_menu->addAction(move_lane_down_action);

	view_menu->addSeparator();

	QAction* zoom_in_time_action = new QAction(tr("Zoom &In Time"));
	this->addAction(zoom_in_time_action);
	view_menu->addAction(zoom_in_time_action);
	zoom_in_time_action->setShortcut(QKeySequence(QKeySequence::ZoomIn));

	QAction* zoom_out_time_action = new QAction(tr("Zoom &Out Time"));
	this->addAction(zoom_out_time_action);
	view_menu->addAction(zoom_out_time_action);
	zoom_out_time_action->setShortcut(QKeySequence(QKeySequence::ZoomOut));

	QAction* zoom_in_lane_action = new QAction(tr("Zoom &In Lane"));
	this->addAction(zoom_in_lane_action);
	view_menu->addAction(zoom_in_lane_action);

	QAction* zoom_out_lane_action = new QAction(tr("Zoom &Out Lane"));
	this->addAction(zoom_out_lane_action);
	view_menu->addAction(zoom_out_lane_action);
}

void Window::createTransportMenu()
{
	QMenu* transport_menu = this->menuBar()->addMenu(tr("Trans&port"));

	QAction* play_action = new QAction(tr("&Play"));
	this->addAction(play_action);
	transport_menu->addAction(play_action);
	play_action->setShortcut(QKeySequence(Qt::Key_Space));

	QAction* stop_action = new QAction(tr("&Stop"));
	this->addAction(stop_action);
	transport_menu->addAction(stop_action);
	stop_action->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Space));

	QAction* record_action = new QAction(tr("&Record"));
	this->addAction(record_action);
	transport_menu->addAction(record_action);
	record_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));

	transport_menu->addSeparator();

	QAction* set_playback_position_action = new QAction(tr("Set Playback Position"));
	this->addAction(set_playback_position_action);
	transport_menu->addAction(set_playback_position_action);

	QAction* go_to_playback_position_action = new QAction(tr("Go to Playback Position"));
	this->addAction(go_to_playback_position_action);
	transport_menu->addAction(go_to_playback_position_action);

	transport_menu->addSeparator();

	QAction* next_marker_action = new QAction(tr("&Next Marker"));
	this->addAction(next_marker_action);
	transport_menu->addAction(next_marker_action);
	next_marker_action->setShortcut(QKeySequence(QKeySequence::MoveToNextWord));

	QAction* previous_marker_action = new QAction(tr("Pre&vious Marker"));
	this->addAction(previous_marker_action);
	transport_menu->addAction(previous_marker_action);
	previous_marker_action->setShortcut(QKeySequence(QKeySequence::MoveToPreviousWord));

	QAction* go_to_marker_action = new QAction(tr("Go to &Marker..."));
	this->addAction(go_to_marker_action);
	transport_menu->addAction(go_to_marker_action);
	go_to_marker_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
}

void Window::createToolsMenu()
{
	QMenu* tools_menu = this->menuBar()->addMenu(tr("&Tools"));

	QAction* external_utility_action = new QAction(tr("External &Utility..."));
	this->addAction(external_utility_action);
	tools_menu->addAction(external_utility_action);
	external_utility_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));

	tools_menu->addSeparator();

	QAction* record_macro_action = new QAction(tr("&Record Macro..."));
	this->addAction(record_macro_action);
	tools_menu->addAction(record_macro_action);
	record_macro_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));

	QAction* macros_action = new QAction(tr("&Macros..."));
	this->addAction(macros_action);
	tools_menu->addAction(macros_action);

	tools_menu->addSeparator();

	QAction* preferences_action = new QAction(tr("&Preferences..."));
	this->addAction(preferences_action);
	tools_menu->addAction(preferences_action);
	preferences_action->setShortcut(QKeySequence(QKeySequence::Preferences));
}

void Window::createHelpMenu()
{
	QMenu* help_menu = this->menuBar()->addMenu(tr("&Help"));

	QAction* user_manual_action = new QAction(tr("&User Manual"));
	this->addAction(user_manual_action);
	help_menu->addAction(user_manual_action);

	QAction* about_seqer_action = new QAction(tr("&About Seqer"));
	this->addAction(about_seqer_action);
	help_menu->addAction(about_seqer_action);
}

