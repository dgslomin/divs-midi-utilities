
class Window;
class TextBox;

#include <QtWidgets>
#include <rtmidi_c.h>

class Window: public QMainWindow
{
	Q_OBJECT

public:
	void closeEvent(QCloseEvent* event);

	QTextEdit* text_box;
};

class TextBox: public QTextEdit
{
	Q_OBJECT

public:
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void focusOutEvent(QFocusEvent* event);
};

