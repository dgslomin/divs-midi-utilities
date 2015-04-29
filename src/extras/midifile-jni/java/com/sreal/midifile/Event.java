
package com.sreal.midifile;

public class Event
{
	static { java.lang.System.loadLibrary("midifilejni"); }

	private byte[] pointer;

	private Event() {}
	public native boolean equals(java.lang.Object other);
	public native void delete();
	public native com.sreal.midifile.Track getTrack();
	public native com.sreal.midifile.Event getPreviousEventInTrack();
	public native com.sreal.midifile.Event getNextEventInTrack();
	public native com.sreal.midifile.Event getPreviousEventInFile();
	public native com.sreal.midifile.Event getNextEventInFile();
	public native long getTick();
	public native void setTick(long tick);
	public native int getType();
	public native boolean isNoteEvent();
	public native boolean isNoteStartEvent();
	public native boolean isNoteEndEvent();
	public native boolean isTempoEvent();
	public native boolean isTimeSignatureEvent();
	public native boolean isLyricEvent();
	public native boolean isMarkerEvent();
	public native boolean isVoiceEvent();

	public native int getNoteOffEventChannel();
	public native void setNoteOffEventChannel(int channel);
	public native int getNoteOffEventNote();
	public native void setNoteOffEventNote(int note);
	public native int getNoteOffEventVelocity();
	public native void setNoteOffEventVelocity(int velocity);

	public native int getNoteOnEventChannel();
	public native void setNoteOnEventChannel(int channel);
	public native int getNoteOnEventNote();
	public native void setNoteOnEventNote(int note);
	public native int getNoteOnEventVelocity();
	public native void setNoteOnEventVelocity(int velocity);

	public native int getKeyPressureEventChannel();
	public native void setKeyPressureEventChannel(int channel);
	public native int getKeyPressureEventNote();
	public native void setKeyPressureEventNote(int note);
	public native int getKeyPressureEventAmount();
	public native void setKeyPressureEventAmount(int amount);

	public native int getControlChangeEventChannel();
	public native void setControlChangeEventChannel(int channel);
	public native int getControlChangeEventNumber();
	public native void setControlChangeEventNumber(int number);
	public native int getControlChangeEventValue();
	public native void setControlChangeEventValue(int value);

	public native int getProgramChangeEventChannel();
	public native void setProgramChangeEventChannel(int channel);
	public native int getProgramChangeEventNumber();
	public native void setProgramChangeEventNumber(int number);

	public native int getChannelPressureEventChannel();
	public native void setChannelPressureEventChannel(int channel);
	public native int getChannelPressureEventAmount();
	public native void setChannelPressureEventAmount(int amount);

	public native int getPitchWheelEventChannel();
	public native void setPitchWheelEventChannel(int channel);
	public native int getPitchWheelEventValue();
	public native void setPitchWheelEventValue(int value);

	public native byte[] getSysexEventData();
	public native void setSysexEventData(byte[] data_buffer);

	public native int getMetaEventNumber();
	public native void setMetaEventNumber(int number);
	public native byte[] getMetaEventData();
	public native void setMetaEventData(byte[] data_buffer);

	public native int getNoteStartEventChannel();
	public native void setNoteStartEventChannel(int channel);
	public native int getNoteStartEventNote();
	public native void setNoteStartEventNote(int note);
	public native int getNoteStartEventVelocity();
	public native void setNoteStartEventVelocity(int velocity);
	public native com.sreal.midifile.Event getNoteStartEventNoteEndEvent();

	public native int getNoteEndEventChannel();
	public native void setNoteEndEventChannel(int channel);
	public native int getNoteEndEventNote();
	public native void setNoteEndEventNote(int note);
	public native int getNoteEndEventVelocity();
	public native void setNoteEndEventVelocity(int velocity); /* caution:  will replace a note on event with a note off */
	public native com.sreal.midifile.Event getNoteEndEventNoteStartEvent();

	public native float getTempoEventTempo();
	public native void setTempoEventTempo(float tempo);

	public native int getTimeSignatureEventNumerator();
	public native int getTimeSignatureEventDenominator();
	public native void setTimeSignatureEventTimeSignature(int numerator, int denominator);

	public native java.lang.String getLyricEventText();
	public native void setLyricEventText(java.lang.String text);

	public native java.lang.String getMarkerEventText();
	public native void setMarkerEventText(java.lang.String text);

	public native byte[] getVoiceEventData();
	public native void setVoiceEventData(byte[] data_buffer);
}

