#ifndef MIDIUTIL_QT_INCLUDED
#define MIDIUTIL_QT_INCLUDED

/* Common helpers that work with Qt. */

#include <QtWidgets>

#define MIDI_UTIL_NUMBER_OF_KEYCODES 256

class MidiUtilKeyCodes
{
public:
	MidiUtilKeyCodes();
	int getKeyCodeFromEvent(QKeyEvent* event);
	int getKeyCodeFromName(QString name);
	QString getNameFromKeyCode(int key_code);
	
private:
	void add(QString name, int qt_key_code, quint32 native_virtual_key = 0);

	QString names[MIDI_UTIL_NUMBER_OF_KEYCODES];
	QHash<QString, int> name_to_key_code_map;
	QHash<QPair<int, quint32>, int> qt_key_code_and_native_virtual_key_to_key_code_map;
	int next_key_code = 0;
};

#endif
