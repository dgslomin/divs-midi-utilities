#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

class Window;

#include <QCloseEvent>
#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include "sequence.h"

class Window: public QMainWindow
{
	Q_OBJECT

public:
	Sequence* sequence;
	QSplitter* sidebar_splitter;
	QSplitter* lane_splitter;
	QTabWidget* sidebar_tab_widget;
	bool use_linear_time;
	int pixels_per_beat;
	int pixels_per_second;
	int scroll_x;

	Window(Window* existing_window);
	~Window();
	void closeEvent(QCloseEvent* event);
	bool isModified();
	bool isLastWindowForSequence();
	QString getFilename();
	void new_();
	void newWindow();
	void open();
	bool load(QString filename);
	bool saveChangesIfNeeded();
	bool saveChanges();
	bool save();
	bool saveAs();
	void quit();
	void selectAll();
	void selectNone();
	void addLane();
	void removeLane();
	void focusInspector();
	void refreshData();
	void refreshDisplay();
	int getXFromTick(long tick);
	long getTickFromX(int x);
	void createMenuBar();
	void createFileMenu();
	void createEditMenu();
	void createViewMenu();
	void createTransportMenu();
	void createToolsMenu();
	void createHelpMenu();
};

#endif
