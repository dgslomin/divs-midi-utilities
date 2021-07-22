
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
	QMenu* file_menu = window->menuBar()->addMenu(QObject::tr("&File"));

	QAction* new_sequence_action = new QAction(QObject::tr("&New"));
	window->addAction(new_sequence_action);
	file_menu->addAction(new_sequence_action);
	new_sequence_action->setShortcut(QKeySequence(QKeySequence::New));
	QObject::connect(new_sequence_action, SIGNAL(triggered()), window, SLOT(newSequence()));

	QAction* new_window_action = new QAction(QObject::tr("New &Window"));
	window->addAction(new_window_action);
	new_window_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
	file_menu->addAction(new_window_action);
	QObject::connect(new_window_action, SIGNAL(triggered()), window, SLOT(newWindow()));

	QAction* open_action = new QAction(QObject::tr("&Open..."));
	window->addAction(open_action);
	file_menu->addAction(open_action);
	open_action->setShortcut(QKeySequence(QKeySequence::Open));
	QObject::connect(open_action, SIGNAL(triggered()), window, SLOT(open()));

	QAction* save_action = new QAction(QObject::tr("&Save"));
	window->addAction(save_action);
	file_menu->addAction(save_action);
	save_action->setShortcut(QKeySequence(QKeySequence::Save));
	QObject::connect(save_action, SIGNAL(triggered()), window, SLOT(save()));

	QAction* save_as_action = new QAction(QObject::tr("Save &As..."));
	window->addAction(save_as_action);
	save_as_action->setShortcut(QKeySequence(QKeySequence::SaveAs));
	file_menu->addAction(save_as_action);
	QObject::connect(save_as_action, SIGNAL(triggered()), window, SLOT(saveAs()));

	file_menu->addSeparator();

	QAction* close_action = new QAction(QObject::tr("&Close"));
	window->addAction(close_action);
	file_menu->addAction(close_action);
	close_action->setShortcut(QKeySequence(QKeySequence::Close));
	QObject::connect(close_action, SIGNAL(triggered()), window, SLOT(close()));

	QAction* quit_action = new QAction(QObject::tr("&Quit"));
	window->addAction(quit_action);
	file_menu->addAction(quit_action);
	quit_action->setShortcut(QKeySequence(QKeySequence::Quit));
	QObject::connect(quit_action, SIGNAL(triggered()), window, SLOT(quit()));
}

void Menu::createEditMenu(Window* window)
{
	QMenu* edit_menu = window->menuBar()->addMenu(QObject::tr("&Edit"));

	QAction* undo_action = new QAction(QObject::tr("&Undo"));
	window->addAction(undo_action);
	edit_menu->addAction(undo_action);
	undo_action->setShortcut(QKeySequence(QKeySequence::Undo));

	QAction* redo_action = new QAction(QObject::tr("&Redo"));
	window->addAction(redo_action);
	edit_menu->addAction(redo_action);
	redo_action->setShortcut(QKeySequence(QKeySequence::Redo));

	edit_menu->addSeparator();

	QAction* cut_action = new QAction(QObject::tr("Cu&t"));
	window->addAction(cut_action);
	edit_menu->addAction(cut_action);
	cut_action->setShortcut(QKeySequence(QKeySequence::Cut));
	QObject::connect(cut_action, SIGNAL(triggered()), window, SIGNAL(cut()));

	QAction* copy_action = new QAction(QObject::tr("&Copy"));
	window->addAction(copy_action);
	edit_menu->addAction(copy_action);
	copy_action->setShortcut(QKeySequence(QKeySequence::Copy));
	QObject::connect(copy_action, SIGNAL(triggered()), window, SIGNAL(copy_()));

	QAction* paste_action = new QAction(QObject::tr("&Paste"));
	window->addAction(paste_action);
	edit_menu->addAction(paste_action);
	paste_action->setShortcut(QKeySequence(QKeySequence::Paste));
	QObject::connect(paste_action, SIGNAL(triggered()), window, SIGNAL(paste()));

	QAction* delete_action = new QAction(QObject::tr("&Delete"));
	window->addAction(delete_action);
	edit_menu->addAction(delete_action);
	delete_action->setShortcut(QKeySequence(QKeySequence::Delete));
	QObject::connect(delete_action, SIGNAL(triggered()), window, SLOT(delete_()));

	edit_menu->addSeparator();

	QAction* select_all_action = new QAction(QObject::tr("Select &All"));
	window->addAction(select_all_action);
	edit_menu->addAction(select_all_action);
	select_all_action->setShortcut(QKeySequence(QKeySequence::SelectAll));
	QObject::connect(select_all_action, SIGNAL(triggered()), window, SLOT(selectAll()));

	QAction* select_none_action = new QAction(QObject::tr("Select &None"));
	window->addAction(select_none_action);
	edit_menu->addAction(select_none_action);
	select_none_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
	QObject::connect(select_none_action, SIGNAL(triggered()), window, SLOT(selectNone()));

	QAction* select_time_range_action = new QAction(QObject::tr("Select Ti&me Range"));
	window->addAction(select_time_range_action);
	edit_menu->addAction(select_time_range_action);
}

void Menu::createViewMenu(Window* window)
{
	QMenu* view_menu = window->menuBar()->addMenu(QObject::tr("&View"));

	QAction* zoom_in_time_action = new QAction(QObject::tr("Zoom &In Time"));
	window->addAction(zoom_in_time_action);
	view_menu->addAction(zoom_in_time_action);
	zoom_in_time_action->setShortcut(QKeySequence(QKeySequence::ZoomIn));
	QObject::connect(zoom_in_time_action, SIGNAL(triggered()), window, SLOT(zoomInTime()));

	QAction* zoom_out_time_action = new QAction(QObject::tr("Zoom &Out Time"));
	window->addAction(zoom_out_time_action);
	view_menu->addAction(zoom_out_time_action);
	zoom_out_time_action->setShortcut(QKeySequence(QKeySequence::ZoomOut));
	QObject::connect(zoom_out_time_action, SIGNAL(triggered()), window, SLOT(zoomOutTime()));

	QAction* zoom_in_lane_action = new QAction(QObject::tr("Zoom &In Lane"));
	window->addAction(zoom_in_lane_action);
	view_menu->addAction(zoom_in_lane_action);
	zoom_in_lane_action->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Plus));
	QObject::connect(zoom_in_lane_action, SIGNAL(triggered()), window, SIGNAL(zoomInLane()));

	QAction* zoom_out_lane_action = new QAction(QObject::tr("Zoom &Out Lane"));
	window->addAction(zoom_out_lane_action);
	view_menu->addAction(zoom_out_lane_action);
	zoom_out_lane_action->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Minus));
	QObject::connect(zoom_out_lane_action, SIGNAL(triggered()), window, SIGNAL(zoomOutLane()));

	view_menu->addSeparator();

	QAction* add_lane_action = new QAction(QObject::tr("&Add Lane"));
	window->addAction(add_lane_action);
	view_menu->addAction(add_lane_action);
	QObject::connect(add_lane_action, SIGNAL(triggered()), window, SLOT(addLane()));

	QAction* remove_lane_action = new QAction(QObject::tr("&Remove Lane"));
	window->addAction(remove_lane_action);
	view_menu->addAction(remove_lane_action);
	QObject::connect(remove_lane_action, SIGNAL(triggered()), window, SLOT(removeLane()));

	QAction* move_lane_up_action = new QAction(QObject::tr("Move Lane &Up"));
	window->addAction(move_lane_up_action);
	view_menu->addAction(move_lane_up_action);

	QAction* move_lane_down_action = new QAction(QObject::tr("Move Lane &Down"));
	window->addAction(move_lane_down_action);
	view_menu->addAction(move_lane_down_action);

	view_menu->addSeparator();

	QAction* linear_time_action = new QAction(QObject::tr("&Linear Time"));
	window->addAction(linear_time_action);
	view_menu->addAction(linear_time_action);
	linear_time_action->setCheckable(true);
	linear_time_action->setChecked(window->use_linear_time);
	QObject::connect(linear_time_action, SIGNAL(toggled(bool)), window, SLOT(setUseLinearTime(bool)));
}

void Menu::createTransportMenu(Window* window)
{
	QMenu* transport_menu = window->menuBar()->addMenu(QObject::tr("Trans&port"));

	QAction* play_action = new QAction(QObject::tr("&Play"));
	window->addAction(play_action);
	transport_menu->addAction(play_action);
	play_action->setShortcut(QKeySequence(Qt::Key_Space));

	QAction* record_action = new QAction(QObject::tr("&Record"));
	window->addAction(record_action);
	transport_menu->addAction(record_action);
	record_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));

	transport_menu->addSeparator();

	QAction* set_play_position_action = new QAction(QObject::tr("Set Play Position"));
	window->addAction(set_play_position_action);
	transport_menu->addAction(set_play_position_action);

	QAction* go_to_play_position_action = new QAction(QObject::tr("Go to Play Position"));
	window->addAction(go_to_play_position_action);
	transport_menu->addAction(go_to_play_position_action);

	QAction* go_to_stop_position_action = new QAction(QObject::tr("Go to Stop Position"));
	window->addAction(go_to_stop_position_action);
	transport_menu->addAction(go_to_stop_position_action);

	transport_menu->addSeparator();

	QAction* next_marker_action = new QAction(QObject::tr("&Next Marker"));
	window->addAction(next_marker_action);
	transport_menu->addAction(next_marker_action);
	next_marker_action->setShortcut(QKeySequence(QKeySequence::MoveToNextWord));

	QAction* previous_marker_action = new QAction(QObject::tr("Pre&vious Marker"));
	window->addAction(previous_marker_action);
	transport_menu->addAction(previous_marker_action);
	previous_marker_action->setShortcut(QKeySequence(QKeySequence::MoveToPreviousWord));

	QAction* go_to_marker_action = new QAction(QObject::tr("Go to &Marker..."));
	window->addAction(go_to_marker_action);
	transport_menu->addAction(go_to_marker_action);
	go_to_marker_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
}

void Menu::createToolsMenu(Window* window)
{
	QMenu* tools_menu = window->menuBar()->addMenu(QObject::tr("&Tools"));

	QAction* external_utility_action = new QAction(QObject::tr("External &Utility..."));
	window->addAction(external_utility_action);
	tools_menu->addAction(external_utility_action);
	external_utility_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));

	tools_menu->addSeparator();

	QAction* record_macro_action = new QAction(QObject::tr("&Record Macro..."));
	window->addAction(record_macro_action);
	tools_menu->addAction(record_macro_action);
	record_macro_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));

	QAction* macros_action = new QAction(QObject::tr("&Macros..."));
	window->addAction(macros_action);
	tools_menu->addAction(macros_action);

	tools_menu->addSeparator();

	QAction* preferences_action = new QAction(QObject::tr("&Preferences..."));
	window->addAction(preferences_action);
	tools_menu->addAction(preferences_action);
	preferences_action->setShortcut(QKeySequence(QKeySequence::Preferences));
}

void Menu::createHelpMenu(Window* window)
{
	QMenu* help_menu = window->menuBar()->addMenu(QObject::tr("&Help"));

	QAction* user_manual_action = new QAction(QObject::tr("&User Manual"));
	window->addAction(user_manual_action);
	help_menu->addAction(user_manual_action);

	QAction* about_seqer_action = new QAction(QObject::tr("&About Seqer"));
	window->addAction(about_seqer_action);
	help_menu->addAction(about_seqer_action);
	QObject::connect(about_seqer_action, SIGNAL(triggered()), window, SLOT(aboutSeqer()));
}

