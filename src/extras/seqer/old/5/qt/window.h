#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

class Window;

#include <QCloseEvent>
#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>

class Window: public QMainWindow
{
	Q_OBJECT

public:
	Window();

private:
	QSplitter* sidebar_splitter;
	QSplitter* lane_splitter;
	QTabWidget* sidebar_tab_widget;

	void closeEvent(QCloseEvent* event);
	void createMenuBar();
	void createFileMenu();
	void createEditMenu();
	void createViewMenu();
	void createTransportMenu();
	void createToolsMenu();
	void createHelpMenu();
};

#endif
