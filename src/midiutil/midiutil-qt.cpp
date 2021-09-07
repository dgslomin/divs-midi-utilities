
#include <QtWidgets>
#include <midiutil-qt.h>

MidiUtilKeyCodes::MidiUtilKeyCodes()
{
	this->add("space", Qt::Key_Space);
	this->add("exclamation", Qt::Key_Exclam);
	this->add("double-quote", Qt::Key_QuoteDbl);
	this->add("number", Qt::Key_NumberSign);
	this->add("dollar", Qt::Key_Dollar);
	this->add("percent", Qt::Key_Percent);
	this->add("ampersand", Qt::Key_Ampersand);
	this->add("single-quote", Qt::Key_Apostrophe);
	this->add("open-paren", Qt::Key_ParenLeft);
	this->add("close-paren", Qt::Key_ParenRight);
	this->add("asterisk", Qt::Key_Asterisk);
	this->add("plus", Qt::Key_Plus);
	this->add("comma", Qt::Key_Comma);
	this->add("minus", Qt::Key_Minus);
	this->add("period", Qt::Key_Period);
	this->add("slash", Qt::Key_Slash);
	this->add("0", Qt::Key_0);
	this->add("1", Qt::Key_1);
	this->add("2", Qt::Key_2);
	this->add("3", Qt::Key_3);
	this->add("4", Qt::Key_4);
	this->add("5", Qt::Key_5);
	this->add("6", Qt::Key_6);
	this->add("7", Qt::Key_7);
	this->add("8", Qt::Key_8);
	this->add("9", Qt::Key_9);
	this->add("colon", Qt::Key_Colon);
	this->add("semicolon", Qt::Key_Semicolon);
	this->add("less-than", Qt::Key_Less);
	this->add("equals", Qt::Key_Equal);
	this->add("greater-than", Qt::Key_Greater);
	this->add("question", Qt::Key_Question);
	this->add("at", Qt::Key_At);
	this->add("a", Qt::Key_A);
	this->add("b", Qt::Key_B);
	this->add("c", Qt::Key_C);
	this->add("d", Qt::Key_D);
	this->add("e", Qt::Key_E);
	this->add("f", Qt::Key_F);
	this->add("g", Qt::Key_G);
	this->add("h", Qt::Key_H);
	this->add("i", Qt::Key_I);
	this->add("j", Qt::Key_J);
	this->add("k", Qt::Key_K);
	this->add("l", Qt::Key_L);
	this->add("m", Qt::Key_M);
	this->add("n", Qt::Key_N);
	this->add("o", Qt::Key_O);
	this->add("p", Qt::Key_P);
	this->add("q", Qt::Key_Q);
	this->add("r", Qt::Key_R);
	this->add("s", Qt::Key_S);
	this->add("t", Qt::Key_T);
	this->add("u", Qt::Key_U);
	this->add("v", Qt::Key_V);
	this->add("w", Qt::Key_W);
	this->add("x", Qt::Key_X);
	this->add("y", Qt::Key_Y);
	this->add("z", Qt::Key_Z);
	this->add("open-bracket", Qt::Key_BracketLeft);
	this->add("backslash", Qt::Key_Backslash);
	this->add("close-bracket", Qt::Key_BracketRight);
	this->add("caret", Qt::Key_AsciiCircum);
	this->add("underscore", Qt::Key_Underscore);
	this->add("backtick", Qt::Key_QuoteLeft);
	this->add("open-brace", Qt::Key_BraceLeft);
	this->add("pipe", Qt::Key_Bar);
	this->add("close-brace", Qt::Key_BraceRight);
	this->add("tilde", Qt::Key_AsciiTilde);
	this->add("escape", Qt::Key_Escape);
	this->add("tab", Qt::Key_Tab);
	this->add("backtab", Qt::Key_Backtab);
	this->add("backspace", Qt::Key_Backspace);
	this->add("enter", Qt::Key_Return);
	this->add("numpad-enter", Qt::Key_Enter);
	this->add("insert", Qt::Key_Insert);
	this->add("delete", Qt::Key_Delete);
	this->add("pause", Qt::Key_Pause);
	this->add("print-screen", Qt::Key_Print);
	this->add("sys-req", Qt::Key_SysReq);
	this->add("clear", Qt::Key_Clear);
	this->add("home", Qt::Key_Home);
	this->add("end", Qt::Key_End);
	this->add("left", Qt::Key_Left);
	this->add("up", Qt::Key_Up);
	this->add("right", Qt::Key_Right);
	this->add("down", Qt::Key_Down);
	this->add("page-up", Qt::Key_PageUp);
	this->add("page-down", Qt::Key_PageDown);
#if defined(Q_OS_WIN)
	this->add("shift-left", Qt::Key_Shift);
	this->add("shift-right", Qt::Key_Shift, 0xA1);
	this->add("shift", Qt::Key_Shift);
	this->add("control-left", Qt::Key_Control);
	this->add("control-right", Qt::Key_Control, 0xA3);
	this->add("control", Qt::Key_Control);
	this->add("windows-left", Qt::Key_Meta);
	this->add("windows-right", Qt::Key_Meta, 0x5C);
	this->add("windows", Qt::Key_Meta);
	this->add("alt-left", Qt::Key_Alt);
	this->add("alt-right", Qt::Key_Alt, 0xA5);
	this->add("alt", Qt::Key_Alt);
#elif defined(Q_OS_MACOS)
	this->add("shift-left", Qt::Key_Shift);
	this->add("shift-right", Qt::Key_Shift, 0x3C);
	this->add("shift", Qt::Key_Shift);
	this->add("command-left", Qt::Key_Control);
	this->add("command-right", Qt::Key_Control, 0x36);
	this->add("command", Qt::Key_Control);
	this->add("control-left", Qt::Key_Meta);
	this->add("control-right", Qt::Key_Meta, 0x3E);
	this->add("control", Qt::Key_Meta);
	this->add("option-left", Qt::Key_Alt);
	this->add("option-right", Qt::Key_Alt, 0x3D);
	this->add("option", Qt::Key_Alt);
#elif defined(Q_OS_UNIX)
	this->add("shift-left", Qt::Key_Shift);
	this->add("shift-right", Qt::Key_Shift, 0xFFE2);
	this->add("shift", Qt::Key_Shift);
	this->add("control-left", Qt::Key_Control);
	this->add("control-right", Qt::Key_Control, 0xFFE4);
	this->add("control", Qt::Key_Control);
	this->add("windows-left", Qt::Key_Meta);
	this->add("windows-right", Qt::Key_Meta, 0xFFE8);
	this->add("windows", Qt::Key_Meta);
	this->add("alt-left", Qt::Key_Alt);
	this->add("alt-right", Qt::Key_Alt, 0xFFEA);
	this->add("alt", Qt::Key_Alt);
#else
	this->add("shift", Qt::Key_Shift);
	this->add("control", Qt::Key_Control);
	this->add("windows", Qt::Key_Meta);
	this->add("alt", Qt::Key_Alt);
#endif
	this->add("caps-lock", Qt::Key_CapsLock);
	this->add("num-lock", Qt::Key_NumLock);
	this->add("scroll-lock", Qt::Key_ScrollLock);
	this->add("f1", Qt::Key_F1);
	this->add("f2", Qt::Key_F2);
	this->add("f3", Qt::Key_F3);
	this->add("f4", Qt::Key_F4);
	this->add("f5", Qt::Key_F5);
	this->add("f6", Qt::Key_F6);
	this->add("f7", Qt::Key_F7);
	this->add("f8", Qt::Key_F8);
	this->add("f9", Qt::Key_F9);
	this->add("f10", Qt::Key_F10);
	this->add("f11", Qt::Key_F11);
	this->add("f12", Qt::Key_F12);
	this->add("f13", Qt::Key_F13);
	this->add("f14", Qt::Key_F14);
	this->add("f15", Qt::Key_F15);
	this->add("f16", Qt::Key_F16);
	this->add("f17", Qt::Key_F17);
	this->add("f18", Qt::Key_F18);
	this->add("f19", Qt::Key_F19);
	this->add("f20", Qt::Key_F20);
	this->add("f21", Qt::Key_F21);
	this->add("f22", Qt::Key_F22);
	this->add("f23", Qt::Key_F23);
	this->add("f24", Qt::Key_F24);
	this->add("f25", Qt::Key_F25);
	this->add("f26", Qt::Key_F26);
	this->add("f27", Qt::Key_F27);
	this->add("f28", Qt::Key_F28);
	this->add("f29", Qt::Key_F29);
	this->add("f30", Qt::Key_F30);
	this->add("f31", Qt::Key_F31);
	this->add("f32", Qt::Key_F32);
	this->add("f33", Qt::Key_F33);
	this->add("f34", Qt::Key_F34);
	this->add("f35", Qt::Key_F35);
	this->add("super-left", Qt::Key_Super_L);
	this->add("super-right", Qt::Key_Super_R);
	this->add("super", Qt::Key_Super_L);
	this->add("windows-menu", Qt::Key_Menu);
	this->add("hyper-left", Qt::Key_Hyper_L);
	this->add("hyper-right", Qt::Key_Hyper_R);
	this->add("hyper", Qt::Key_Hyper_L);
	this->add("help", Qt::Key_Help);
	this->add("direction-left", Qt::Key_Direction_L);
	this->add("direction-right", Qt::Key_Direction_R);
}

int MidiUtilKeyCodes::getKeyCodeFromEvent(QKeyEvent* event)
{
	int qt_key_code = event->key();
	quint32 native_virtual_key = event->nativeVirtualKey();
	qDebug("qt_key_code = %d, native_virtual_key = %d, native_scan_code = %d", qt_key_code, native_virtual_key, event->nativeScanCode());
	int key_code = this->qt_key_code_and_native_virtual_key_to_key_code_map.value(QPair<int, quint32>(qt_key_code, native_virtual_key), -1);
	if (key_code < 0) key_code = this->qt_key_code_and_native_virtual_key_to_key_code_map.value(QPair<int, quint32>(qt_key_code, 0), -1);
	return key_code;
}

int MidiUtilKeyCodes::getKeyCodeFromName(QString name)
{
	return this->name_to_key_code_map.value(name, -1);
}

QString MidiUtilKeyCodes::getNameFromKeyCode(int number)
{
	return this->names[number];
}

void MidiUtilKeyCodes::add(QString name, int qt_key_code, quint32 native_virtual_key)
{
	QPair<int, quint32> qt_key_code_and_native_virtual_key = QPair<int, quint32>(qt_key_code, native_virtual_key);
	int key_code = this->qt_key_code_and_native_virtual_key_to_key_code_map.value(qt_key_code_and_native_virtual_key, -1);

	if (key_code < 0)
	{
		key_code = (this->next_key_code)++;
		this->qt_key_code_and_native_virtual_key_to_key_code_map.insert(qt_key_code_and_native_virtual_key, key_code);
		this->names[key_code] = name;
	}

	this->name_to_key_code_map.insert(name, key_code);
}

