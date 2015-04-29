
package com.sreal.midifile;

public class MeasureBeat
{
	static { java.lang.System.loadLibrary("midifilejni"); }

	private byte[] pointer;

	public MeasureBeat() { this.init(); }
	private native void init();
	public native void finalize();
	public native long getMeasure();
	public native void setMeasure(long measure);
	public native float getBeat();
	public native void setBeat(float beat);
	public native java.lang.String toString();
	public native void parse(java.lang.String string);
}

