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
	QSplitter* sidebar_splitter;
	QSplitter* lane_splitter;
	QTabWidget* sidebar_tab_widget;

	Window();
	void closeEvent(QCloseEvent* event);
};

#endif
