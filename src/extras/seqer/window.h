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
	static QVector<Window*> windows;
	bool confirm_on_close = true;
	int cursor_x = 0;
	float scroll_x = 0;
	Sequence* sequence;
	QSplitter* sidebar_splitter;
	QSplitter* lane_splitter;
	QTabWidget* sidebar_tab_widget;
	InspectorSidebar* inspector_sidebar;
	bool use_linear_time;
	float pixels_per_beat;
	float pixels_per_second;

	Window(Window* existing_window);
	~Window();
	void closeEvent(QCloseEvent* event);
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
	void undo();
	void redo();
	void delete_();
	void selectAll();
	void selectNone();
	void zoomInTime();
	void zoomOutTime();
	void addLane();
	void removeLane();
	void setUseLinearTime(bool use_linear_time);
	void aboutSeqer();
	void underlyingSequenceUpdated();
	void focusInspector();

signals:
	void sequenceUpdated();
	void cut();
	void copy_();
	void paste();
	void zoomInLane();
	void zoomOutLane();
};

#endif
