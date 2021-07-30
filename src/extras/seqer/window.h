#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

class Window;

#include <QtWidgets>
#include "inspector-sidebar.h"
#include "lane.h"
#include "sequence.h"

class Window: public QMainWindow
{
	Q_OBJECT

public:
	static QVector<Window*> windows;
	static bool confirm_on_close;

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
	Lane* getFocusedLane();
	void scrollCursorIntoView();

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
	void cut();
	void copy_();
	void paste();
	void delete_();
	void selectAll();
	void selectNone();
	void zoomInTime();
	void zoomOutTime();
	void zoomInLane();
	void zoomOutLane();
	void addNoteLane();
	void addVelocityLane();
	void addControllerLane();
	void addTempoLane();
	void addMarkerLane();
	void addAllEventsLane();
	void removeLane();
	void moveLaneUp();
	void moveLaneDown();
	void setUseLinearTime(bool use_linear_time);
	void nextMarker();
	void previousMarker();
	void goToMarker();
	void aboutSeqer();
	void sequenceUpdated();
	void focusInspector();
};

#endif
