
#include <QtWidgets>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-rtmidi.h>
#include <midiutil-qt.h>
#include "delta.h"

#define ACTION_NOOP 10000
#define ACTION_EXIT 10001

static MidiUtilKeyCodes* key_codes = new MidiUtilKeyCodes();
static Window* window;
static TextBox* text_box;
static bool startup_error = false;
static RtMidiOutPtr midi_out = NULL;
static int channel_number = 0;
static int program_number = -1;
static int velocity = 64;
static int map[MIDI_UTIL_NUMBER_OF_KEY_CODES];
static int down[MIDI_UTIL_NUMBER_OF_KEY_CODES];
static int current_note = 63;
static int current_key_code = -1;

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

static void send_program_change(int channel, int number)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_PROGRAM_CHANGE];
		MidiUtilMessage_setProgramChange(message, channel, number);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_PROGRAM_CHANGE);
	}
}

static void usage(QString program_name)
{
	text_box->setText(QString("Usage:  %1 [ --out <port> ] [ --channel <n> ] [ --program <n> ] [ --velocity <n> ] [ --map <filename.xml> ]\n").arg(program_name));
	startup_error = true;
}

void Window::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event)

	if (midi_out != NULL)
	{
		send_note_off(channel_number, current_note);
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
		int action = map[key_code];
		down[key_code] = 1;

		switch (action)
		{
			case ACTION_NOOP:
			{
				this->setText(QString("<map key=\"%1\" action=\"noop\" />").arg(key_name));
				break;
			}
			case ACTION_EXIT:
			{
				::window->close();
				break;
			}
			default:
			{
				char note_name[128];

				send_note_off(channel_number, current_note);
				current_note = MidiUtil_clamp(current_note + action, 0, 127);
				send_note_on(channel_number, current_note, velocity);
				current_key_code = key_code;

				MidiUtil_setNoteNameFromNumber(action, note_name);
				this->setText(QString("<map key=\"%1\" action=\"%2\" />").arg(key_name).arg(action));
				break;
			}
		}
	}
}

void TextBox::keyReleaseEvent(QKeyEvent* event)
{
	if (startup_error) return;
	if (event->isAutoRepeat()) return;
	int key_code = key_codes->getKeyCodeFromEvent(event);
	if (key_code < 0) return;
	int action = map[key_code];
	down[key_code] = 0;

	switch (action)
	{
		case ACTION_NOOP:
		case ACTION_EXIT:
		{
			break;
		}
		default:
		{
			if (key_code == current_key_code) send_note_off(channel_number, current_note);
			break;
		}
	}
}

void TextBox::focusOutEvent(QFocusEvent* event)
{
	Q_UNUSED(event)
	if (startup_error) return;
	send_note_off(channel_number, current_note);
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
		map[i] = ACTION_NOOP;
		down[i] = 0;
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port((char *)("delta"), argv[i], (char *)("delta"))) == NULL)
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

						if ((key >= 0) && (action != ""))
						{
							if (action == "noop")
							{
								map[key] = ACTION_NOOP;
							}
							else if (action == "exit")
							{
								map[key] = ACTION_EXIT;
							}
							else
							{
								map[key] = action.toInt();
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

