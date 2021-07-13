#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

class Window;

#include <QCloseEvent>
#include <QMainWindow>
#include <QPaintEvent>
#include <QSplitter>
#include <QTabWidget>
#include "inspector-sidebar.h"
#include "sequence.h"

class Window: public QMainWindow
{
	Q_OBJECT

public:
	Sequence* sequence;
	QSplitter* sidebar_splitter;
	QSplitter* lane_splitter;
	QTabWidget* sidebar_tab_widget;
	InspectorSidebar* inspector_sidebar;
	int cursor_x = 0;
	int scroll_x = 0;
	bool use_linear_time;
	int pixels_per_beat;
	int pixels_per_second;

	Window(Window* existing_window);
	~Window();
	void closeEvent(QCloseEvent* event);
	void paintEvent(QPaintEvent* event);
	bool save(bool ask_first, bool save_as, QString filename);
	int getXFromTick(long tick);
	long getTickFromX(int x);
	void createMenuBar();
	void createFileMenu();
	void createEditMenu();
	void createViewMenu();
	void createTransportMenu();
	void createToolsMenu();
	void createHelpMenu();

public slots:
	void newSequence();
	void newWindow();
	void open();
	void open(QString filename);
	void save();
	void saveAs();
	void saveAs(QString filename);
	void quit();
	void selectAll();
	void selectNone();
	void addLane();
	void removeLane();
	void focusInspector();
};

#endif
