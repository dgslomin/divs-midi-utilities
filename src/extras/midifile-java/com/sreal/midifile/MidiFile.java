
package com.sreal.midifile

public class MidiFile
{
	public static int DIVISION_TYPE_PPQ = 0;
	public static int DIVISION_TYPE_SMPTE24 = 1;
	public static int DIVISION_TYPE_SMPTE25 = 2;
	public static int DIVISION_TYPE_SMPTE30DROP = 3;
	public static int DIVISION_TYPE_SMPTE30 = 4;

	protected int file_format;
	protected int division_type;
	protected int resolution;
	protected int number_of_ticks_per_beat;
	protected float number_of_frames_per_second;
	protected int number_of_tracks;
	protected com.sreal.midifile.Track first_track;
	protected java.lang.ref.WeakReference last_track;
	protected com.sreal.midifile.Event first_event;
	protected java.lang.ref.WeakReference last_event;

	public MidiFile(int file_format, int division_type, int resolution)
	{
		this.file_format = file_format;
		this.division_type = division_type;
		this.resolution = resolution;

		if (this.division_type == com.sreal.midifile.MidiFile.DIVISION_TYPE_PPQ)
		{
			this.number_of_ticks_per_beat = resolution;
			this.number_of_frames_per_second = 30.0;
		}
		else if (this.division_type == com.sreal.midifile.MidiFile.DIVISION_TYPE_SMPTE24)
		{
			this.number_of_ticks_per_beat = 960;
			this.number_of_frames_per_second = 24.0;
		}
		else if (this.division_type == com.sreal.midifile.MidiFile.DIVISION_TYPE_SMPTE25)
		{
			this.number_of_ticks_per_beat = 960;
			this.number_of_frames_per_second = 25.0;
		}
		else if (this.division_type == com.sreal.midifile.MidiFile.DIVISION_TYPE_SMPTE30DROP)
		{
			this.number_of_ticks_per_beat = 960;
			this.number_of_frames_per_second = 29.97;
		}
		else if (this.division_type == com.sreal.midifile.MidiFile.DIVISION_TYPE_SMPTE30)
		{
			this.number_of_ticks_per_beat = 960;
			this.number_of_frames_per_second = 30.0;
		}
	}

	public MidiFile(java.lang.String filename) throws java.io.IOException
	{
		this(new java.io.FileInputStream(filename));
	}

	public MidiFile(java.io.InputStream input_stream)
	{
		byte[] chunk_id = new byte[4];
		int amount_read = 0;

		input_stream.read(chunk_id);
		amount_read += 4;

		if ((chunk_id[0] == 0x52) && (chunk_id[1] == 0x49) && (chunk_id[2] == 0x46) && (chunk_id[3] == 0x46))
		{
			input_stream.read(chunk_id);
			amount_read += 4;
			if (chunk_id[0] != 0x52) || (chunk_id[1] != 0x4D) || (chunk_id[2] != 0x49) || (chunk_id[3] != 0x44)) throw new com.sreal.midifile.Exception();

			input_stream.read(chunk_id);
			amount_read += 4;
			if (chunk_id[0] != 0x64) || (chunk_id[1] != 0x61) || (chunk_id[2] != 0x74) || (chunk_id[3] != 0x61)) throw new com.sreal.midifile.Exception();
		}
	}

	class MidiFileInputStream implements java.io.InputStream
	{
		java.io.InputStream input_stream;

		MidiFileInputStream(java.io.InputStream input_stream)
		{
			this.input_stream = input_stream;
		}

		public int available() throws java.io.IOException
		{
			return this.input_stream.available();
		}

		public void close() throws java.io.IOException
		{
			this.input_stream.close();
		}

		public void mark(int readlimit)
		{
			this.input_stream.mark(readlimit);
		}

		public boolean markSupported()
		{
			return this.input_stream.markSupported();
		}

		public int read() throws java.io.IOException
		{
			return this.input_stream.read();
		}

		public int read(byte[] b) throws java.io.IOException
		{
			return this.input_stream.read(b);
		}

		public int read(byte[] b, int off, int len) throws java.io.IOException
		{
			return this.input_stream.read(b, off, len);
		}

		public void reset() throws java.io.IOException
		{
			this.input_stream.reset();
		}

		public long skip(long n) throws java.io.IOException
		{
			return this.input_stream.skip(n);
		}

		byte[] readFourCC() throws java.io.IOException
		{
			byte[] four_cc = new byte[4];
			return this.read(four_cc);
		}

		static int interpretInt16(byte[] b)
		{
			return (int)(b[0]) << 8) | (int)(b[1]);
		}

		int readInt16() throws java.io.IOException
		{
			byte[] b = new byte[4];
			this.read(b);
			return com.sreal.midifile.MidiFile.MidiFileInputStream.interpretInt16(b);
		}
	}

	public static void writeMidiFile(java.io.OutputStream output_stream, com.sreal.midifile.MidiFile midi_file) throws java.io.IOException
	{
	}

	public static void writeMidiFileHeader(java.io.OutputStream output_stream, int format, int number_of_tracks, int division_type, int resolution) throws java.io.IOException
	{
	}

	public static void writeTrack(java.io.OutputStream output_stream, com.sreal.midifile.Track track) throws java.io.IOException
	{
	}

	public static int getTrackSize(com.sreal.midifile.Track track)
	{
	}

	public static void writeTrackHeader(java.io.OutputStream output_stream, int track_size) throws java.io.IOException
	{
	}

	public static int getEventSize(com.sreal.midifile.Event event)
	{
	}

	public static void writeEvent(java.io.OutputStream output_stream, com.sreal.midifile.Event event) throws java.io.IOException
	{
	}

	public static void writeUInt16(java.io.OutputStream output_stream, int value) throws java.io.IOException
	{
	}

	public static void writeUInt32(java.io.OutputStream output_stream, int value) throws java.io.IOException
	{
	}

	public static void writeVariableWidthInt(java.io.OutputStream output_stream, long value) throws java.io.IOException
	{	
	}
}

