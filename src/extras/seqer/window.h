#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

class Window;

#include <QtWidgets>
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
	float scroll_x = 0;
	bool use_linear_time;
	float pixels_per_beat;
	float pixels_per_second;

	Window(Window* existing_window);
	~Window();
	void closeEvent(QCloseEvent* event);
	void paintEvent(QPaintEvent* event);
	bool save(bool ask_first, bool save_as, QString filename);
	int getXFromTick(long tick);
	long getTickFromX(int x);
	void scrollXBy(float offset);
	void zoomXBy(float factor);

public slots:
	void newSequence();
	void newWindow();
	void open();
	void open(QString filename);
	void save();
	void saveAs();
	void saveAs(QString filename);
	void quit();
	void deleteSelected();
	void selectAll();
	void selectNone();
	void setUseLinearTime(bool use_linear_time);
	void addLane();
	void removeLane();
	void focusInspector();

signals:
	void sequenceUpdated();
};

#endif
