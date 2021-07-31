
#include <QtWidgets>
#include "menu.h"
#include "window.h"

void Menu::createMenuBar(Window* window)
{
	createFileMenu(window);
	createEditMenu(window);
	createViewMenu(window);
	createTransportMenu(window);
	createToolsMenu(window);
	createHelpMenu(window);
}

void Menu::createFileMenu(Window* window)
{
	QMenu* file_menu = window->menuBar()->addMenu(tr("&File"));

	QAction* new_sequence_action = new QAction(tr("&New"));
	window->addAction(new_sequence_action);
	file_menu->addAction(new_sequence_action);
	new_sequence_action->setShortcut(QKeySequence(QKeySequence::New));
	connect(new_sequence_action, SIGNAL(triggered()), window, SLOT(newSequence()));

	QAction* new_window_action = new QAction(tr("New &Window"));
	window->addAction(new_window_action);
	new_window_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
	file_menu->addAction(new_window_action);
	connect(new_window_action, SIGNAL(triggered()), window, SLOT(newWindow()));

	QAction* open_action = new QAction(tr("&Open..."));
	window->addAction(open_action);
	file_menu->addAction(open_action);
	open_action->setShortcut(QKeySequence(QKeySequence::Open));
	connect(open_action, SIGNAL(triggered()), window, SLOT(open()));

	QAction* save_action = new QAction(tr("&Save"));
	window->addAction(save_action);
	file_menu->addAction(save_action);
	save_action->setShortcut(QKeySequence(QKeySequence::Save));
	connect(save_action, SIGNAL(triggered()), window, SLOT(save()));

	QAction* save_as_action = new QAction(tr("Save &As..."));
	window->addAction(save_as_action);
	save_as_action->setShortcut(QKeySequence(QKeySequence::SaveAs));
	file_menu->addAction(save_as_action);
	connect(save_as_action, SIGNAL(triggered()), window, SLOT(saveAs()));

	file_menu->addSeparator();

	QAction* close_action = new QAction(tr("&Close"));
	window->addAction(close_action);
	file_menu->addAction(close_action);
	close_action->setShortcut(QKeySequence(QKeySequence::Close));
	connect(close_action, SIGNAL(triggered()), window, SLOT(close()));

	QAction* quit_action = new QAction(tr("&Quit"));
	window->addAction(quit_action);
	file_menu->addAction(quit_action);
	quit_action->setShortcut(QKeySequence(QKeySequence::Quit));
	connect(quit_action, SIGNAL(triggered()), window, SLOT(quit()));
}

void Menu::createEditMenu(Window* window)
{
	QMenu* edit_menu = window->menuBar()->addMenu(tr("&Edit"));

	QAction* undo_action = new QAction(tr("&Undo"));
	window->addAction(undo_action);
	edit_menu->addAction(undo_action);
	undo_action->setShortcut(QKeySequence(QKeySequence::Undo));
	connect(undo_action, SIGNAL(triggered()), window, SLOT(undo()));

	QAction* redo_action = new QAction(tr("&Redo"));
	window->addAction(redo_action);
	edit_menu->addAction(redo_action);
	redo_action->setShortcut(QKeySequence(QKeySequence::Redo));
	connect(redo_action, SIGNAL(triggered()), window, SLOT(redo()));

	edit_menu->addSeparator();

	QAction* cut_action = new QAction(tr("Cu&t"));
	window->addAction(cut_action);
	edit_menu->addAction(cut_action);
	cut_action->setShortcut(QKeySequence(QKeySequence::Cut));
	connect(cut_action, SIGNAL(triggered()), window, SLOT(cut()));

	QAction* copy_action = new QAction(tr("&Copy"));
	window->addAction(copy_action);
	edit_menu->addAction(copy_action);
	copy_action->setShortcut(QKeySequence(QKeySequence::Copy));
	connect(copy_action, SIGNAL(triggered()), window, SLOT(copy_()));

	QAction* paste_action = new QAction(tr("&Paste"));
	window->addAction(paste_action);
	edit_menu->addAction(paste_action);
	paste_action->setShortcut(QKeySequence(QKeySequence::Paste));
	connect(paste_action, SIGNAL(triggered()), window, SLOT(paste()));

	QAction* delete_action = new QAction(tr("&Delete"));
	window->addAction(delete_action);
	edit_menu->addAction(delete_action);
	delete_action->setShortcut(QKeySequence(QKeySequence::Delete));
	connect(delete_action, SIGNAL(triggered()), window, SLOT(delete_()));

	edit_menu->addSeparator();

	QAction* select_all_action = new QAction(tr("Select &All"));
	window->addAction(select_all_action);
	edit_menu->addAction(select_all_action);
	select_all_action->setShortcut(QKeySequence(QKeySequence::SelectAll));
	connect(select_all_action, SIGNAL(triggered()), window, SLOT(selectAll()));

	QAction* select_none_action = new QAction(tr("Select &None"));
	window->addAction(select_none_action);
	edit_menu->addAction(select_none_action);
	select_none_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
	connect(select_none_action, SIGNAL(triggered()), window, SLOT(selectNone()));

	QAction* select_time_range_action = new QAction(tr("Select Ti&me Range"));
	window->addAction(select_time_range_action);
	edit_menu->addAction(select_time_range_action);
}

void Menu::createViewMenu(Window* window)
{
	QMenu* view_menu = window->menuBar()->addMenu(tr("&View"));

	QAction* zoom_in_time_action = new QAction(tr("Zoom &In Time"));
	window->addAction(zoom_in_time_action);
	view_menu->addAction(zoom_in_time_action);
	zoom_in_time_action->setShortcut(QKeySequence(QKeySequence::ZoomIn));
	connect(zoom_in_time_action, SIGNAL(triggered()), window, SLOT(zoomInTime()));

	QAction* zoom_out_time_action = new QAction(tr("Zoom &Out Time"));
	window->addAction(zoom_out_time_action);
	view_menu->addAction(zoom_out_time_action);
	zoom_out_time_action->setShortcut(QKeySequence(QKeySequence::ZoomOut));
	connect(zoom_out_time_action, SIGNAL(triggered()), window, SLOT(zoomOutTime()));

	QAction* zoom_in_lane_action = new QAction(tr("Zoom &In Lane"));
	window->addAction(zoom_in_lane_action);
	view_menu->addAction(zoom_in_lane_action);
	zoom_in_lane_action->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Plus));
	connect(zoom_in_lane_action, SIGNAL(triggered()), window, SLOT(zoomInLane()));

	QAction* zoom_out_lane_action = new QAction(tr("Zoom &Out Lane"));
	window->addAction(zoom_out_lane_action);
	view_menu->addAction(zoom_out_lane_action);
	zoom_out_lane_action->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Minus));
	connect(zoom_out_lane_action, SIGNAL(triggered()), window, SLOT(zoomOutLane()));

	view_menu->addSeparator();

	QMenu* add_lane_submenu = view_menu->addMenu(tr("&Add Lane"));

	QAction* add_note_lane_action = new QAction(tr("&Note Lane"));
	window->addAction(add_note_lane_action);
	add_lane_submenu->addAction(add_note_lane_action);
	connect(add_note_lane_action, SIGNAL(triggered()), window, SLOT(addNoteLane()));

	QAction* add_velocity_lane_action = new QAction(tr("&Velocity Lane"));
	window->addAction(add_velocity_lane_action);
	add_lane_submenu->addAction(add_velocity_lane_action);
	connect(add_velocity_lane_action, SIGNAL(triggered()), window, SLOT(addVelocityLane()));

	QAction* add_controller_lane_action = new QAction(tr("&Controller Lane"));
	window->addAction(add_controller_lane_action);
	add_lane_submenu->addAction(add_controller_lane_action);
	connect(add_controller_lane_action, SIGNAL(triggered()), window, SLOT(addControllerLane()));

	QAction* add_tempo_lane_action = new QAction(tr("&Tempo Lane"));
	window->addAction(add_tempo_lane_action);
	add_lane_submenu->addAction(add_tempo_lane_action);
	connect(add_tempo_lane_action, SIGNAL(triggered()), window, SLOT(addTempoLane()));

	QAction* add_marker_lane_action = new QAction(tr("&Marker Lane"));
	window->addAction(add_marker_lane_action);
	add_lane_submenu->addAction(add_marker_lane_action);
	connect(add_marker_lane_action, SIGNAL(triggered()), window, SLOT(addMarkerLane()));

	QAction* add_all_events_lane_action = new QAction(tr("&All Events Lane"));
	window->addAction(add_all_events_lane_action);
	add_lane_submenu->addAction(add_all_events_lane_action);
	connect(add_all_events_lane_action, SIGNAL(triggered()), window, SLOT(addAllEventsLane()));

	QAction* remove_lane_action = new QAction(tr("&Remove Lane"));
	window->addAction(remove_lane_action);
	view_menu->addAction(remove_lane_action);
	connect(remove_lane_action, SIGNAL(triggered()), window, SLOT(removeLane()));

	QAction* move_lane_up_action = new QAction(tr("Move Lane &Up"));
	window->addAction(move_lane_up_action);
	view_menu->addAction(move_lane_up_action);
	connect(move_lane_up_action, SIGNAL(triggered()), window, SLOT(moveLaneUp()));

	QAction* move_lane_down_action = new QAction(tr("Move Lane &Down"));
	window->addAction(move_lane_down_action);
	view_menu->addAction(move_lane_down_action);
	connect(move_lane_down_action, SIGNAL(triggered()), window, SLOT(moveLaneDown()));

	view_menu->addSeparator();

	QAction* linear_time_action = new QAction(tr("&Linear Time"));
	window->addAction(linear_time_action);
	view_menu->addAction(linear_time_action);
	linear_time_action->setCheckable(true);
	linear_time_action->setChecked(window->use_linear_time);
	connect(linear_time_action, SIGNAL(toggled(bool)), window, SLOT(setUseLinearTime(bool)));
}

void Menu::createTransportMenu(Window* window)
{
	QMenu* transport_menu = window->menuBar()->addMenu(tr("Trans&port"));

	QAction* play_action = new QAction(tr("&Play"));
	window->addAction(play_action);
	transport_menu->addAction(play_action);
	play_action->setShortcut(QKeySequence(Qt::Key_Space));

	QAction* record_action = new QAction(tr("&Record"));
	window->addAction(record_action);
	transport_menu->addAction(record_action);
	record_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));

	transport_menu->addSeparator();

	QAction* set_play_position_action = new QAction(tr("Set Play Position"));
	window->addAction(set_play_position_action);
	transport_menu->addAction(set_play_position_action);
	set_play_position_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backslash));

	QAction* go_to_play_position_action = new QAction(tr("Go to Play Position"));
	window->addAction(go_to_play_position_action);
	transport_menu->addAction(go_to_play_position_action);
	go_to_play_position_action->setShortcut(QKeySequence(Qt::Key_Backslash));

	QAction* go_to_stop_position_action = new QAction(tr("Go to Stop Position"));
	window->addAction(go_to_stop_position_action);
	transport_menu->addAction(go_to_stop_position_action);
	go_to_stop_position_action->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Backslash));

	transport_menu->addSeparator();

	QAction* next_marker_action = new QAction(tr("&Next Marker"));
	window->addAction(next_marker_action);
	transport_menu->addAction(next_marker_action);
#ifdef Q_OS_MACOS
	next_marker_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right));
#else
	next_marker_action->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Right));
#endif
	connect(next_marker_action, SIGNAL(triggered()), window, SLOT(nextMarker()));

	QAction* previous_marker_action = new QAction(tr("Pre&vious Marker"));
	window->addAction(previous_marker_action);
	transport_menu->addAction(previous_marker_action);
#ifdef Q_OS_MACOS
	previous_marker_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left));
#else
	previous_marker_action->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
#endif
	connect(previous_marker_action, SIGNAL(triggered()), window, SLOT(previousMarker()));

	QAction* go_to_marker_action = new QAction(tr("Go to &Marker..."));
	window->addAction(go_to_marker_action);
	transport_menu->addAction(go_to_marker_action);
	go_to_marker_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
	connect(go_to_marker_action, SIGNAL(triggered()), window, SLOT(goToMarker()));
}

void Menu::createToolsMenu(Window* window)
{
	QMenu* tools_menu = window->menuBar()->addMenu(tr("&Tools"));

	QAction* external_utility_action = new QAction(tr("External &Utility..."));
	window->addAction(external_utility_action);
	tools_menu->addAction(external_utility_action);
	external_utility_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));

	tools_menu->addSeparator();

	QAction* preferences_action = new QAction(tr("&Preferences..."));
	window->addAction(preferences_action);
	tools_menu->addAction(preferences_action);
	preferences_action->setShortcut(QKeySequence(QKeySequence::Preferences));
}

void Menu::createHelpMenu(Window* window)
{
	QMenu* help_menu = window->menuBar()->addMenu(tr("&Help"));

	QAction* user_manual_action = new QAction(tr("&User Manual"));
	window->addAction(user_manual_action);
	help_menu->addAction(user_manual_action);

	QAction* about_seqer_action = new QAction(tr("&About Seqer"));
	window->addAction(about_seqer_action);
	help_menu->addAction(about_seqer_action);
	connect(about_seqer_action, SIGNAL(triggered()), window, SLOT(aboutSeqer()));
}

