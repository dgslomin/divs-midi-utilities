
package com.sreal.midifile;

public class Track
{
	static { java.lang.System.loadLibrary("midifilejni"); }

	private byte[] pointer;

	private Track() {}
	public native boolean equals(java.lang.Object other);
	public native void delete();
	public native com.sreal.midifile.MidiFile getMidiFile();
	public native int getNumber();
	public native long getEndTick();
	public native void setEndTick(long end_tick);
	public native com.sreal.midifile.Track createTrackBefore();
	public native com.sreal.midifile.Track getPreviousTrack();
	public native com.sreal.midifile.Track getNextTrack();
	public native com.sreal.midifile.Event createNoteOffEvent(long tick, int channel, int note, int velocity);
	public native com.sreal.midifile.Event createNoteOnEvent(long tick, int channel, int note, int velocity);
	public native com.sreal.midifile.Event createKeyPressureEvent(long tick, int channel, int note, int amount);
	public native com.sreal.midifile.Event createControlChangeEvent(long tick, int channel, int number, int value);
	public native com.sreal.midifile.Event createProgramChangeEvent(long tick, int channel, int number);
	public native com.sreal.midifile.Event createChannelPressureEvent(long tick, int channel, int amount);
	public native com.sreal.midifile.Event createPitchWheelEvent(long tick, int channel, int value);
	public native com.sreal.midifile.Event createSysexEvent(long tick, byte[] data_buffer);
	public native com.sreal.midifile.Event createMetaEvent(long tick, int number, byte[] data_buffer);
	public native com.sreal.midifile.Event createNoteStartAndEndEvents(long start_tick, long end_tick, int channel, int note, int start_velocity, int end_velocity); /* returns the start event */
	public native com.sreal.midifile.Event createTempoEvent(long tick, float tempo); /* tempo is in BPM */
	public native com.sreal.midifile.Event createTimeSignatureEvent(long tick, int numerator, int denominator);
	public native com.sreal.midifile.Event createLyricEvent(long tick, java.lang.String text);
	public native com.sreal.midifile.Event createMarkerEvent(long tick, java.lang.String text);
	public native com.sreal.midifile.Event createVoiceEvent(long tick, byte[] data_buffer);
	public native com.sreal.midifile.Event getFirstEvent();
	public native com.sreal.midifile.Event getLastEvent();
	public native void visitEvents(com.sreal.midifile.EventVisitor event_visitor);
}

