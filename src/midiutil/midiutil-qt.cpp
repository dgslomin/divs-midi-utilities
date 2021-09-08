
#include <QtWidgets>
#include <midiutil-qt.h>

MidiUtilKeyCodes::MidiUtilKeyCodes()
{
#if defined(Q_OS_WIN)
	// Adapted from <https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html>
	this->add("escape", 0x01);
	this->add("f1", 0x3B);
	this->add("f2", 0x3C);
	this->add("f3", 0x3D);
	this->add("f4", 0x3E);
	this->add("f5", 0x3F);
	this->add("f6", 0x40);
	this->add("f7", 0x41);
	this->add("f8", 0x42);
	this->add("f9", 0x43);
	this->add("f10", 0x44);
	this->add("f11", 0x57);
	this->add("f12", 0x58);
	this->add("backtick", 0x29);
	this->add("1", 0x02);
	this->add("2", 0x03);
	this->add("3", 0x04);
	this->add("4", 0x05);
	this->add("5", 0x06);
	this->add("6", 0x07);
	this->add("7", 0x08);
	this->add("8", 0x09);
	this->add("9", 0x0A);
	this->add("0", 0x0B);
	this->add("minus", 0x0C);
	this->add("equals", 0x0D);
	this->add("backspace", 0x0E);
	this->add("tab", 0x0F);
	this->add("Q", 0x10);
	this->add("W", 0x11);
	this->add("E", 0x12);
	this->add("R", 0x13);
	this->add("T", 0x14);
	this->add("Y", 0x15);
	this->add("U", 0x16);
	this->add("I", 0x17);
	this->add("O", 0x18);
	this->add("P", 0x19);
	this->add("open-bracket", 0x1A);
	this->add("close-bracket", 0x1B);
	this->add("backslash", 0x2B);
	this->add("caps-lock", 0x3A);
	this->add("A", 0x1E);
	this->add("S", 0x1F);
	this->add("D", 0x20);
	this->add("F", 0x21);
	this->add("G", 0x22);
	this->add("H", 0x23);
	this->add("J", 0x24);
	this->add("K", 0x25);
	this->add("L", 0x26);
	this->add("semicolon", 0x27);
	this->add("single-quote", 0x28);
	this->add("enter", 0x1C);
	this->add("shift-left", 0x2A);
	this->add("shift", 0x2A);
	this->add("Z", 0x2C);
	this->add("X", 0x2D);
	this->add("C", 0x2E);
	this->add("V", 0x2F);
	this->add("B", 0x30);
	this->add("N", 0x31);
	this->add("M", 0x32);
	this->add("comma", 0x33);
	this->add("period", 0x34);
	this->add("slash", 0x35);
	this->add("shift-right", 0x36);
	this->add("control-left", 0x1D);
	this->add("control", 0x1D);
	this->add("windows-left", 0x5B);
	this->add("windows", 0x5B);
	this->add("alt-left", 0x38);
	this->add("alt", 0x38);
	this->add("space", 0x39);
	this->add("alt-right", 0x138);
	this->add("windows-menu", 0x5D);
	this->add("windows-right", 0x5C);
	this->add("control-right", 0x11D);
	this->add("scroll-lock", 0x46);
	this->add("insert", 0x152);
	this->add("delete", 0x153);
	this->add("home", 0x147);
	this->add("end", 0x14F);
	this->add("page-up", 0x149);
	this->add("page-down", 0x151);
	this->add("up", 0x148);
	this->add("down", 0x150);
	this->add("left", 0x14B);
	this->add("right", 0x14D);
	this->add("num-lock", 0x45);
	this->add("numpad-1", 0x4F);
	this->add("numpad-2", 0x50);
	this->add("numpad-3", 0x51);
	this->add("numpad-4", 0x4B);
	this->add("numpad-5", 0x4C);
	this->add("numpad-6", 0x4D);
	this->add("numpad-7", 0x47);
	this->add("numpad-8", 0x48);
	this->add("numpad-9", 0x49);
	this->add("numpad-0", 0x52);
	this->add("numpad-decimal", 0x53);
	this->add("numpad-multiply", 0x37);
	this->add("numpad-divide", 0x135);
	this->add("numpad-add", 0x4E);
	this->add("numpad-subtract", 0x4A);
	this->add("numpad-enter", 0x11C);
#elif defined(Q_OS_MACOS)
	// Adapted from <https://eastmanreference.com/complete-list-of-applescript-key-codes>
	this->add("escape", 53);
	this->add("f1", 122);
	this->add("f2", 120);
	this->add("f3", 99);
	this->add("f4", 118);
	this->add("f5", 96);
	this->add("f6", 97);
	this->add("f7", 98);
	this->add("f8", 100);
	this->add("f9", 101);
	this->add("f10", 109);
	this->add("f11", 103);
	this->add("f12", 111);
	this->add("f13", 105);
	this->add("f14", 107);
	this->add("f15", 113);
	this->add("f16", 106);
	this->add("f17", 64);
	this->add("f18", 79);
	this->add("f19", 80);
	this->add("f20", 90);
	this->add("backtick", 50);
	this->add("1", 18);
	this->add("2", 19);
	this->add("3", 20);
	this->add("4", 21);
	this->add("5", 23);
	this->add("6", 22);
	this->add("7", 26);
	this->add("8", 28);
	this->add("9", 25);
	this->add("0", 29);
	this->add("minus", 27);
	this->add("equals", 24);
	this->add("backspace", 51);
	this->add("tab", 48);
	this->add("Q", 12);
	this->add("W", 13);
	this->add("E", 14);
	this->add("R", 15);
	this->add("T", 17);
	this->add("Y", 16);
	this->add("U", 32);
	this->add("I", 34);
	this->add("O", 31);
	this->add("P", 35);
	this->add("open-bracket", 33);
	this->add("close-bracket", 30);
	this->add("backslash", 42);
	this->add("caps-lock", 57);
	this->add("A", 0);
	this->add("S", 1);
	this->add("D", 2);
	this->add("F", 3);
	this->add("G", 5);
	this->add("H", 4);
	this->add("J", 38);
	this->add("K", 40);
	this->add("L", 37);
	this->add("semicolon", 41);
	this->add("single-quote", 39);
	this->add("enter", 36);
	this->add("shift-left", 57);
	this->add("shift", 57);
	this->add("Z", 6);
	this->add("X", 7);
	this->add("C", 8);
	this->add("V", 9);
	this->add("B", 11);
	this->add("N", 45);
	this->add("M", 46);
	this->add("comma", 43);
	this->add("period", 47);
	this->add("slash", 44);
	this->add("shift-right", 60);
	this->add("control-left", 59);
	this->add("control", 59);
	this->add("option-left", 58);
	this->add("option", 58);
	this->add("command-left", 55);
	this->add("command", 55);
	this->add("space", 49);
	this->add("command-right", 54);
	this->add("option-right", 61);
	this->add("control-right", 62);
	this->add("help", 0x72);
	this->add("delete", 0x75);
	this->add("home", 115);
	this->add("end", 119);
	this->add("page-up", 116);
	this->add("page-down", 121);
	this->add("up", 126);
	this->add("down", 125);
	this->add("left", 123);
	this->add("right", 124);
	this->add("numpad-1", 83);
	this->add("numpad-2", 84);
	this->add("numpad-3", 85);
	this->add("numpad-4", 86);
	this->add("numpad-5", 87);
	this->add("numpad-6", 88);
	this->add("numpad-7", 89);
	this->add("numpad-8", 91);
	this->add("numpad-9", 92);
	this->add("numpad-0", 82);
	this->add("numpad-decimal", 65);
	this->add("numpad-multiply", 67);
	this->add("numpad-divide", 75);
	this->add("numpad-add", 69);
	this->add("numpad-subtract", 78);
	this->add("numpad-equal", 81);
	this->add("numpad-clear", 71);
	this->add("numpad-enter", 76);
#elif defined(Q_OS_UNIX)
	// Adapted from <https://cgit.freedesktop.org/xorg/proto/x11proto/tree/keysymdef.h>
	this->add("escape", 0xFF1B);
	this->add("f1", 0xFFBE);
	this->add("f2", 0xFFBF);
	this->add("f3", 0xFFC0);
	this->add("f4", 0xFFC1);
	this->add("f5", 0xFFC2);
	this->add("f6", 0xFFC3);
	this->add("f7", 0xFFC4);
	this->add("f8", 0xFFC5);
	this->add("f9", 0xFFC6);
	this->add("f10", 0xFFC7);
	this->add("f11", 0xFFC8);
	this->add("f12", 0xFFC9);
	this->add("backtick", 0x60);
	this->add("backtick", 0x7E);
	this->add("1", 0x31);
	this->add("1", 0x21);
	this->add("2", 0x32);
	this->add("2", 0x40);
	this->add("3", 0x33);
	this->add("3", 0x23);
	this->add("4", 0x34);
	this->add("4", 0x24);
	this->add("5", 0x35);
	this->add("5", 0x25);
	this->add("6", 0x36);
	this->add("6", 0x5E);
	this->add("7", 0x37);
	this->add("7", 0x26);
	this->add("8", 0x38);
	this->add("8", 0x2A);
	this->add("9", 0x39);
	this->add("9", 0x28);
	this->add("0", 0x30);
	this->add("0", 0x29);
	this->add("minus", 0x2D);
	this->add("minus", 0x5F);
	this->add("equals", 0x3D);
	this->add("equals", 0x2B);
	this->add("backspace", 0xFF08);
	this->add("tab", 0xFF09);
	this->add("Q", 0x71);
	this->add("Q", 0x51);
	this->add("W", 0x77);
	this->add("W", 0x57);
	this->add("E", 0x65);
	this->add("E", 0x45);
	this->add("R", 0x72);
	this->add("R", 0x52);
	this->add("T", 0x74);
	this->add("T", 0x54);
	this->add("Y", 0x79);
	this->add("Y", 0x59);
	this->add("U", 0x75);
	this->add("U", 0x55);
	this->add("I", 0x69);
	this->add("I", 0x49);
	this->add("O", 0x6F);
	this->add("O", 0x4F);
	this->add("P", 0x70);
	this->add("P", 0x50);
	this->add("open-bracket", 0x5B);
	this->add("open-bracket", 0x7B);
	this->add("close-bracket", 0x5D);
	this->add("close-bracket", 0x7D);
	this->add("backslash", 0x5C);
	this->add("backslash", 0x7C);
	this->add("caps-lock", 0xFFE5);
	this->add("A", 0x61);
	this->add("A", 0x41);
	this->add("S", 0x73);
	this->add("S", 0x53);
	this->add("D", 0x64);
	this->add("D", 0x44);
	this->add("F", 0x66);
	this->add("F", 0x46);
	this->add("G", 0x67);
	this->add("G", 0x47);
	this->add("H", 0x68);
	this->add("H", 0x48);
	this->add("J", 0x6A);
	this->add("J", 0x4A);
	this->add("K", 0x6B);
	this->add("K", 0x4B);
	this->add("L", 0x6C);
	this->add("L", 0x4C);
	this->add("semicolon", 0x3B);
	this->add("semicolon", 0x3A);
	this->add("single-quote", 0x27);
	this->add("single-quote", 0x22);
	this->add("enter", 0xFF0D);
	this->add("shift-left", 0xFFE1);
	this->add("shift", 0xFFE1);
	this->add("Z", 0x7A);
	this->add("Z", 0x5A);
	this->add("X", 0x78);
	this->add("X", 0x58);
	this->add("C", 0x63);
	this->add("C", 0x43);
	this->add("V", 0x76);
	this->add("V", 0x56);
	this->add("B", 0x62);
	this->add("B", 0x42);
	this->add("N", 0x6E);
	this->add("N", 0x4E);
	this->add("M", 0x6D);
	this->add("M", 0x4D);
	this->add("comma", 0x2C);
	this->add("comma", 0x3C);
	this->add("period", 0x2E);
	this->add("period", 0x3E);
	this->add("slash", 0x2F);
	this->add("slash", 0x3F);
	this->add("shift-right", 0xFFE2);
	this->add("control-left", 0xFFE3);
	this->add("control", 0xFFE3);
	this->add("windows-left", 0xFFE7);
	this->add("windows", 0xFFE7);
	this->add("alt-left", 0xFFE9);
	this->add("alt", 0xFFE9);
	this->add("space", 0x20);
	this->add("alt-right", 0xFFEA);
	this->add("windows-menu", 0xFF67);
	this->add("windows-right", 0xFFE8);
	this->add("control-right", 0xFFE4);
	this->add("scroll-lock", 0xFF14);
	this->add("insert", 0xFF63);
	this->add("delete", 0xFFFF);
	this->add("home", 0xFF50);
	this->add("end", 0xFF57);
	this->add("page-up", 0xFF55);
	this->add("page-down", 0xFF56);
	this->add("up", 0xFF52);
	this->add("down", 0xFF54);
	this->add("left", 0xFF51);
	this->add("right", 0xFF53);
#endif
}

int MidiUtilKeyCodes::getKeyCodeFromEvent(QKeyEvent* event)
{
	quint32 native_code;

#if defined(Q_OS_WIN)
	native_code = event->nativeScanCode();
#else
	native_code = event->nativeVirtualKey();
#endif

	return this->native_code_to_key_code_map.value(native_code, -1);
}

int MidiUtilKeyCodes::getKeyCodeFromName(QString name)
{
	return this->name_to_key_code_map.value(name, -1);
}

QString MidiUtilKeyCodes::getNameFromKeyCode(int number)
{
	return this->names[number];
}

void MidiUtilKeyCodes::add(QString name, quint32 native_code)
{
	int key_code = this->name_to_key_code_map.value(name, -1);

	if (key_code < 0)
	{
		key_code = this->native_code_to_key_code_map.value(native_code, -1);

		if (key_code < 0)
		{
			key_code = (this->next_key_code)++;
			this->native_code_to_key_code_map.insert(native_code, key_code);
			this->names[key_code] = name;
		}

		this->name_to_key_code_map.insert(name, key_code);
	}
	else
	{
		this->native_code_to_key_code_map.insert(native_code, key_code);
	}
}

