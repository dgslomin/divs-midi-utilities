
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <midifile.h>

static void jobject_setPointer(JNIEnv *env, jobject jobject_, void *pointer)
{
	if (jobject_ != NULL)
	{
		jclass jclass_ = (*env)->GetObjectClass(env, jobject_);
		jfieldID jfieldID_ = (*env)->GetFieldID(env, jclass_, "pointer", "[B");
		jbyteArray jbyteArray_ = (*env)->NewByteArray(env, sizeof(void *));
		(*env)->SetByteArrayRegion(env, jbyteArray_, 0, sizeof(void *), (jbyte *)(&pointer));
		(*env)->SetObjectField(env, jobject_, jfieldID_, jbyteArray_);
	}
}

static void *jobject_getPointer(JNIEnv *env, jobject jobject_)
{
	if (jobject_ == NULL)
	{
		return NULL;
	}
	else
	{
		jclass jclass_ = (*env)->GetObjectClass(env, jobject_);
		jfieldID jfieldID_ = (*env)->GetFieldID(env, jclass_, "pointer", "[B");
		jbyteArray jbyteArray_ = (*env)->GetObjectField(env, jobject_, jfieldID_);
		void *pointer = NULL;
		(*env)->GetByteArrayRegion(env, jbyteArray_, 0, sizeof(void *), (jbyte *)(&pointer));
		return pointer;
	}
}

static jobject jobject_newWithPointer(JNIEnv *env, jclass jclass_, char *class_name, void *pointer)
{
	if (pointer == NULL)
	{
		return NULL;
	}
	else
	{
		if (jclass_ == NULL) jclass_ = (*env)->FindClass(env, class_name);
		jobject jobject_ = (*env)->AllocObject(env, jclass_);
		jobject_setPointer(env, jobject_, pointer);
		return jobject_;
	}
}

struct EventVisitorData
{
	JNIEnv *env;
	jobject jobject_event_visitor;
	jmethodID jmethodID_visitEvent;
	jobject jobject_event;
};

static void visitEvent(MidiFileEvent_t event, void *user_data)
{
	struct EventVisitorData *event_visitor_data = (struct EventVisitorData *)(user_data);
	jobject_setPointer(event_visitor_data->env, event_visitor_data->jobject_event, event);
	(*(event_visitor_data->env))->CallVoidMethod(event_visitor_data->env, event_visitor_data->jobject_event_visitor, event_visitor_data->jmethodID_visitEvent, event_visitor_data->jobject_event);
}

static jboolean JNICALL Java_com_sreal_midifile_MidiFile_equals(JNIEnv *env, jobject jobject_midi_file, jobject jobject_other)
{
	jclass jclass_midi_file = (*env)->FindClass(env, "com/sreal/midifile/MidiFile");

	if ((*env)->IsInstanceOf(env, jobject_other, jclass_midi_file))
	{
		MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
		MidiFile_t other = jobject_getPointer(env, jobject_other);
		return (midi_file == other);
	}
	else
	{
		return 0;
	}
}

static jobject JNICALL Java_com_sreal_midifile_MidiFile_load(JNIEnv *env, jclass jclass_midi_file, jstring jstring_filename)
{
	const char *filename = (*env)->GetStringUTFChars(env, jstring_filename, NULL);
	MidiFile_t midi_file = MidiFile_load((char *)(filename));
	(*env)->ReleaseStringUTFChars(env, jstring_filename, filename);

	if (midi_file == NULL)
	{
		jclass jclass_exception = (*env)->FindClass(env, "java/io/IOException");
		(*env)->ThrowNew(env, jclass_exception, "Cannot load file.");
		return NULL;
	}
	else
	{
		return jobject_newWithPointer(env, jclass_midi_file, NULL, midi_file);
	}
}

static void JNICALL Java_com_sreal_midifile_MidiFile_save(JNIEnv *env, jobject jobject_midi_file, jstring jstring_filename)
{
	const char *filename = (*env)->GetStringUTFChars(env, jstring_filename, NULL);
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);

	if (MidiFile_save(midi_file, (char *)(filename)) < 0)
	{
		jclass jclass_exception = (*env)->FindClass(env, "java/io/IOException");
		(*env)->ThrowNew(env, jclass_exception, "Cannot save file.");
	}

	(*env)->ReleaseStringUTFChars(env, jstring_filename, filename);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_init(JNIEnv *env, jobject jobject_midi_file, jint file_format, jint division_type, jint resolution)
{
	MidiFile_t midi_file = MidiFile_new(file_format, division_type, resolution);
	jobject_setPointer(env, jobject_midi_file, midi_file);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_free(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFile_free(midi_file);
}

static jint JNICALL Java_com_sreal_midifile_MidiFile_getFileFormat(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getFileFormat(midi_file);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setFileFormat(JNIEnv *env, jobject jobject_midi_file, jint file_format)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFile_setFileFormat(midi_file, file_format);
}

static jint JNICALL Java_com_sreal_midifile_MidiFile_getDivisionType(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getDivisionType(midi_file);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setDivisionType(JNIEnv *env, jobject jobject_midi_file, jint division_type)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFile_setDivisionType(midi_file, division_type);
}

static jint JNICALL Java_com_sreal_midifile_MidiFile_getResolution(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getResolution(midi_file);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setResolution(JNIEnv *env, jobject jobject_midi_file, jint resolution)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFile_setResolution(midi_file, resolution);
}

static jint JNICALL Java_com_sreal_midifile_MidiFile_getNumberOfTicksPerBeat(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getNumberOfTicksPerBeat(midi_file);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setNumberOfTicksPerBeat(JNIEnv *env, jobject jobject_midi_file, jint number_of_ticks_per_beat)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFile_setResolution(midi_file, number_of_ticks_per_beat);
}

static jfloat JNICALL Java_com_sreal_midifile_MidiFile_getNumberOfFramesPerSecond(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getNumberOfFramesPerSecond(midi_file);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setNumberOfFramesPerSecond(JNIEnv *env, jobject jobject_midi_file, jfloat number_of_frames_per_second)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFile_setNumberOfFramesPerSecond(midi_file, number_of_frames_per_second);
}

static jobject JNICALL Java_com_sreal_midifile_MidiFile_createTrack(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileTrack_t track = MidiFile_createTrack(midi_file);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Track", track);
}

static jint JNICALL Java_com_sreal_midifile_MidiFile_getNumberOfTracks(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getNumberOfTracks(midi_file);
}

static jobject JNICALL Java_com_sreal_midifile_MidiFile_getTrackByNumber(JNIEnv *env, jobject jobject_midi_file, jint number, jboolean create)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileTrack_t track = MidiFile_getTrackByNumber(midi_file, number, create);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Track", track);
}

static jobject JNICALL Java_com_sreal_midifile_MidiFile_getFirstTrack(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileTrack_t track = MidiFile_getFirstTrack(midi_file);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Track", track);
}

static jobject JNICALL Java_com_sreal_midifile_MidiFile_getLastTrack(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileTrack_t track = MidiFile_getLastTrack(midi_file);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Track", track);
}

static jobject JNICALL Java_com_sreal_midifile_MidiFile_getFirstEvent(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_MidiFile_getLastEvent(JNIEnv *env, jobject jobject_midi_file)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileEvent_t event = MidiFile_getLastEvent(midi_file);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_visitEvents(JNIEnv *env, jobject jobject_midi_file, jobject jobject_event_visitor)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	jclass jclass_event = (*env)->FindClass(env, "com/sreal/midifile/Event");
	jclass jclass_event_visitor = (*env)->GetObjectClass(env, jobject_event_visitor);
	jmethodID jmethodID_visitEvent = (*env)->GetMethodID(env, jclass_event_visitor, "visitEvent", "(Lcom/sreal/midifile/Event;)V");
	jobject jobject_event = (*env)->AllocObject(env, jclass_event);
	struct EventVisitorData event_visitor_data;
	event_visitor_data.env = env;
	event_visitor_data.jobject_event_visitor = jobject_event_visitor;
	event_visitor_data.jmethodID_visitEvent = jmethodID_visitEvent;
	event_visitor_data.jobject_event = jobject_event;
	MidiFile_visitEvents(midi_file, visitEvent, &event_visitor_data);
	(*env)->DeleteLocalRef(env, jobject_event);
}

static jfloat JNICALL Java_com_sreal_midifile_MidiFile_getBeatFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getBeatFromTick(midi_file, tick);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromBeat(JNIEnv *env, jobject jobject_midi_file, jfloat beat)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getTickFromBeat(midi_file, beat);
}

static jfloat JNICALL Java_com_sreal_midifile_MidiFile_getTimeFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getTimeFromTick(midi_file, tick);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromTime(JNIEnv *env, jobject jobject_midi_file, jfloat time)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	return MidiFile_getTickFromTime(midi_file, time);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setMeasureBeatFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick, jobject jobject_measure_beat)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	MidiFile_setMeasureBeatFromTick(midi_file, tick, measure_beat);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromMeasureBeat(JNIEnv *env, jobject jobject_midi_file, jobject jobject_measure_beat)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	return MidiFile_getTickFromMeasureBeat(midi_file, measure_beat);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setMeasureBeatTickFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick, jobject jobject_measure_beat_tick)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	MidiFile_setMeasureBeatTickFromTick(midi_file, tick, measure_beat_tick);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromMeasureBeatTick(JNIEnv *env, jobject jobject_midi_file, jobject jobject_measure_beat_tick)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	return MidiFile_getTickFromMeasureBeatTick(midi_file, measure_beat_tick);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setHourMinuteSecondFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick, jobject jobject_hour_minute_second)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	MidiFile_setHourMinuteSecondFromTick(midi_file, tick, hour_minute_second);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromHourMinuteSecond(JNIEnv *env, jobject jobject_midi_file, jobject jobject_hour_minute_second)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	return MidiFile_getTickFromHourMinuteSecond(midi_file, hour_minute_second);
}

static void JNICALL Java_com_sreal_midifile_MidiFile_setHourMinuteSecondFrameFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick, jobject jobject_hour_minute_second_frame)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	MidiFile_setHourMinuteSecondFrameFromTick(midi_file, tick, hour_minute_second_frame);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromHourMinuteSecondFrame(JNIEnv *env, jobject jobject_midi_file, jobject jobject_hour_minute_second_frame)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	return MidiFile_getTickFromHourMinuteSecondFrame(midi_file, hour_minute_second_frame);
}

static jstring JNICALL Java_com_sreal_midifile_MidiFile_getMeasureBeatStringFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	char *measure_beat_string = MidiFile_getMeasureBeatStringFromTick(midi_file, tick);
	return (*env)->NewStringUTF(env, measure_beat_string);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromMeasureBeatString(JNIEnv *env, jobject jobject_midi_file, jstring jstring_measure_beat_string)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	const char *measure_beat_string = (*env)->GetStringUTFChars(env, jstring_measure_beat_string, NULL);
	long tick = MidiFile_getTickFromMeasureBeatString(midi_file, (char *)(measure_beat_string));
	(*env)->ReleaseStringUTFChars(env, jstring_measure_beat_string, measure_beat_string);
	return tick;
}

static jstring JNICALL Java_com_sreal_midifile_MidiFile_getMeasureBeatTickStringFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	char *measure_beat_tick_string = MidiFile_getMeasureBeatTickStringFromTick(midi_file, tick);
	return (*env)->NewStringUTF(env, measure_beat_tick_string);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromMeasureBeatTickString(JNIEnv *env, jobject jobject_midi_file, jstring jstring_measure_beat_tick_string)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	const char *measure_beat_tick_string = (*env)->GetStringUTFChars(env, jstring_measure_beat_tick_string, NULL);
	long tick = MidiFile_getTickFromMeasureBeatTickString(midi_file, (char *)(measure_beat_tick_string));
	(*env)->ReleaseStringUTFChars(env, jstring_measure_beat_tick_string, measure_beat_tick_string);
	return tick;
}

static jstring JNICALL Java_com_sreal_midifile_MidiFile_getHourMinuteSecondStringFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	char *hour_minute_second_string = MidiFile_getHourMinuteSecondStringFromTick(midi_file, tick);
	return (*env)->NewStringUTF(env, hour_minute_second_string);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromHourMinuteSecondString(JNIEnv *env, jobject jobject_midi_file, jstring jstring_hour_minute_second_string)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	const char *hour_minute_second_string = (*env)->GetStringUTFChars(env, jstring_hour_minute_second_string, NULL);
	long tick = MidiFile_getTickFromHourMinuteSecondString(midi_file, (char *)(hour_minute_second_string));
	(*env)->ReleaseStringUTFChars(env, jstring_hour_minute_second_string, hour_minute_second_string);
	return tick;
}

static jstring JNICALL Java_com_sreal_midifile_MidiFile_getHourMinuteSecondFrameStringFromTick(JNIEnv *env, jobject jobject_midi_file, jlong tick)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	char *hour_minute_second_frame_string = MidiFile_getHourMinuteSecondFrameStringFromTick(midi_file, tick);
	return (*env)->NewStringUTF(env, hour_minute_second_frame_string);
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromHourMinuteSecondFrameString(JNIEnv *env, jobject jobject_midi_file, jstring jstring_hour_minute_second_frame_string)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	const char *hour_minute_second_frame_string = (*env)->GetStringUTFChars(env, jstring_hour_minute_second_frame_string, NULL);
	long tick = MidiFile_getTickFromHourMinuteSecondFrameString(midi_file, (char *)(hour_minute_second_frame_string));
	(*env)->ReleaseStringUTFChars(env, jstring_hour_minute_second_frame_string, hour_minute_second_frame_string);
	return tick;
}

static jlong JNICALL Java_com_sreal_midifile_MidiFile_getTickFromMarker(JNIEnv *env, jobject jobject_midi_file, jstring jstring_marker)
{
	MidiFile_t midi_file = jobject_getPointer(env, jobject_midi_file);
	const char *marker = (*env)->GetStringUTFChars(env, jstring_marker, NULL);
	long tick = MidiFile_getTickFromMarker(midi_file, (char *)(marker));
	(*env)->ReleaseStringUTFChars(env, jstring_marker, marker);
	return tick;
}

static jboolean JNICALL Java_com_sreal_midifile_Track_equals(JNIEnv *env, jobject jobject_track, jobject jobject_other)
{
	jclass jclass_track = (*env)->FindClass(env, "com/sreal/midifile/Track");

	if ((*env)->IsInstanceOf(env, jobject_other, jclass_track))
	{
		MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
		MidiFileTrack_t other = jobject_getPointer(env, jobject_other);
		return (track == other);
	}
	else
	{
		return 0;
	}
}

static void JNICALL Java_com_sreal_midifile_Track_delete(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileTrack_delete(track);
}

static jobject JNICALL Java_com_sreal_midifile_Track_getMidiFile(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFile_t midi_file = MidiFileTrack_getMidiFile(track);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/MidiFile", midi_file);
}

static jint JNICALL Java_com_sreal_midifile_Track_getNumber(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	return MidiFileTrack_getNumber(track);
}

static jlong JNICALL Java_com_sreal_midifile_Track_getEndTick(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	return MidiFileTrack_getEndTick(track);
}

static void JNICALL Java_com_sreal_midifile_Track_setEndTick(JNIEnv *env, jobject jobject_track, jlong end_tick)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileTrack_setEndTick(track, end_tick);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createTrackBefore(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileTrack_t other = MidiFileTrack_createTrackBefore(track);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Track", other);
}

static jobject JNICALL Java_com_sreal_midifile_Track_getPreviousTrack(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileTrack_t other = MidiFileTrack_getPreviousTrack(track);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Track", other);
}

static jobject JNICALL Java_com_sreal_midifile_Track_getNextTrack(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileTrack_t other = MidiFileTrack_getNextTrack(track);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Track", other);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createNoteOffEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint channel, jint note, jint velocity)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createNoteOffEvent(track, tick, channel, note, velocity);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createNoteOnEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint channel, jint note, jint velocity)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createNoteOnEvent(track, tick, channel, note, velocity);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createKeyPressureEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint channel, jint note, jint amount)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createKeyPressureEvent(track, tick, channel, note, amount);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createControlChangeEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint channel, jint number, jint value)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createControlChangeEvent(track, tick, channel, number, value);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createProgramChangeEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint channel, jint number)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createProgramChangeEvent(track, tick, channel, number);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createChannelPressureEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint channel, jint amount)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createChannelPressureEvent(track, tick, channel, amount);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createPitchWheelEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint channel, jint value)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createPitchWheelEvent(track, tick, channel, value);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createSysexEvent(JNIEnv *env, jobject jobject_track, jlong tick, jbyteArray jbyteArray_data_buffer)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	int data_length = (*env)->GetArrayLength(env, jbyteArray_data_buffer);
	jbyte *data_buffer = (*env)->GetByteArrayElements(env, jbyteArray_data_buffer, NULL);
	MidiFileEvent_t event = MidiFileTrack_createSysexEvent(track, tick, data_length, data_buffer);
	(*env)->ReleaseByteArrayElements(env, jbyteArray_data_buffer, data_buffer, JNI_ABORT);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createMetaEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint number, jbyteArray jbyteArray_data_buffer)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	int data_length = (*env)->GetArrayLength(env, jbyteArray_data_buffer);
	jbyte *data_buffer = (*env)->GetByteArrayElements(env, jbyteArray_data_buffer, NULL);
	MidiFileEvent_t event = MidiFileTrack_createMetaEvent(track, tick, number, data_length, data_buffer);
	(*env)->ReleaseByteArrayElements(env, jbyteArray_data_buffer, data_buffer, JNI_ABORT);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createNoteStartAndEndEvents(JNIEnv *env, jobject jobject_track, jlong start_tick, jlong end_tick, jint channel, jint note, jint start_velocity, jint end_velocity)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createNoteStartAndEndEvents(track, start_tick, end_tick, channel, note, start_velocity, end_velocity);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createTempoEvent(JNIEnv *env, jobject jobject_track, jlong tick, jfloat tempo)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createTempoEvent(track, tick, tempo);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createTimeSignatureEvent(JNIEnv *env, jobject jobject_track, jlong tick, jint numerator, jint denominator)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_createTimeSignatureEvent(track, tick, numerator, denominator);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createLyricEvent(JNIEnv *env, jobject jobject_track, jlong tick, jstring jstring_text)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	const char *text = (*env)->GetStringUTFChars(env, jstring_text, NULL);
	MidiFileEvent_t event = MidiFileTrack_createLyricEvent(track, tick, (char *)(text));
	(*env)->ReleaseStringUTFChars(env, jstring_text, text);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createMarkerEvent(JNIEnv *env, jobject jobject_track, jlong tick, jstring jstring_text)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	const char *text = (*env)->GetStringUTFChars(env, jstring_text, NULL);
	MidiFileEvent_t event = MidiFileTrack_createMarkerEvent(track, tick, (char *)(text));
	(*env)->ReleaseStringUTFChars(env, jstring_text, text);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_createVoiceEvent(JNIEnv *env, jobject jobject_track, jlong tick, jbyteArray jbyteArray_data_buffer)
{
	int data_length = (*env)->GetArrayLength(env, jbyteArray_data_buffer);

	if (data_length == sizeof(unsigned long))
	{
		MidiFileTrack_t track;
		MidiFileEvent_t event;
		unsigned long data = 0;

		(*env)->GetByteArrayRegion(env, jbyteArray_data_buffer, 0, sizeof(unsigned long), (jbyte *)(&data));
		track = jobject_getPointer(env, jobject_track);
		event = MidiFileTrack_createVoiceEvent(track, tick, data);
		return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
	}
	else
	{
		return NULL;
	}
}

static jobject JNICALL Java_com_sreal_midifile_Track_getFirstEvent(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_getFirstEvent(track);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static jobject JNICALL Java_com_sreal_midifile_Track_getLastEvent(JNIEnv *env, jobject jobject_track)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	MidiFileEvent_t event = MidiFileTrack_getLastEvent(track);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", event);
}

static void JNICALL Java_com_sreal_midifile_Track_visitEvents(JNIEnv *env, jobject jobject_track, jobject jobject_event_visitor)
{
	MidiFileTrack_t track = jobject_getPointer(env, jobject_track);
	jclass jclass_event = (*env)->FindClass(env, "com/sreal/midifile/Event");
	jclass jclass_event_visitor = (*env)->GetObjectClass(env, jobject_event_visitor);
	jmethodID jmethodID_visitEvent = (*env)->GetMethodID(env, jclass_event_visitor, "visitEvent", "(Lcom/sreal/midifile/Event;)V");
	jobject jobject_event = (*env)->AllocObject(env, jclass_event);
	struct EventVisitorData event_visitor_data;
	event_visitor_data.env = env;
	event_visitor_data.jobject_event_visitor = jobject_event_visitor;
	event_visitor_data.jmethodID_visitEvent = jmethodID_visitEvent;
	event_visitor_data.jobject_event = jobject_event;
	MidiFileTrack_visitEvents(track, visitEvent, &event_visitor_data);
	(*env)->DeleteLocalRef(env, jobject_event);
}

static void JNICALL Java_com_sreal_midifile_Event_delete(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileEvent_delete(event);
}

static jobject JNICALL Java_com_sreal_midifile_Event_getTrack(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileTrack_t track = MidiFileEvent_getTrack(event);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Track", track);
}

static jobject JNICALL Java_com_sreal_midifile_Event_getPreviousEventInTrack(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileEvent_t other = MidiFileEvent_getPreviousEventInTrack(event);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", other);
}

static jobject JNICALL Java_com_sreal_midifile_Event_getNextEventInTrack(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileEvent_t other = MidiFileEvent_getNextEventInTrack(event);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", other);
}

static jobject JNICALL Java_com_sreal_midifile_Event_getPreviousEventInFile(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileEvent_t other = MidiFileEvent_getPreviousEventInFile(event);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", other);
}

static jobject JNICALL Java_com_sreal_midifile_Event_getNextEventInFile(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileEvent_t other = MidiFileEvent_getNextEventInFile(event);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", other);
}

static jlong JNICALL Java_com_sreal_midifile_Event_getTick(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_getTick(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setTick(JNIEnv *env, jobject jobject_event, jlong tick)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileEvent_setTick(event, tick);
}

static jint JNICALL Java_com_sreal_midifile_Event_getType(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_getType(event);
}

static jboolean JNICALL Java_com_sreal_midifile_Event_isNoteEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_isNoteEvent(event);
}

static jboolean JNICALL Java_com_sreal_midifile_Event_isNoteStartEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_isNoteStartEvent(event);
}

static jboolean JNICALL Java_com_sreal_midifile_Event_isNoteEndEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_isNoteEndEvent(event);
}

static jboolean JNICALL Java_com_sreal_midifile_Event_isTempoEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_isTempoEvent(event);
}

static jboolean JNICALL Java_com_sreal_midifile_Event_isTimeSignatureEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_isTimeSignatureEvent(event);
}

static jboolean JNICALL Java_com_sreal_midifile_Event_isLyricEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_isLyricEvent(event);
}

static jboolean JNICALL Java_com_sreal_midifile_Event_isMarkerEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_isMarkerEvent(event);
}

static jboolean JNICALL Java_com_sreal_midifile_Event_isVoiceEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileEvent_isVoiceEvent(event);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteOffEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteOffEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteOffEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteOffEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteOffEventNote(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteOffEvent_getNote(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteOffEventNote(JNIEnv *env, jobject jobject_event, jint note)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteOffEvent_setNote(event, note);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteOffEventVelocity(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteOffEvent_getVelocity(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteOffEventVelocity(JNIEnv *env, jobject jobject_event, jint velocity)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteOffEvent_setVelocity(event, velocity);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteOnEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteOnEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteOnEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteOnEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteOnEventNote(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteOnEvent_getNote(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteOnEventNote(JNIEnv *env, jobject jobject_event, jint note)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteOnEvent_setNote(event, note);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteOnEventVelocity(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteOnEvent_getVelocity(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteOnEventVelocity(JNIEnv *env, jobject jobject_event, jint velocity)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteOnEvent_setVelocity(event, velocity);
}

static jint JNICALL Java_com_sreal_midifile_Event_getKeyPressureEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileKeyPressureEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setKeyPressureEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileKeyPressureEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getKeyPressureEventNote(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileKeyPressureEvent_getNote(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setKeyPressureEventNote(JNIEnv *env, jobject jobject_event, jint note)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileKeyPressureEvent_setNote(event, note);
}

static jint JNICALL Java_com_sreal_midifile_Event_getKeyPressureEventAmount(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileKeyPressureEvent_getAmount(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setKeyPressureEventAmount(JNIEnv *env, jobject jobject_event, jint amount)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileKeyPressureEvent_setAmount(event, amount);
}

static jint JNICALL Java_com_sreal_midifile_Event_getControlChangeEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileControlChangeEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setControlChangeEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileControlChangeEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getControlChangeEventNumber(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileControlChangeEvent_getNumber(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setControlChangeEventNumber(JNIEnv *env, jobject jobject_event, jint number)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileControlChangeEvent_setNumber(event, number);
}

static jint JNICALL Java_com_sreal_midifile_Event_getControlChangeEventValue(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileControlChangeEvent_getValue(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setControlChangeEventValue(JNIEnv *env, jobject jobject_event, jint value)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileControlChangeEvent_setValue(event, value);
}

static jint JNICALL Java_com_sreal_midifile_Event_getProgramChangeEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileProgramChangeEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setProgramChangeEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileProgramChangeEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getProgramChangeEventNumber(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileProgramChangeEvent_getNumber(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setProgramChangeEventNumber(JNIEnv *env, jobject jobject_event, jint number)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileProgramChangeEvent_setNumber(event, number);
}

static jint JNICALL Java_com_sreal_midifile_Event_getChannelPressureEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileChannelPressureEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setChannelPressureEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileChannelPressureEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getChannelPressureEventAmount(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileChannelPressureEvent_getAmount(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setChannelPressureEventAmount(JNIEnv *env, jobject jobject_event, jint amount)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileChannelPressureEvent_setAmount(event, amount);
}

static jint JNICALL Java_com_sreal_midifile_Event_getPitchWheelEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFilePitchWheelEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setPitchWheelEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFilePitchWheelEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getPitchWheelEventValue(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFilePitchWheelEvent_getValue(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setPitchWheelEventValue(JNIEnv *env, jobject jobject_event, jint value)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFilePitchWheelEvent_setValue(event, value);
}

static jbyteArray JNICALL Java_com_sreal_midifile_Event_getSysexEventData(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	int data_length = MidiFileSysexEvent_getDataLength(event);
	unsigned char *data_buffer = MidiFileSysexEvent_getData(event);

	if (data_buffer == NULL)
	{
		return NULL;
	}
	else
	{
		jbyteArray jbyteArray_data_buffer = (*env)->NewByteArray(env, data_length);
		(*env)->SetByteArrayRegion(env, jbyteArray_data_buffer, 0, data_length, data_buffer);
		return jbyteArray_data_buffer;
	}
}

static void JNICALL Java_com_sreal_midifile_Event_setSysexEventData(JNIEnv *env, jobject jobject_event, jbyteArray jbyteArray_data_buffer)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	int data_length = (*env)->GetArrayLength(env, jbyteArray_data_buffer);
	jbyte *data_buffer = (*env)->GetByteArrayElements(env, jbyteArray_data_buffer, NULL);
	MidiFileSysexEvent_setData(event, data_length, data_buffer);
	(*env)->ReleaseByteArrayElements(env, jbyteArray_data_buffer, data_buffer, JNI_ABORT);
}

static jint JNICALL Java_com_sreal_midifile_Event_getMetaEventNumber(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileMetaEvent_getNumber(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setMetaEventNumber(JNIEnv *env, jobject jobject_event, jint number)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileMetaEvent_setNumber(event, number);
}

static jbyteArray JNICALL Java_com_sreal_midifile_Event_getMetaEventData(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	int data_length = MidiFileMetaEvent_getDataLength(event);
	unsigned char *data_buffer = MidiFileMetaEvent_getData(event);

	if (data_buffer == NULL)
	{
		return NULL;
	}
	else
	{
		jbyteArray jbyteArray_data_buffer = (*env)->NewByteArray(env, data_length);
		(*env)->SetByteArrayRegion(env, jbyteArray_data_buffer, 0, data_length, data_buffer);
		return jbyteArray_data_buffer;
	}
}

static void JNICALL Java_com_sreal_midifile_Event_setMetaEventData(JNIEnv *env, jobject jobject_event, jbyteArray jbyteArray_data_buffer)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	int data_length = (*env)->GetArrayLength(env, jbyteArray_data_buffer);
	jbyte *data_buffer = (*env)->GetByteArrayElements(env, jbyteArray_data_buffer, NULL);
	MidiFileMetaEvent_setData(event, data_length, data_buffer);
	(*env)->ReleaseByteArrayElements(env, jbyteArray_data_buffer, data_buffer, JNI_ABORT);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteStartEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteStartEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteStartEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteStartEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteStartEventNote(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteStartEvent_getNote(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteStartEventNote(JNIEnv *env, jobject jobject_event, jint note)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteStartEvent_setNote(event, note);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteStartEventVelocity(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteStartEvent_getVelocity(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteStartEventVelocity(JNIEnv *env, jobject jobject_event, jint velocity)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteStartEvent_setVelocity(event, velocity);
}

static jobject JNICALL Java_com_sreal_midifile_Event_getNoteStartEventNoteEndEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileEvent_t other = MidiFileNoteStartEvent_getNoteEndEvent(event);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", other);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteEndEventChannel(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteEndEvent_getChannel(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteEndEventChannel(JNIEnv *env, jobject jobject_event, jint channel)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteEndEvent_setChannel(event, channel);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteEndEventNote(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteEndEvent_getNote(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteEndEventNote(JNIEnv *env, jobject jobject_event, jint note)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteEndEvent_setNote(event, note);
}

static jint JNICALL Java_com_sreal_midifile_Event_getNoteEndEventVelocity(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileNoteEndEvent_getVelocity(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setNoteEndEventVelocity(JNIEnv *env, jobject jobject_event, jint velocity)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileNoteEndEvent_setVelocity(event, velocity);
}

static jobject JNICALL Java_com_sreal_midifile_Event_getNoteEndEventNoteStartEvent(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileEvent_t other = MidiFileNoteEndEvent_getNoteStartEvent(event);
	return jobject_newWithPointer(env, NULL, "com/sreal/midifile/Event", other);
}

static jfloat JNICALL Java_com_sreal_midifile_Event_getTempoEventTempo(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileTempoEvent_getTempo(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setTempoEventTempo(JNIEnv *env, jobject jobject_event, jfloat tempo)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileTempoEvent_setTempo(event, tempo);
}

static jint JNICALL Java_com_sreal_midifile_Event_getTimeSignatureEventNumerator(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileTimeSignatureEvent_getNumerator(event);
}

static jint JNICALL Java_com_sreal_midifile_Event_getTimeSignatureEventDenominator(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	return MidiFileTimeSignatureEvent_getDenominator(event);
}

static void JNICALL Java_com_sreal_midifile_Event_setTimeSignatureEventTimeSignature(JNIEnv *env, jobject jobject_event, jint numerator, jint denominator)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	MidiFileTimeSignatureEvent_setTimeSignature(event, numerator, denominator);
}

static jstring JNICALL Java_com_sreal_midifile_Event_getLyricEventText(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	char *text = MidiFileLyricEvent_getText(event);
	return (*env)->NewStringUTF(env, text);
}

static void JNICALL Java_com_sreal_midifile_Event_setLyricEventText(JNIEnv *env, jobject jobject_event, jstring jstring_text)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	const char *text = (*env)->GetStringUTFChars(env, jstring_text, NULL);
	MidiFileLyricEvent_setText(event, (char *)(text));
	(*env)->ReleaseStringUTFChars(env, jstring_text, text);
}

static jstring JNICALL Java_com_sreal_midifile_Event_getMarkerEventText(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	char *text = MidiFileMarkerEvent_getText(event);
	return (*env)->NewStringUTF(env, text);
}

static void JNICALL Java_com_sreal_midifile_Event_setMarkerEventText(JNIEnv *env, jobject jobject_event, jstring jstring_text)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	const char *text = (*env)->GetStringUTFChars(env, jstring_text, NULL);
	MidiFileMarkerEvent_setText(event, (char *)(text));
	(*env)->ReleaseStringUTFChars(env, jstring_text, text);
}

static jbyteArray JNICALL Java_com_sreal_midifile_Event_getVoiceEventData(JNIEnv *env, jobject jobject_event)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	unsigned long data = MidiFileVoiceEvent_getData(event);
	jbyteArray jbyteArray_data_buffer = (*env)->NewByteArray(env, sizeof(unsigned long));
	(*env)->SetByteArrayRegion(env, jbyteArray_data_buffer, 0, sizeof(unsigned long), (jbyte *)(&data));
	return jbyteArray_data_buffer;
}

static void JNICALL Java_com_sreal_midifile_Event_setVoiceEventData(JNIEnv *env, jobject jobject_event, jbyteArray jbyteArray_data_buffer)
{
	MidiFileEvent_t event = jobject_getPointer(env, jobject_event);
	unsigned long data = 0;
	(*env)->GetByteArrayRegion(env, jbyteArray_data_buffer, 0, sizeof(unsigned long), (jbyte *)(&data));
	MidiFileVoiceEvent_setData(event, data);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeat_init(JNIEnv *env, jobject jobject_measure_beat)
{
	MidiFileMeasureBeat_t measure_beat = MidiFileMeasureBeat_new();
	jobject_setPointer(env, jobject_measure_beat, measure_beat);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeat_finalize(JNIEnv *env, jobject jobject_measure_beat)
{
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	MidiFileMeasureBeat_free(measure_beat);
}

static jlong JNICALL Java_com_sreal_midifile_MeasureBeat_getMeasure(JNIEnv *env, jobject jobject_measure_beat)
{
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	return MidiFileMeasureBeat_getMeasure(measure_beat);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeat_setMeasure(JNIEnv *env, jobject jobject_measure_beat, jlong measure)
{
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	MidiFileMeasureBeat_setMeasure(measure_beat, measure);
}

static jfloat JNICALL Java_com_sreal_midifile_MeasureBeat_getBeat(JNIEnv *env, jobject jobject_measure_beat)
{
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	return MidiFileMeasureBeat_getBeat(measure_beat);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeat_setBeat(JNIEnv *env, jobject jobject_measure_beat, jfloat beat)
{
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	MidiFileMeasureBeat_setBeat(measure_beat, beat);
}

static jstring JNICALL Java_com_sreal_midifile_MeasureBeat_toString(JNIEnv *env, jobject jobject_measure_beat)
{
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	char *measure_beat_string = MidiFileMeasureBeat_toString(measure_beat);
	return (*env)->NewStringUTF(env, measure_beat_string);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeat_parse(JNIEnv *env, jobject jobject_measure_beat, jstring jstring_measure_beat_string)
{
	MidiFileMeasureBeat_t measure_beat = jobject_getPointer(env, jobject_measure_beat);
	const char *measure_beat_string = (*env)->GetStringUTFChars(env, jstring_measure_beat_string, NULL);
	MidiFileMeasureBeat_parse(measure_beat, (char *)(measure_beat_string));
	(*env)->ReleaseStringUTFChars(env, jstring_measure_beat_string, measure_beat_string);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeatTick_init(JNIEnv *env, jobject jobject_measure_beat_tick)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = MidiFileMeasureBeatTick_new();
	jobject_setPointer(env, jobject_measure_beat_tick, measure_beat_tick);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeatTick_finalize(JNIEnv *env, jobject jobject_measure_beat_tick)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	MidiFileMeasureBeatTick_free(measure_beat_tick);
}

static jlong JNICALL Java_com_sreal_midifile_MeasureBeatTick_getMeasure(JNIEnv *env, jobject jobject_measure_beat_tick)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	return MidiFileMeasureBeatTick_getMeasure(measure_beat_tick);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeatTick_setMeasure(JNIEnv *env, jobject jobject_measure_beat_tick, jlong measure)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	MidiFileMeasureBeatTick_setMeasure(measure_beat_tick, measure);
}

static jlong JNICALL Java_com_sreal_midifile_MeasureBeatTick_getBeat(JNIEnv *env, jobject jobject_measure_beat_tick)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	return MidiFileMeasureBeatTick_getBeat(measure_beat_tick);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeatTick_setBeat(JNIEnv *env, jobject jobject_measure_beat_tick, jlong beat)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	MidiFileMeasureBeatTick_setBeat(measure_beat_tick, beat);
}

static jfloat JNICALL Java_com_sreal_midifile_MeasureBeatTick_getTick(JNIEnv *env, jobject jobject_measure_beat_tick)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	return MidiFileMeasureBeatTick_getTick(measure_beat_tick);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeatTick_setTick(JNIEnv *env, jobject jobject_measure_beat_tick, jfloat tick)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	MidiFileMeasureBeatTick_setTick(measure_beat_tick, tick);
}

static jstring JNICALL Java_com_sreal_midifile_MeasureBeatTick_toString(JNIEnv *env, jobject jobject_measure_beat_tick)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	char *measure_beat_tick_string = MidiFileMeasureBeatTick_toString(measure_beat_tick);
	return (*env)->NewStringUTF(env, measure_beat_tick_string);
}

static void JNICALL Java_com_sreal_midifile_MeasureBeatTick_parse(JNIEnv *env, jobject jobject_measure_beat_tick, jstring jstring_measure_beat_tick_string)
{
	MidiFileMeasureBeatTick_t measure_beat_tick = jobject_getPointer(env, jobject_measure_beat_tick);
	const char *measure_beat_tick_string = (*env)->GetStringUTFChars(env, jstring_measure_beat_tick_string, NULL);
	MidiFileMeasureBeatTick_parse(measure_beat_tick, (char *)(measure_beat_tick_string));
	(*env)->ReleaseStringUTFChars(env, jstring_measure_beat_tick_string, measure_beat_tick_string);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecond_init(JNIEnv *env, jobject jobject_hour_minute_second)
{
	MidiFileHourMinuteSecond_t hour_minute_second = MidiFileHourMinuteSecond_new();
	jobject_setPointer(env, jobject_hour_minute_second, hour_minute_second);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecond_finalize(JNIEnv *env, jobject jobject_hour_minute_second)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	MidiFileHourMinuteSecond_free(hour_minute_second);
}

static jlong JNICALL Java_com_sreal_midifile_HourMinuteSecond_getHour(JNIEnv *env, jobject jobject_hour_minute_second)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	return MidiFileHourMinuteSecond_getHour(hour_minute_second);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecond_setHour(JNIEnv *env, jobject jobject_hour_minute_second, jlong hour)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	MidiFileHourMinuteSecond_setHour(hour_minute_second, hour);
}

static jlong JNICALL Java_com_sreal_midifile_HourMinuteSecond_getMinute(JNIEnv *env, jobject jobject_hour_minute_second)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	return MidiFileHourMinuteSecond_getMinute(hour_minute_second);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecond_setMinute(JNIEnv *env, jobject jobject_hour_minute_second, jlong minute)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	MidiFileHourMinuteSecond_setMinute(hour_minute_second, minute);
}

static jfloat JNICALL Java_com_sreal_midifile_HourMinuteSecond_getSecond(JNIEnv *env, jobject jobject_hour_minute_second)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	return MidiFileHourMinuteSecond_getSecond(hour_minute_second);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecond_setSecond(JNIEnv *env, jobject jobject_hour_minute_second, jfloat second)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	MidiFileHourMinuteSecond_setSecond(hour_minute_second, second);
}

static jstring JNICALL Java_com_sreal_midifile_HourMinuteSecond_toString(JNIEnv *env, jobject jobject_hour_minute_second)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	char *hour_minute_second_string = MidiFileHourMinuteSecond_toString(hour_minute_second);
	return (*env)->NewStringUTF(env, hour_minute_second_string);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecond_parse(JNIEnv *env, jobject jobject_hour_minute_second, jstring jstring_hour_minute_second_string)
{
	MidiFileHourMinuteSecond_t hour_minute_second = jobject_getPointer(env, jobject_hour_minute_second);
	const char *hour_minute_second_string = (*env)->GetStringUTFChars(env, jstring_hour_minute_second_string, NULL);
	MidiFileHourMinuteSecond_parse(hour_minute_second, (char *)(hour_minute_second_string));
	(*env)->ReleaseStringUTFChars(env, jstring_hour_minute_second_string, hour_minute_second_string);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_init(JNIEnv *env, jobject jobject_hour_minute_second_frame)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = MidiFileHourMinuteSecondFrame_new();
	jobject_setPointer(env, jobject_hour_minute_second_frame, hour_minute_second_frame);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_finalize(JNIEnv *env, jobject jobject_hour_minute_second_frame)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	MidiFileHourMinuteSecondFrame_free(hour_minute_second_frame);
}

static jlong JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_getHour(JNIEnv *env, jobject jobject_hour_minute_second_frame)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	return MidiFileHourMinuteSecondFrame_getHour(hour_minute_second_frame);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_setHour(JNIEnv *env, jobject jobject_hour_minute_second_frame, jlong hour)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	MidiFileHourMinuteSecondFrame_setHour(hour_minute_second_frame, hour);
}

static jlong JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_getMinute(JNIEnv *env, jobject jobject_hour_minute_second_frame)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	return MidiFileHourMinuteSecondFrame_getMinute(hour_minute_second_frame);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_setMinute(JNIEnv *env, jobject jobject_hour_minute_second_frame, jlong minute)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	MidiFileHourMinuteSecondFrame_setMinute(hour_minute_second_frame, minute);
}

static jlong JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_getSecond(JNIEnv *env, jobject jobject_hour_minute_second_frame)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	return MidiFileHourMinuteSecondFrame_getSecond(hour_minute_second_frame);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_setSecond(JNIEnv *env, jobject jobject_hour_minute_second_frame, jlong second)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	MidiFileHourMinuteSecondFrame_setSecond(hour_minute_second_frame, second);
}

static jfloat JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_getFrame(JNIEnv *env, jobject jobject_hour_minute_second_frame)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	return MidiFileHourMinuteSecondFrame_getFrame(hour_minute_second_frame);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_setFrame(JNIEnv *env, jobject jobject_hour_minute_second_frame, jfloat frame)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	MidiFileHourMinuteSecondFrame_setFrame(hour_minute_second_frame, frame);
}

static jstring JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_toString(JNIEnv *env, jobject jobject_hour_minute_second_frame)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	char *hour_minute_second_frame_string = MidiFileHourMinuteSecondFrame_toString(hour_minute_second_frame);
	return (*env)->NewStringUTF(env, hour_minute_second_frame_string);
}

static void JNICALL Java_com_sreal_midifile_HourMinuteSecondFrame_parse(JNIEnv *env, jobject jobject_hour_minute_second_frame, jstring jstring_hour_minute_second_frame_string)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame = jobject_getPointer(env, jobject_hour_minute_second_frame);
	const char *hour_minute_second_frame_string = (*env)->GetStringUTFChars(env, jstring_hour_minute_second_frame_string, NULL);
	MidiFileHourMinuteSecondFrame_parse(hour_minute_second_frame, (char *)(hour_minute_second_frame_string));
	(*env)->ReleaseStringUTFChars(env, jstring_hour_minute_second_frame_string, hour_minute_second_frame_string);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv *env = NULL;
	if ((*vm)->GetEnv(vm, (void **)(&env), JNI_VERSION_1_2) != JNI_OK) return -1;

	{
		JNINativeMethod native_methods[] = {
			{"equals", "(Ljava/lang/Object;)Z", Java_com_sreal_midifile_MidiFile_equals},
			{"load", "(Ljava/lang/String;)Lcom/sreal/midifile/MidiFile;", Java_com_sreal_midifile_MidiFile_load},
			{"save", "(Ljava/lang/String;)V", Java_com_sreal_midifile_MidiFile_save},
			{"init", "(III)V", Java_com_sreal_midifile_MidiFile_init},
			{"free", "()V", Java_com_sreal_midifile_MidiFile_free},
			{"getFileFormat", "()I", Java_com_sreal_midifile_MidiFile_getFileFormat},
			{"setFileFormat", "(I)V", Java_com_sreal_midifile_MidiFile_setFileFormat},
			{"getDivisionType", "()I", Java_com_sreal_midifile_MidiFile_getDivisionType},
			{"setDivisionType", "(I)V", Java_com_sreal_midifile_MidiFile_setDivisionType},
			{"getResolution", "()I", Java_com_sreal_midifile_MidiFile_getResolution},
			{"setResolution", "(I)V", Java_com_sreal_midifile_MidiFile_setResolution},
			{"getNumberOfTicksPerBeat", "()I", Java_com_sreal_midifile_MidiFile_getNumberOfTicksPerBeat},
			{"setNumberOfTicksPerBeat", "(I)V", Java_com_sreal_midifile_MidiFile_setNumberOfTicksPerBeat},
			{"getNumberOfFramesPerSecond", "()F", Java_com_sreal_midifile_MidiFile_getNumberOfFramesPerSecond},
			{"setNumberOfFramesPerSecond", "(F)V", Java_com_sreal_midifile_MidiFile_setNumberOfFramesPerSecond},
			{"createTrack", "()Lcom/sreal/midifile/Track;", Java_com_sreal_midifile_MidiFile_createTrack},
			{"getNumberOfTracks", "()I", Java_com_sreal_midifile_MidiFile_getNumberOfTracks},
			{"getTrackByNumber", "(IZ)Lcom/sreal/midifile/Track;", Java_com_sreal_midifile_MidiFile_getTrackByNumber},
			{"getFirstTrack", "()Lcom/sreal/midifile/Track;", Java_com_sreal_midifile_MidiFile_getFirstTrack},
			{"getLastTrack", "()Lcom/sreal/midifile/Track;", Java_com_sreal_midifile_MidiFile_getLastTrack},
			{"getFirstEvent", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_MidiFile_getFirstEvent},
			{"getLastEvent", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_MidiFile_getLastEvent},
			{"visitEvents", "(Lcom/sreal/midifile/EventVisitor;)V", Java_com_sreal_midifile_MidiFile_visitEvents},
			{"getBeatFromTick", "(J)F", Java_com_sreal_midifile_MidiFile_getBeatFromTick},
			{"getTickFromBeat", "(F)J", Java_com_sreal_midifile_MidiFile_getTickFromBeat},
			{"getTimeFromTick", "(J)F", Java_com_sreal_midifile_MidiFile_getTimeFromTick},
			{"getTickFromTime", "(F)J", Java_com_sreal_midifile_MidiFile_getTickFromTime},
			{"setMeasureBeatFromTick", "(JLcom/sreal/midifile/MeasureBeat;)V", Java_com_sreal_midifile_MidiFile_setMeasureBeatFromTick},
			{"getTickFromMeasureBeat", "(Lcom/sreal/midifile/MeasureBeat;)J", Java_com_sreal_midifile_MidiFile_getTickFromMeasureBeat},
			{"setMeasureBeatTickFromTick", "(JLcom/sreal/midifile/MeasureBeatTick;)V", Java_com_sreal_midifile_MidiFile_setMeasureBeatTickFromTick},
			{"getTickFromMeasureBeatTick", "(Lcom/sreal/midifile/MeasureBeatTick;)J", Java_com_sreal_midifile_MidiFile_getTickFromMeasureBeatTick},
			{"setHourMinuteSecondFromTick", "(JLcom/sreal/midifile/HourMinuteSecond;)V", Java_com_sreal_midifile_MidiFile_setHourMinuteSecondFromTick},
			{"getTickFromHourMinuteSecond", "(Lcom/sreal/midifile/HourMinuteSecond;)J", Java_com_sreal_midifile_MidiFile_getTickFromHourMinuteSecond},
			{"setHourMinuteSecondFrameFromTick", "(JLcom/sreal/midifile/HourMinuteSecondFrame;)V", Java_com_sreal_midifile_MidiFile_setHourMinuteSecondFrameFromTick},
			{"getTickFromHourMinuteSecondFrame", "(Lcom/sreal/midifile/HourMinuteSecondFrame;)J", Java_com_sreal_midifile_MidiFile_getTickFromHourMinuteSecondFrame},
			{"getMeasureBeatStringFromTick", "(J)Ljava/lang/String;", Java_com_sreal_midifile_MidiFile_getMeasureBeatStringFromTick},
			{"getTickFromMeasureBeatString", "(Ljava/lang/String;)J", Java_com_sreal_midifile_MidiFile_getTickFromMeasureBeatString},
			{"getMeasureBeatTickStringFromTick", "(J)Ljava/lang/String;", Java_com_sreal_midifile_MidiFile_getMeasureBeatTickStringFromTick},
			{"getTickFromMeasureBeatTickString", "(Ljava/lang/String;)J", Java_com_sreal_midifile_MidiFile_getTickFromMeasureBeatTickString},
			{"getHourMinuteSecondStringFromTick", "(J)Ljava/lang/String;", Java_com_sreal_midifile_MidiFile_getHourMinuteSecondStringFromTick},
			{"getTickFromHourMinuteSecondString", "(Ljava/lang/String;)J", Java_com_sreal_midifile_MidiFile_getTickFromHourMinuteSecondString},
			{"getHourMinuteSecondFrameStringFromTick", "(J)Ljava/lang/String;", Java_com_sreal_midifile_MidiFile_getHourMinuteSecondFrameStringFromTick},
			{"getTickFromHourMinuteSecondFrameString", "(Ljava/lang/String;)J", Java_com_sreal_midifile_MidiFile_getTickFromHourMinuteSecondFrameString},
			{"getTickFromMarker", "(Ljava/lang/String;)J", Java_com_sreal_midifile_MidiFile_getTickFromMarker}
		};

		jclass jclass_ = (*env)->FindClass(env, "com/sreal/midifile/MidiFile");
		(*env)->RegisterNatives(env, jclass_, native_methods, sizeof(native_methods) / sizeof(JNINativeMethod));
	}

	{
		JNINativeMethod native_methods[] = {
			{"equals", "(Ljava/lang/Object;)Z", Java_com_sreal_midifile_Track_equals},
			{"delete", "()V", Java_com_sreal_midifile_Track_delete},
			{"getMidiFile", "()Lcom/sreal/midifile/MidiFile;", Java_com_sreal_midifile_Track_getMidiFile},
			{"getNumber", "()I", Java_com_sreal_midifile_Track_getNumber},
			{"getEndTick", "()J", Java_com_sreal_midifile_Track_getEndTick},
			{"setEndTick", "(J)V", Java_com_sreal_midifile_Track_setEndTick},
			{"createTrackBefore", "()Lcom/sreal/midifile/Track;", Java_com_sreal_midifile_Track_createTrackBefore},
			{"getPreviousTrack", "()Lcom/sreal/midifile/Track;", Java_com_sreal_midifile_Track_getPreviousTrack},
			{"getNextTrack", "()Lcom/sreal/midifile/Track;", Java_com_sreal_midifile_Track_getNextTrack},
			{"createNoteOffEvent", "(JIII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createNoteOffEvent},
			{"createNoteOnEvent", "(JIII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createNoteOnEvent},
			{"createKeyPressureEvent", "(JIII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createKeyPressureEvent},
			{"createControlChangeEvent", "(JIII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createControlChangeEvent},
			{"createProgramChangeEvent", "(JII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createProgramChangeEvent},
			{"createChannelPressureEvent", "(JII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createChannelPressureEvent},
			{"createPitchWheelEvent", "(JII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createPitchWheelEvent},
			{"createSysexEvent", "(J[B)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createSysexEvent},
			{"createMetaEvent", "(JI[B)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createMetaEvent},
			{"createNoteStartAndEndEvents", "(JJIIII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createNoteStartAndEndEvents},
			{"createTempoEvent", "(JF)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createTempoEvent},
			{"createTimeSignatureEvent", "(JII)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createTimeSignatureEvent},
			{"createLyricEvent", "(JLjava/lang/String;)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createLyricEvent},
			{"createMarkerEvent", "(JLjava/lang/String;)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createMarkerEvent},
			{"createVoiceEvent", "(J[B)Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_createVoiceEvent},
			{"getFirstEvent", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_getFirstEvent},
			{"getLastEvent", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Track_getLastEvent},
			{"visitEvents", "(Lcom/sreal/midifile/EventVisitor;)V", Java_com_sreal_midifile_Track_visitEvents}
		};

		jclass jclass_ = (*env)->FindClass(env, "com/sreal/midifile/Track");
		(*env)->RegisterNatives(env, jclass_, native_methods, sizeof(native_methods) / sizeof(JNINativeMethod));
	}

	{
		JNINativeMethod native_methods[] = {
			{"delete", "()V", Java_com_sreal_midifile_Event_delete},
			{"getTrack", "()Lcom/sreal/midifile/Track;", Java_com_sreal_midifile_Event_getTrack},
			{"getPreviousEventInTrack", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Event_getPreviousEventInTrack},
			{"getNextEventInTrack", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Event_getNextEventInTrack},
			{"getPreviousEventInFile", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Event_getPreviousEventInFile},
			{"getNextEventInFile", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Event_getNextEventInFile},
			{"getTick", "()J", Java_com_sreal_midifile_Event_getTick},
			{"setTick", "(J)V", Java_com_sreal_midifile_Event_setTick},
			{"getType", "()I", Java_com_sreal_midifile_Event_getType},
			{"isNoteEvent", "()Z", Java_com_sreal_midifile_Event_isNoteEvent},
			{"isNoteStartEvent", "()Z", Java_com_sreal_midifile_Event_isNoteStartEvent},
			{"isNoteEndEvent", "()Z", Java_com_sreal_midifile_Event_isNoteEndEvent},
			{"isTempoEvent", "()Z", Java_com_sreal_midifile_Event_isTempoEvent},
			{"isTimeSignatureEvent", "()Z", Java_com_sreal_midifile_Event_isTimeSignatureEvent},
			{"isLyricEvent", "()Z", Java_com_sreal_midifile_Event_isLyricEvent},
			{"isMarkerEvent", "()Z", Java_com_sreal_midifile_Event_isMarkerEvent},
			{"isVoiceEvent", "()Z", Java_com_sreal_midifile_Event_isVoiceEvent},
			{"getNoteOffEventChannel", "()I", Java_com_sreal_midifile_Event_getNoteOffEventChannel},
			{"setNoteOffEventChannel", "(I)V", Java_com_sreal_midifile_Event_setNoteOffEventChannel},
			{"getNoteOffEventNote", "()I", Java_com_sreal_midifile_Event_getNoteOffEventNote},
			{"setNoteOffEventNote", "(I)V", Java_com_sreal_midifile_Event_setNoteOffEventNote},
			{"getNoteOffEventVelocity", "()I", Java_com_sreal_midifile_Event_getNoteOffEventVelocity},
			{"setNoteOffEventVelocity", "(I)V", Java_com_sreal_midifile_Event_setNoteOffEventVelocity},
			{"getNoteOnEventChannel", "()I", Java_com_sreal_midifile_Event_getNoteOnEventChannel},
			{"setNoteOnEventChannel", "(I)V", Java_com_sreal_midifile_Event_setNoteOnEventChannel},
			{"getNoteOnEventNote", "()I", Java_com_sreal_midifile_Event_getNoteOnEventNote},
			{"setNoteOnEventNote", "(I)V", Java_com_sreal_midifile_Event_setNoteOnEventNote},
			{"getNoteOnEventVelocity", "()I", Java_com_sreal_midifile_Event_getNoteOnEventVelocity},
			{"setNoteOnEventVelocity", "(I)V", Java_com_sreal_midifile_Event_setNoteOnEventVelocity},
			{"getKeyPressureEventChannel", "()I", Java_com_sreal_midifile_Event_getKeyPressureEventChannel},
			{"setKeyPressureEventChannel", "(I)V", Java_com_sreal_midifile_Event_setKeyPressureEventChannel},
			{"getKeyPressureEventNote", "()I", Java_com_sreal_midifile_Event_getKeyPressureEventNote},
			{"setKeyPressureEventNote", "(I)V", Java_com_sreal_midifile_Event_setKeyPressureEventNote},
			{"getKeyPressureEventAmount", "()I", Java_com_sreal_midifile_Event_getKeyPressureEventAmount},
			{"setKeyPressureEventAmount", "(I)V", Java_com_sreal_midifile_Event_setKeyPressureEventAmount},
			{"getControlChangeEventChannel", "()I", Java_com_sreal_midifile_Event_getControlChangeEventChannel},
			{"setControlChangeEventChannel", "(I)V", Java_com_sreal_midifile_Event_setControlChangeEventChannel},
			{"getControlChangeEventNumber", "()I", Java_com_sreal_midifile_Event_getControlChangeEventNumber},
			{"setControlChangeEventNumber", "(I)V", Java_com_sreal_midifile_Event_setControlChangeEventNumber},
			{"getControlChangeEventValue", "()I", Java_com_sreal_midifile_Event_getControlChangeEventValue},
			{"setControlChangeEventValue", "(I)V", Java_com_sreal_midifile_Event_setControlChangeEventValue},
			{"getProgramChangeEventChannel", "()I", Java_com_sreal_midifile_Event_getProgramChangeEventChannel},
			{"setProgramChangeEventChannel", "(I)V", Java_com_sreal_midifile_Event_setProgramChangeEventChannel},
			{"getProgramChangeEventNumber", "()I", Java_com_sreal_midifile_Event_getProgramChangeEventNumber},
			{"setProgramChangeEventNumber", "(I)V", Java_com_sreal_midifile_Event_setProgramChangeEventNumber},
			{"getChannelPressureEventChannel", "()I", Java_com_sreal_midifile_Event_getChannelPressureEventChannel},
			{"setChannelPressureEventChannel", "(I)V", Java_com_sreal_midifile_Event_setChannelPressureEventChannel},
			{"getChannelPressureEventAmount", "()I", Java_com_sreal_midifile_Event_getChannelPressureEventAmount},
			{"setChannelPressureEventAmount", "(I)V", Java_com_sreal_midifile_Event_setChannelPressureEventAmount},
			{"getPitchWheelEventChannel", "()I", Java_com_sreal_midifile_Event_getPitchWheelEventChannel},
			{"setPitchWheelEventChannel", "(I)V", Java_com_sreal_midifile_Event_setPitchWheelEventChannel},
			{"getPitchWheelEventValue", "()I", Java_com_sreal_midifile_Event_getPitchWheelEventValue},
			{"setPitchWheelEventValue", "(I)V", Java_com_sreal_midifile_Event_setPitchWheelEventValue},
			{"getSysexEventData", "()[B", Java_com_sreal_midifile_Event_getSysexEventData},
			{"setSysexEventData", "([B)V", Java_com_sreal_midifile_Event_setSysexEventData},
			{"getMetaEventNumber", "()I", Java_com_sreal_midifile_Event_getMetaEventNumber},
			{"setMetaEventNumber", "(I)V", Java_com_sreal_midifile_Event_setMetaEventNumber},
			{"getMetaEventData", "()[B", Java_com_sreal_midifile_Event_getMetaEventData},
			{"setMetaEventData", "([B)V", Java_com_sreal_midifile_Event_setMetaEventData},
			{"getNoteStartEventChannel", "()I", Java_com_sreal_midifile_Event_getNoteStartEventChannel},
			{"setNoteStartEventChannel", "(I)V", Java_com_sreal_midifile_Event_setNoteStartEventChannel},
			{"getNoteStartEventNote", "()I", Java_com_sreal_midifile_Event_getNoteStartEventNote},
			{"setNoteStartEventNote", "(I)V", Java_com_sreal_midifile_Event_setNoteStartEventNote},
			{"getNoteStartEventVelocity", "()I", Java_com_sreal_midifile_Event_getNoteStartEventVelocity},
			{"setNoteStartEventVelocity", "(I)V", Java_com_sreal_midifile_Event_setNoteStartEventVelocity},
			{"getNoteStartEventNoteEndEvent", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Event_getNoteStartEventNoteEndEvent},
			{"getNoteEndEventChannel", "()I", Java_com_sreal_midifile_Event_getNoteEndEventChannel},
			{"setNoteEndEventChannel", "(I)V", Java_com_sreal_midifile_Event_setNoteEndEventChannel},
			{"getNoteEndEventNote", "()I", Java_com_sreal_midifile_Event_getNoteEndEventNote},
			{"setNoteEndEventNote", "(I)V", Java_com_sreal_midifile_Event_setNoteEndEventNote},
			{"getNoteEndEventVelocity", "()I", Java_com_sreal_midifile_Event_getNoteEndEventVelocity},
			{"setNoteEndEventVelocity", "(I)V", Java_com_sreal_midifile_Event_setNoteEndEventVelocity},
			{"getNoteEndEventNoteStartEvent", "()Lcom/sreal/midifile/Event;", Java_com_sreal_midifile_Event_getNoteEndEventNoteStartEvent},
			{"getTempoEventTempo", "()F", Java_com_sreal_midifile_Event_getTempoEventTempo},
			{"setTempoEventTempo", "(F)V", Java_com_sreal_midifile_Event_setTempoEventTempo},
			{"getTimeSignatureEventNumerator", "()I", Java_com_sreal_midifile_Event_getTimeSignatureEventNumerator},
			{"getTimeSignatureEventDenominator", "()I", Java_com_sreal_midifile_Event_getTimeSignatureEventDenominator},
			{"setTimeSignatureEventTimeSignature", "(II)V", Java_com_sreal_midifile_Event_setTimeSignatureEventTimeSignature},
			{"getLyricEventText", "()Ljava/lang/String;", Java_com_sreal_midifile_Event_getLyricEventText},
			{"setLyricEventText", "(Ljava/lang/String;)V", Java_com_sreal_midifile_Event_setLyricEventText},
			{"getMarkerEventText", "()Ljava/lang/String;", Java_com_sreal_midifile_Event_getMarkerEventText},
			{"setMarkerEventText", "(Ljava/lang/String;)V", Java_com_sreal_midifile_Event_setMarkerEventText},
			{"getVoiceEventData", "()[B", Java_com_sreal_midifile_Event_getVoiceEventData},
			{"setVoiceEventData", "([B)V", Java_com_sreal_midifile_Event_setVoiceEventData}
		};

		jclass jclass_ = (*env)->FindClass(env, "com/sreal/midifile/Event");
		(*env)->RegisterNatives(env, jclass_, native_methods, sizeof(native_methods) / sizeof(JNINativeMethod));
	}

	{
		JNINativeMethod native_methods[] = {
			{"init", "()V", Java_com_sreal_midifile_MeasureBeat_init},
			{"finalize", "()V", Java_com_sreal_midifile_MeasureBeat_finalize},
			{"getMeasure", "()J", Java_com_sreal_midifile_MeasureBeat_getMeasure},
			{"setMeasure", "(J)V", Java_com_sreal_midifile_MeasureBeat_setMeasure},
			{"getBeat", "()F", Java_com_sreal_midifile_MeasureBeat_getBeat},
			{"setBeat", "(F)V", Java_com_sreal_midifile_MeasureBeat_setBeat},
			{"toString", "()Ljava/lang/String;", Java_com_sreal_midifile_MeasureBeat_toString},
			{"parse", "(Ljava/lang/String;)V", Java_com_sreal_midifile_MeasureBeat_parse}
		};

		jclass jclass_ = (*env)->FindClass(env, "com/sreal/midifile/MeasureBeat");
		(*env)->RegisterNatives(env, jclass_, native_methods, sizeof(native_methods) / sizeof(JNINativeMethod));
	}

	{
		JNINativeMethod native_methods[] = {
			{"init", "()V", Java_com_sreal_midifile_MeasureBeatTick_init},
			{"finalize", "()V", Java_com_sreal_midifile_MeasureBeatTick_finalize},
			{"getMeasure", "()J", Java_com_sreal_midifile_MeasureBeatTick_getMeasure},
			{"setMeasure", "(J)V", Java_com_sreal_midifile_MeasureBeatTick_setMeasure},
			{"getBeat", "()J", Java_com_sreal_midifile_MeasureBeatTick_getBeat},
			{"setBeat", "(J)V", Java_com_sreal_midifile_MeasureBeatTick_setBeat},
			{"getTick", "()F", Java_com_sreal_midifile_MeasureBeatTick_getTick},
			{"setTick", "(F)V", Java_com_sreal_midifile_MeasureBeatTick_setTick},
			{"toString", "()Ljava/lang/String;", Java_com_sreal_midifile_MeasureBeatTick_toString},
			{"parse", "(Ljava/lang/String;)V", Java_com_sreal_midifile_MeasureBeatTick_parse}
		};

		jclass jclass_ = (*env)->FindClass(env, "com/sreal/midifile/MeasureBeatTick");
		(*env)->RegisterNatives(env, jclass_, native_methods, sizeof(native_methods) / sizeof(JNINativeMethod));
	}

	{
		JNINativeMethod native_methods[] = {
			{"init", "()V", Java_com_sreal_midifile_HourMinuteSecond_init},
			{"finalize", "()V", Java_com_sreal_midifile_HourMinuteSecond_finalize},
			{"getHour", "()J", Java_com_sreal_midifile_HourMinuteSecond_getHour},
			{"setHour", "(J)V", Java_com_sreal_midifile_HourMinuteSecond_setHour},
			{"getMinute", "()J", Java_com_sreal_midifile_HourMinuteSecond_getMinute},
			{"setMinute", "(J)V", Java_com_sreal_midifile_HourMinuteSecond_setMinute},
			{"getSecond", "()F", Java_com_sreal_midifile_HourMinuteSecond_getSecond},
			{"setSecond", "(F)V", Java_com_sreal_midifile_HourMinuteSecond_setSecond},
			{"toString", "()Ljava/lang/String;", Java_com_sreal_midifile_HourMinuteSecond_toString},
			{"parse", "(Ljava/lang/String;)V", Java_com_sreal_midifile_HourMinuteSecond_parse}
		};

		jclass jclass_ = (*env)->FindClass(env, "com/sreal/midifile/HourMinuteSecond");
		(*env)->RegisterNatives(env, jclass_, native_methods, sizeof(native_methods) / sizeof(JNINativeMethod));
	}

	{
		JNINativeMethod native_methods[] = {
			{"init", "()V", Java_com_sreal_midifile_HourMinuteSecondFrame_init},
			{"finalize", "()V", Java_com_sreal_midifile_HourMinuteSecondFrame_finalize},
			{"getHour", "()J", Java_com_sreal_midifile_HourMinuteSecondFrame_getHour},
			{"setHour", "(J)V", Java_com_sreal_midifile_HourMinuteSecondFrame_setHour},
			{"getMinute", "()J", Java_com_sreal_midifile_HourMinuteSecondFrame_getMinute},
			{"setMinute", "(J)V", Java_com_sreal_midifile_HourMinuteSecondFrame_setMinute},
			{"getSecond", "()J", Java_com_sreal_midifile_HourMinuteSecondFrame_getSecond},
			{"setSecond", "(J)V", Java_com_sreal_midifile_HourMinuteSecondFrame_setSecond},
			{"getFrame", "()F", Java_com_sreal_midifile_HourMinuteSecondFrame_getFrame},
			{"setFrame", "(F)V", Java_com_sreal_midifile_HourMinuteSecondFrame_setFrame},
			{"toString", "()Ljava/lang/String;", Java_com_sreal_midifile_HourMinuteSecondFrame_toString},
			{"parse", "(Ljava/lang/String;)V", Java_com_sreal_midifile_HourMinuteSecondFrame_parse}
		};

		jclass jclass_ = (*env)->FindClass(env, "com/sreal/midifile/HourMinuteSecondFrame");
		(*env)->RegisterNatives(env, jclass_, native_methods, sizeof(native_methods) / sizeof(JNINativeMethod));
	}

	return JNI_VERSION_1_2;
}

