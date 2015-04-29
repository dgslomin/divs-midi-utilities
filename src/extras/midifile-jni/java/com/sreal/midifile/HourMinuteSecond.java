
package com.sreal.midifile;

public class HourMinuteSecond
{
	static { java.lang.System.loadLibrary("midifilejni"); }

	private byte[] pointer;

	public HourMinuteSecond() { this.init(); }
	private native void init();
	public native void finalize();
	public native long getHour();
	public native void setHour(long hour);
	public native long getMinute();
	public native void setMinute(long minute);
	public native float getSecond();
	public native void setSecond(float second);
	public native java.lang.String toString();
	public native void parse(java.lang.String string);
}

