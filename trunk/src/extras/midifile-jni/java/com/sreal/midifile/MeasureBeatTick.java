
package com.sreal.midifile;

public class MeasureBeatTick
{
	static { java.lang.System.loadLibrary("midifilejni"); }

	private byte[] pointer;

	public MeasureBeatTick() { this.init(); }
	private native void init();
	public native void finalize();
	public native long getMeasure();
	public native void setMeasure(long measure);
	public native long getBeat();
	public native void setBeat(long beat);
	public native float getTick();
	public native void setTick(float tick);
	public native java.lang.String toString();
	public native void parse(java.lang.String string);
}

