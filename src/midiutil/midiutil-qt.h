#ifndef MIDIUTIL_QT_INCLUDED
#define MIDIUTIL_QT_INCLUDED

/* Common helpers that work with Qt. */

#include <QtWidgets>

#define MIDI_UTIL_NUMBER_OF_KEY_CODES 256

class MidiUtilKeyCodes
{
public:
	MidiUtilKeyCodes();
	int getKeyCodeFromEvent(QKeyEvent* event);
	int getKeyCodeFromName(QString name);
	QString getNameFromKeyCode(int key_code);
	
private:
	void add(QString name, quint32 native_code);

	QString names[MIDI_UTIL_NUMBER_OF_KEY_CODES];
	QHash<QString, int> name_to_key_code_map;
	QHash<quint32, int> native_code_to_key_code_map;
	int next_key_code = 0;
};

#endif
