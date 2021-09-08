
#include <QtWidgets>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-rtmidi.h>
#include <midiutil-qt.h>
#include "qwertymidi.h"

#define ACTION_NOOP 10000
#define ACTION_EXIT 10001
#define ACTION_PANIC 10002
#define ACTION_SHIFT_UP_OCTAVE 10003
#define ACTION_SHIFT_DOWN_OCTAVE 10004
#define ACTION_STAY_UP_OCTAVE 10005
#define ACTION_STAY_DOWN_OCTAVE 10006
#define ACTION_SHIFT_UP_NOTE 10007
#define ACTION_SHIFT_DOWN_NOTE 10008
#define ACTION_STAY_UP_NOTE 10009
#define ACTION_STAY_DOWN_NOTE 10010
#define ACTION_SUSTAIN 10011
#define ACTION_ALT 10012

static MidiUtilKeyCodes* key_codes = new MidiUtilKeyCodes();
static Window* window;
static TextBox* text_box;
static bool startup_error = false;
static RtMidiOutPtr midi_out = NULL;
static int channel_number = 0;
static int program_number = -1;
static int velocity = 64;
static int map[MIDI_UTIL_NUMBER_OF_KEY_CODES][2];
static int down[MIDI_UTIL_NUMBER_OF_KEY_CODES];
static int down_transposition[MIDI_UTIL_NUMBER_OF_KEY_CODES];
static int transposition = 0;
static int alt = 0;

static void add_mapping(int key, int alt, QString action)
{
	if (action == "noop")
	{
		map[key][alt] = ACTION_NOOP;
	}
	else if (action == "exit")
	{
		map[key][alt] = ACTION_EXIT;
	}
	else if (action == "panic")
	{
		map[key][alt] = ACTION_PANIC;
	}
	else if (action == "shift-up-octave")
	{
		map[key][alt] = ACTION_SHIFT_UP_OCTAVE;
	}
	else if (action == "shift-down-octave")
	{
		map[key][alt] = ACTION_SHIFT_DOWN_OCTAVE;
	}
	else if (action == "stay-up-octave")
	{
		map[key][alt] = ACTION_STAY_UP_OCTAVE;
	}
	else if (action == "stay-down-octave")
	{
		map[key][alt] = ACTION_STAY_DOWN_OCTAVE;
	}
	else if (action == "shift-up-note")
	{
		map[key][alt] = ACTION_SHIFT_UP_NOTE;
	}
	else if (action == "shift-down-note")
	{
		map[key][alt] = ACTION_SHIFT_DOWN_NOTE;
	}
	else if (action == "stay-up-note")
	{
		map[key][alt] = ACTION_STAY_UP_NOTE;
	}
	else if (action == "stay-down-note")
	{
		map[key][alt] = ACTION_STAY_DOWN_NOTE;
	}
	else if (action == "sustain")
	{
		map[key][alt] = ACTION_SUSTAIN;
	}
	else if (action == "alt")
	{
		map[key][alt] = ACTION_ALT;
	}
	else
	{
		int note = MidiUtil_getNoteNumberFromName((char *)(action.toStdString().c_str()));
		if (note >= 0) map[key][alt] = note;
	}
}

static void print_key_binding(QString key_name, int action)
{
	switch (action)
	{
		case ACTION_NOOP:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"noop\" />").arg(key_name));
			break;
		}
		case ACTION_EXIT:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"exit\" />").arg(key_name));
			break;
		}
		case ACTION_PANIC:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"panic\" />").arg(key_name));
			break;
		}
		case ACTION_SHIFT_UP_OCTAVE:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"shift-up-octave\" />").arg(key_name));
			break;
		}
		case ACTION_STAY_UP_OCTAVE:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"stay-up-octave\" />").arg(key_name));
			break;
		}
		case ACTION_SHIFT_DOWN_OCTAVE:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"shift-down-octave\" />").arg(key_name));
			break;
		}
		case ACTION_STAY_DOWN_OCTAVE:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"stay-down-octave\" />").arg(key_name));
			break;
		}
		case ACTION_SHIFT_UP_NOTE:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"shift-up-note\" />").arg(key_name));
			break;
		}
		case ACTION_STAY_UP_NOTE:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"stay-up-note\" />").arg(key_name));
			break;
		}
		case ACTION_SHIFT_DOWN_NOTE:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"shift-down-note\" />").arg(key_name));
			break;
		}
		case ACTION_STAY_DOWN_NOTE:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"stay-down-note\" />").arg(key_name));
			break;
		}
		case ACTION_SUSTAIN:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"sustain\" />").arg(key_name));
			break;
		}
		case ACTION_ALT:
		{
			text_box->setText(QString("<map key=\"%1\" action=\"alt\" />").arg(key_name));
			break;
		}
		default:
		{
			char note_name[128];
			MidiUtil_setNoteNameFromNumber(action, note_name);
			text_box->setText(QString("<map key=\"%1\" action=\"%2\" />").arg(key_name).arg(note_name));
			break;
		}
	}
}

static void send_note_on(int channel, int note, int velocity)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
		MidiUtilMessage_setNoteOn(message, channel, note, velocity);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
	}
}

static void send_note_off(int channel, int note)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
		MidiUtilMessage_setNoteOff(message, channel, note, 0);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
	}
}

static void send_control_change(int channel, int number, int value)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE];
		MidiUtilMessage_setControlChange(message, channel, number, value);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE);
	}
}

static void send_program_change(int channel, int number)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_PROGRAM_CHANGE];
		MidiUtilMessage_setProgramChange(message, channel, number);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_PROGRAM_CHANGE);
	}
}

static void panic(int channel)
{
	int i;
	for (i = 0; i < 127; i++) send_note_off(channel, i);
}

static void do_key_down_action(int key_code, int action);
static void do_key_up_action(int key_code, int action);

static void do_key_down_action(int key_code, int action)
{
	Q_UNUSED(key_code)

	switch (action)
	{
		case ACTION_NOOP:
		{
			break;
		}
		case ACTION_EXIT:
		{
			window->close();
			break;
		}
		case ACTION_PANIC:
		{
			panic(channel_number);
			break;
		}
		case ACTION_SHIFT_UP_OCTAVE:
		{
			transposition += 12;
			break;
		}
		case ACTION_STAY_UP_OCTAVE:
		{
			transposition += 12;
			break;
		}
		case ACTION_SHIFT_DOWN_OCTAVE:
		{
			transposition -= 12;
			break;
		}
		case ACTION_STAY_DOWN_OCTAVE:
		{
			transposition -= 12;
			break;
		}
		case ACTION_SHIFT_UP_NOTE:
		{
			transposition++;
			break;
		}
		case ACTION_STAY_UP_NOTE:
		{
			transposition++;
			break;
		}
		case ACTION_SHIFT_DOWN_NOTE:
		{
			transposition--;
			break;
		}
		case ACTION_STAY_DOWN_NOTE:
		{
			transposition--;
			break;
		}
		case ACTION_SUSTAIN:
		{
			send_control_change(channel_number, 64, 127);
			break;
		}
		case ACTION_ALT:
		{
			int i;

			for (i = 0; i < MIDI_UTIL_NUMBER_OF_KEY_CODES; i++)
			{
				if (down[i] && (map[i][0] != ACTION_ALT) && (map[i][alt] != map[i][1]))
				{
					do_key_up_action(i, map[i][alt]);
					do_key_down_action(i, map[i][1]);
				}
			}

			alt = 1;
			break;
		}
		default:
		{
			int transposed_note = action + transposition;
			if ((transposed_note >= 0) && (transposed_note < 128)) send_note_on(channel_number, transposed_note, velocity);
			break;
		}
	}
}

static void do_key_up_action(int key_code, int action)
{
	switch (action)
	{
		case ACTION_NOOP:
		case ACTION_EXIT:
		case ACTION_PANIC:
		case ACTION_STAY_UP_OCTAVE:
		case ACTION_STAY_DOWN_OCTAVE:
		case ACTION_STAY_UP_NOTE:
		case ACTION_STAY_DOWN_NOTE:
		{
			break;
		}
		case ACTION_SHIFT_UP_OCTAVE:
		{
			transposition -= 12;
			break;
		}
		case ACTION_SHIFT_DOWN_OCTAVE:
		{
			transposition += 12;
			break;
		}
		case ACTION_SHIFT_UP_NOTE:
		{
			transposition--;
			break;
		}
		case ACTION_SHIFT_DOWN_NOTE:
		{
			transposition++;
			break;
		}
		case ACTION_SUSTAIN:
		{
			send_control_change(channel_number, 64, 0);
			break;
		}
		case ACTION_ALT:
		{
			int i;

			for (i = 0; i < MIDI_UTIL_NUMBER_OF_KEY_CODES; i++)
			{
				if (down[i] && (map[i][0] != ACTION_ALT) && (map[i][alt] != map[i][0]))
				{
					do_key_up_action(i, map[i][alt]);
					do_key_down_action(i, map[i][0]);
				}
			}

			alt = 0;
			break;
		}
		default:
		{
			int transposed_note = action + down_transposition[key_code];
			if ((transposed_note >= 0) && (transposed_note < 128)) send_note_off(channel_number, transposed_note);
			break;
		}
	}
}

static void usage(QString program_name)
{
	text_box->setText(QString("Usage:  %1 [ --out <port> ] [ --channel <n> ] [ --program <n> ] [ --velocity <n> ] [ --transpose <n> ] [ --map <filename.xml> ]\n").arg(program_name));
	startup_error = true;
}

void Window::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event)

	if (midi_out != NULL)
	{
		panic(channel_number);
		rtmidi_close_port(midi_out);
	}
}

void TextBox::keyPressEvent(QKeyEvent* event)
{
	if (startup_error) return;
	if (event->isAutoRepeat()) return;
	int key_code = key_codes->getKeyCodeFromEvent(event);
	if (key_code < 0) return;

	if (!down[key_code])
	{
		QString key_name = key_codes->getNameFromKeyCode(key_code);
		int action = map[key_code][alt];
		down[key_code] = 1;
		down_transposition[key_code] = transposition;
		do_key_down_action(key_code, action);
		print_key_binding(key_name, action);
	}
}

void TextBox::keyReleaseEvent(QKeyEvent* event)
{
	if (startup_error) return;
	if (event->isAutoRepeat()) return;
	int key_code = key_codes->getKeyCodeFromEvent(event);
	if (key_code < 0) return;
	int action = map[key_code][alt];
	down[key_code] = 0;
	do_key_up_action(key_code, action);
}

void TextBox::focusOutEvent(QFocusEvent* event)
{
	Q_UNUSED(event)
	if (startup_error) return;
	panic(channel_number);
}

int main(int argc, char** argv)
{
	QApplication application(argc, argv);
	application.setOrganizationName("Sreal");
	application.setOrganizationDomain("sreal.com");
	application.setApplicationName("Qwertymidi");

	window = new Window();

	text_box = new TextBox();
	window->setCentralWidget(text_box);
	text_box->setReadOnly(true);
	text_box->setFrameStyle(QFrame::NoFrame);

	window->resize(640, 480);
	window->show();

	for (int i = 0; i < MIDI_UTIL_NUMBER_OF_KEY_CODES; i++)
	{
		map[i][0] = ACTION_NOOP;
		map[i][1] = ACTION_NOOP;
		down[i] = 0;
		down_transposition[i] = 0;
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port((char *)("qwertymidi"), argv[i], (char *)("qwertymidi"))) == NULL)
			{
				text_box->setText(QString("Error:  Cannot open MIDI output port \"%1\".\n").arg(argv[i]));
				startup_error = true;
			}
		}
		else if (strcmp(argv[i], "--channel") == 0)
		{
			if (++i == argc) usage(argv[0]);
			channel_number = QString(argv[i]).toInt();
		}
		else if (strcmp(argv[i], "--program") == 0)
		{
			if (++i == argc) usage(argv[0]);
			program_number = QString(argv[i]).toInt();
		}
		else if (strcmp(argv[i], "--velocity") == 0)
		{
			if (++i == argc) usage(argv[0]);
			velocity = QString(argv[i]).toInt();
		}
		else if (strcmp(argv[i], "--transpose") == 0)
		{
			if (++i == argc) usage(argv[0]);
			transposition = QString(argv[i]).toInt();
		}
		else if (strcmp(argv[i], "--map") == 0)
		{
			if (++i == argc) usage(argv[0]);
			QFile map_file(argv[i]);

			if (map_file.open(QIODevice::ReadOnly))
			{
				QXmlStreamReader map_xml_reader(&map_file);

				while (!map_xml_reader.atEnd())
				{
					if ((map_xml_reader.readNext() == QXmlStreamReader::StartElement) && (map_xml_reader.name().toString() == "map"))
					{
						int key = key_codes->getKeyCodeFromName(map_xml_reader.attributes().value("key").toString());
						QString action = map_xml_reader.attributes().value("action").toString();
						QString alt_action = map_xml_reader.attributes().value("alt-action").toString();

						if (key >= 0)
						{
							if (action == "")
							{
								add_mapping(key, 0, "noop");
								add_mapping(key, 1, (alt_action == "") ? "noop" : alt_action);
							}
							else
							{
								add_mapping(key, 0, action);
								add_mapping(key, 1, (alt_action == "") ? action : alt_action);
							}
						}
					}
				}

				if (map_xml_reader.hasError())
				{
					text_box->setText(QString("Error:  %s\n").arg(map_xml_reader.errorString()));
					startup_error = true;
				}

				map_file.close();
			}
			else
			{
				text_box->setText(QString("Error:  Cannot open \"%1\"\n").arg(map_file.fileName()));
				startup_error = true;
			}
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (!startup_error && (program_number >= 0)) send_program_change(channel_number, program_number);

	return application.exec();
}

