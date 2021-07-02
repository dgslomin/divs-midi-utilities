
#include <QAction>
#include <QCloseEvent>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include "window.h"

Window::Window()
{
	this->setWindowTitle("Seqer");

	QAction* close_action = new QAction(tr("&Close"));
	this->addAction(close_action);
	close_action->setShortcut(QKeySequence(QKeySequence::Close));
	connect(close_action, SIGNAL(triggered()), this, SLOT(close()));

	QMenuBar* menu_bar = this->menuBar();
	QMenu* file_menu = menu_bar->addMenu(tr("&File"));
	file_menu->addAction(tr("&New"));
	file_menu->addAction(tr("New &Window"));
	file_menu->addAction(tr("&Open..."));
	file_menu->addAction(tr("&Save"));
	file_menu->addAction(tr("Save &As..."));
	file_menu->addSeparator();
	file_menu->addAction(close_action);
	file_menu->addAction(tr("&Quit"));
	QMenu* edit_menu = menu_bar->addMenu(tr("&Edit"));
	edit_menu->addAction(tr("&Undo"));
	edit_menu->addAction(tr("&Redo"));
	edit_menu->addSeparator();
	edit_menu->addAction(tr("Cu&t"));
	edit_menu->addAction(tr("&Copy"));
	edit_menu->addAction(tr("&Paste"));
	edit_menu->addAction(tr("&Delete"));
	edit_menu->addSeparator();
	edit_menu->addAction(tr("Select &All"));
	edit_menu->addAction(tr("Select &None"));
	edit_menu->addAction(tr("Select Ti&me Range"));
	QMenu* view_menu = menu_bar->addMenu(tr("&View"));
	view_menu->addAction(tr("Next Lane"));
	view_menu->addAction(tr("Pre&vious Lane"));
	view_menu->addSeparator();
	view_menu->addAction(tr("&Add Lane"));
	view_menu->addAction(tr("&Remove Lane"));
	view_menu->addAction(tr("Move Lane Up"));
	view_menu->addAction(tr("Move Lane Down"));
	view_menu->addSeparator();
	view_menu->addAction(tr("Zoom &In Lane"));
	view_menu->addAction(tr("Zoom &Out Lane"));
	view_menu->addAction(tr("Zoom &In Time"));
	view_menu->addAction(tr("Zoom &Out Time"));
	QMenu* transport_menu = menu_bar->addMenu(tr("Trans&port"));
	transport_menu->addAction(tr("&Play"));
	transport_menu->addAction(tr("&Stop"));
	transport_menu->addAction(tr("&Record"));
	transport_menu->addSeparator();
	transport_menu->addAction(tr("Set Playback Position"));
	transport_menu->addAction(tr("Go to Playback Position"));
	transport_menu->addSeparator();
	transport_menu->addAction(tr("&Next Marker"));
	transport_menu->addAction(tr("Pre&vious Marker"));
	transport_menu->addAction(tr("Go to &Marker..."));
	QMenu* tools_menu = menu_bar->addMenu(tr("&Tools"));
	tools_menu->addAction(tr("External &Utility..."));
	tools_menu->addSeparator();
	tools_menu->addAction(tr("&Record Macro..."));
	tools_menu->addAction(tr("&Macros..."));
	tools_menu->addSeparator();
	tools_menu->addAction(tr("&Preferences..."));
	QMenu* help_menu = menu_bar->addMenu(tr("&Help"));
	help_menu->addAction(tr("&User Manual"));
	help_menu->addAction(tr("&About Seqer"));

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

	this->statusBar();

	QSettings settings;
	this->restoreGeometry(settings.value("state/window-geometry").toByteArray());
	this->restoreState(settings.value("state/window-state").toByteArray());
	this->lane_splitter->restoreState(settings.value("state/lane-splitter-state").toByteArray());
	this->sidebar_splitter->restoreState(settings.value("state/sidebar-splitter-state").toByteArray());
	this->sidebar_tab_widget->setCurrentIndex(settings.value("state/sidebar-tab-index", 0).toInt());
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

