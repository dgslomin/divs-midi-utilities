
package com.sreal.midifile;

public class MidiFile
{
	static { java.lang.System.loadLibrary("midifilejni"); }

	public static final int DIVISION_TYPE_INVALID = -1;
	public static final int DIVISION_TYPE_PPQ = 0;
	public static final int DIVISION_TYPE_SMPTE24 = 1;
	public static final int DIVISION_TYPE_SMPTE25 = 2;
	public static final int DIVISION_TYPE_SMPTE30DROP = 3;
	public static final int DIVISION_TYPE_SMPTE30 = 4;

	public static final int EVENT_TYPE_INVALID = -1;
	public static final int EVENT_TYPE_NOTE_OFF = 0;
	public static final int EVENT_TYPE_NOTE_ON = 1;
	public static final int EVENT_TYPE_KEY_PRESSURE = 2;
	public static final int EVENT_TYPE_CONTROL_CHANGE = 3;
	public static final int EVENT_TYPE_PROGRAM_CHANGE = 4;
	public static final int EVENT_TYPE_CHANNEL_PRESSURE = 5;
	public static final int EVENT_TYPE_PITCH_WHEEL = 6;
	public static final int EVENT_TYPE_SYSEX = 7;
	public static final int EVENT_TYPE_META = 8;

	private byte[] pointer;

	public native boolean equals(java.lang.Object other);

	public static native com.sreal.midifile.MidiFile load(java.lang.String filename) throws java.io.IOException;
	public native void save(java.lang.String filename) throws java.io.IOException;

	public MidiFile(int file_format, int division_type, int resolution) { this.init(file_format, division_type, resolution); }
	private native void init(int file_format, int division_type, int resolution);
	/** not finalize(), because there can be multiple Java object instances which share the same underlying C data structure */
	public native void free();

	public native int getFileFormat();
	public native void setFileFormat(int file_format);
	public native int getDivisionType();
	public native void setDivisionType(int division_type);
	public native int getResolution();
	public native void setResolution(int resolution);
	public native int getNumberOfTicksPerBeat();
	public native void setNumberOfTicksPerBeat(int number_of_ticks_per_beat);
	public native float getNumberOfFramesPerSecond();
	public native void setNumberOfFramesPerSecond(float number_of_frames_per_second);

	public native com.sreal.midifile.Track createTrack();
	public native int getNumberOfTracks();
	public native com.sreal.midifile.Track getTrackByNumber(int number, boolean create);
	public native com.sreal.midifile.Track getFirstTrack();
	public native com.sreal.midifile.Track getLastTrack();
	public native com.sreal.midifile.Event getFirstEvent();
	public native com.sreal.midifile.Event getLastEvent();
	public native void visitEvents(com.sreal.midifile.EventVisitor event_visitor);

	public native float getBeatFromTick(long tick);
	public native long getTickFromBeat(float beat);
	public native float getTimeFromTick(long tick); /* time is in seconds */
	public native long getTickFromTime(float time);
	public native void setMeasureBeatFromTick(long tick, com.sreal.midifile.MeasureBeat measure_beat);
	public native long getTickFromMeasureBeat(com.sreal.midifile.MeasureBeat measure_beat);
	public native void setMeasureBeatTickFromTick(long tick, com.sreal.midifile.MeasureBeatTick measure_beat_tick);
	public native long getTickFromMeasureBeatTick(com.sreal.midifile.MeasureBeatTick measure_beat_tick);
	public native void setHourMinuteSecondFromTick(long tick, com.sreal.midifile.HourMinuteSecond hour_minute_second);
	public native long getTickFromHourMinuteSecond(com.sreal.midifile.HourMinuteSecond hour_minute_second);
	public native void setHourMinuteSecondFrameFromTick(long tick, com.sreal.midifile.HourMinuteSecondFrame hour_minute_second_frame);
	public native long getTickFromHourMinuteSecondFrame(com.sreal.midifile.HourMinuteSecondFrame hour_minute_second_frame);
	public native java.lang.String getMeasureBeatStringFromTick(long tick);
	public native long getTickFromMeasureBeatString(java.lang.String measure_beat_string);
	public native java.lang.String getMeasureBeatTickStringFromTick(long tick);
	public native long getTickFromMeasureBeatTickString(java.lang.String measure_beat_tick_string);
	public native java.lang.String getHourMinuteSecondStringFromTick(long tick);
	public native long getTickFromHourMinuteSecondString(java.lang.String hour_minute_second_string);
	public native java.lang.String getHourMinuteSecondFrameStringFromTick(long tick);
	public native long getTickFromHourMinuteSecondFrameString(java.lang.String hour_minute_second_frame_string);
	public native long getTickFromMarker(java.lang.String marker);
}

