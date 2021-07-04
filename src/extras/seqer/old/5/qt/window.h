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
	void newFile();
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
	void refreshData();
	void refreshDisplay();
};

#endif
