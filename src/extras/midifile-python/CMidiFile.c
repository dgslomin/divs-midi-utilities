
#include <Python.h>
#include <midifile.h>

static void event_visitor_callback_adapter(MidiFileEvent_t event, void *user_data)
{
	PyObject *visitor_callback_pyobject;
	PyObject *event_pyobject;
	PyObject *args_pyobject;

	visitor_callback_pyobject = (PyObject *)(user_data);
	event_pyobject = PyLong_FromVoidPtr(event);
	args_pyobject = Py_BuildValue("(O)", event_pyobject);
	PyEval_CallObject(visitor_callback_pyobject, args_pyobject);
	Py_XDECREF(args_pyobject);
	Py_XDECREF(event_pyobject);
}

static PyObject *CMidiFile_load(PyObject *self, PyObject *args)
{
	char *filename;
	MidiFile_t midi_file;
	PyObject *midi_file_pyobject;

	if (!PyArg_ParseTuple(args, "s", &filename)) return NULL;
	Py_BEGIN_ALLOW_THREADS
	midi_file = MidiFile_load(filename);
	Py_END_ALLOW_THREADS
	midi_file_pyobject = PyLong_FromVoidPtr(midi_file);
	return midi_file_pyobject;
}

static PyObject *CMidiFile_save(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	char *filename;
	MidiFile_t midi_file;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &midi_file_pyobject, &filename)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	Py_BEGIN_ALLOW_THREADS
	res = MidiFile_save(midi_file, filename);
	Py_END_ALLOW_THREADS
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_new(PyObject *self, PyObject *args)
{
	int file_format;
	MidiFileDivisionType_t division_type;
	int resolution;
	MidiFile_t midi_file;
	PyObject *midi_file_pyobject;

	if (!PyArg_ParseTuple(args, "iii", &file_format, &division_type, &resolution)) return NULL;
	midi_file = MidiFile_new(file_format, division_type, resolution);
	midi_file_pyobject = PyLong_FromVoidPtr(midi_file);
	return midi_file_pyobject;
}

static PyObject *CMidiFile_free(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	res = MidiFile_free(midi_file);
	Py_DECREF(midi_file_pyobject);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getFileFormat(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	int file_format;
	PyObject *file_format_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	file_format = MidiFile_getFileFormat(midi_file);
	file_format_pyobject = PyInt_FromLong(file_format);
	return file_format_pyobject;
}

static PyObject *CMidiFile_setFileFormat(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	int file_format;
	MidiFile_t midi_file;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &midi_file_pyobject, &file_format)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	res = MidiFile_setFileFormat(midi_file, file_format);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getDivisionType(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	MidiFileDivisionType_t division_type;
	PyObject *division_type_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	division_type = MidiFile_getDivisionType(midi_file);
	division_type_pyobject = PyInt_FromLong(division_type);
	return division_type_pyobject;
}

static PyObject *CMidiFile_setDivisionType(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFileDivisionType_t division_type;
	MidiFile_t midi_file;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &midi_file_pyobject, &division_type)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	res = MidiFile_setDivisionType(midi_file, division_type);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getResolution(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	int resolution;
	PyObject *resolution_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	resolution = MidiFile_getResolution(midi_file);
	resolution_pyobject = PyInt_FromLong(resolution);
	return resolution_pyobject;
}

static PyObject *CMidiFile_setResolution(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	int resolution;
	MidiFile_t midi_file;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &midi_file_pyobject, &resolution)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	res = MidiFile_setResolution(midi_file, resolution);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getNumberOfTicksPerBeat(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	int number_of_ticks_per_beat;
	PyObject *number_of_ticks_per_beat_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	number_of_ticks_per_beat = MidiFile_getNumberOfTicksPerBeat(midi_file);
	number_of_ticks_per_beat_pyobject = PyInt_FromLong(number_of_ticks_per_beat);
	return number_of_ticks_per_beat_pyobject;
}

static PyObject *CMidiFile_setNumberOfTicksPerBeat(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	int number_of_ticks_per_beat;
	MidiFile_t midi_file;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &midi_file_pyobject, &number_of_ticks_per_beat)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	res = MidiFile_setNumberOfTicksPerBeat(midi_file, number_of_ticks_per_beat);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getNumberOfFramesPerSecond(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	float number_of_frames_per_second;
	PyObject *number_of_frames_per_second_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	number_of_frames_per_second = MidiFile_getNumberOfFramesPerSecond(midi_file);
	number_of_frames_per_second_pyobject = PyFloat_FromDouble(number_of_frames_per_second);
	return number_of_frames_per_second_pyobject;
}

static PyObject *CMidiFile_setNumberOfFramesPerSecond(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	float number_of_frames_per_second;
	MidiFile_t midi_file;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Of", &midi_file_pyobject, &number_of_frames_per_second)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	res = MidiFile_setNumberOfFramesPerSecond(midi_file, number_of_frames_per_second);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_createTrack(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	MidiFileTrack_t midi_file_track;
	PyObject *midi_file_track_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	midi_file_track = MidiFile_createTrack(midi_file);
	midi_file_track_pyobject = PyLong_FromVoidPtr(midi_file_track);
	return midi_file_track_pyobject;
}

static PyObject *CMidiFile_getNumberOfTracks(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	int number_of_tracks;
	PyObject *number_of_tracks_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	number_of_tracks = MidiFile_getNumberOfTracks(midi_file);
	number_of_tracks_pyobject = PyInt_FromLong(number_of_tracks);
	return number_of_tracks_pyobject;
}

static PyObject *CMidiFile_getTrackByNumber(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	int number;
	int create;
	MidiFile_t midi_file;
	MidiFileTrack_t midi_file_track;
	PyObject *midi_file_track_pyobject;

	if (!PyArg_ParseTuple(args, "Oii", &midi_file_pyobject, &number, &create)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	midi_file_track = MidiFile_getTrackByNumber(midi_file, number, create);
	midi_file_track_pyobject = PyLong_FromVoidPtr(midi_file_track);
	return midi_file_track_pyobject;
}

static PyObject *CMidiFile_getFirstTrack(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	MidiFileTrack_t midi_file_track;
	PyObject *midi_file_track_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	midi_file_track = MidiFile_getFirstTrack(midi_file);
	midi_file_track_pyobject = PyLong_FromVoidPtr(midi_file_track);
	return midi_file_track_pyobject;
}

static PyObject *CMidiFile_getLastTrack(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	MidiFileTrack_t midi_file_track;
	PyObject *midi_file_track_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	midi_file_track = MidiFile_getLastTrack(midi_file);
	midi_file_track_pyobject = PyLong_FromVoidPtr(midi_file_track);
	return midi_file_track_pyobject;
}

static PyObject *CMidiFile_getFirstEvent(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	MidiFileEvent_t midi_file_event;
	PyObject *midi_file_event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	midi_file_event = MidiFile_getFirstEvent(midi_file);
	midi_file_event_pyobject = PyLong_FromVoidPtr(midi_file_event);
	return midi_file_event_pyobject;
}

static PyObject *CMidiFile_getLastEvent(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	MidiFile_t midi_file;
	MidiFileEvent_t midi_file_event;
	PyObject *midi_file_event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &midi_file_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	midi_file_event = MidiFile_getLastEvent(midi_file);
	midi_file_event_pyobject = PyLong_FromVoidPtr(midi_file_event);
	return midi_file_event_pyobject;
}

static PyObject *CMidiFile_visitEvents(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	PyObject *visitor_callback_pyobject;
	MidiFile_t midi_file;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "OO", &midi_file_pyobject, &visitor_callback_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	res = MidiFile_visitEvents(midi_file, event_visitor_callback_adapter, visitor_callback_pyobject);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getBeatFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	MidiFile_t midi_file;
	float beat;
	PyObject *beat_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &midi_file_pyobject, &tick)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	beat = MidiFile_getBeatFromTick(midi_file, tick);
	beat_pyobject = PyFloat_FromDouble(beat);
	return beat_pyobject;
}

static PyObject *CMidiFile_getTickFromBeat(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	float beat;
	MidiFile_t midi_file;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "Of", &midi_file_pyobject, &beat)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	tick = MidiFile_getTickFromBeat(midi_file, beat);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_getTimeFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	MidiFile_t midi_file;
	float time;
	PyObject *time_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &midi_file_pyobject, &tick)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	time = MidiFile_getTimeFromTick(midi_file, tick);
	time_pyobject = PyFloat_FromDouble(time);
	return time_pyobject;
}

static PyObject *CMidiFile_getTickFromTime(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	float time;
	MidiFile_t midi_file;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "Of", &midi_file_pyobject, &time)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	tick = MidiFile_getTickFromTime(midi_file, time);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_setMeasureBeatFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	PyObject *measure_beat_pyobject;
	MidiFile_t midi_file;
	MidiFileMeasureBeat_t measure_beat;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "OlO", &midi_file_pyobject, &tick, &measure_beat_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	res = MidiFile_setMeasureBeatFromTick(midi_file, tick, measure_beat);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getTickFromMeasureBeat(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	PyObject *measure_beat_pyobject;
	MidiFile_t midi_file;
	MidiFileMeasureBeat_t measure_beat;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "OO", &midi_file_pyobject, &measure_beat_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	tick = MidiFile_getTickFromMeasureBeat(midi_file, measure_beat);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_setMeasureBeatTickFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	PyObject *measure_beat_tick_pyobject;
	MidiFile_t midi_file;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "OlO", &midi_file_pyobject, &tick, &measure_beat_tick_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	res = MidiFile_setMeasureBeatTickFromTick(midi_file, tick, measure_beat_tick);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getTickFromMeasureBeatTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	PyObject *measure_beat_tick_pyobject;
	MidiFile_t midi_file;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "OO", &midi_file_pyobject, &measure_beat_tick_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	tick = MidiFile_getTickFromMeasureBeatTick(midi_file, measure_beat_tick);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_setHourMinuteSecondFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	PyObject *hour_minute_second_pyobject;
	MidiFile_t midi_file;
	MidiFileHourMinuteSecond_t hour_minute_second;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "OlO", &midi_file_pyobject, &tick, &hour_minute_second_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	res = MidiFile_setHourMinuteSecondFromTick(midi_file, tick, hour_minute_second);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getTickFromHourMinuteSecond(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	PyObject *hour_minute_second_pyobject;
	MidiFile_t midi_file;
	MidiFileHourMinuteSecond_t hour_minute_second;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "OO", &midi_file_pyobject, &hour_minute_second_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	tick = MidiFile_getTickFromHourMinuteSecond(midi_file, hour_minute_second);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_setHourMinuteSecondFrameFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	PyObject *hour_minute_second_frame_pyobject;
	MidiFile_t midi_file;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "OlO", &midi_file_pyobject, &tick, &hour_minute_second_frame_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	res = MidiFile_setHourMinuteSecondFrameFromTick(midi_file, tick, hour_minute_second_frame);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFile_getTickFromHourMinuteSecondFrame(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	PyObject *hour_minute_second_frame_pyobject;
	MidiFile_t midi_file;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "OO", &midi_file_pyobject, &hour_minute_second_frame_pyobject)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	tick = MidiFile_getTickFromHourMinuteSecondFrame(midi_file, hour_minute_second_frame);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_getMeasureBeatStringFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	MidiFile_t midi_file;
	char *measure_beat_string;
	PyObject *measure_beat_string_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &midi_file_pyobject, &tick)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	measure_beat_string = MidiFile_getMeasureBeatStringFromTick(midi_file, tick);
	measure_beat_string_pyobject = Py_BuildValue("s", measure_beat_string);
	return measure_beat_string_pyobject;
}

static PyObject *CMidiFile_getTickFromMeasureBeatString(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	char *measure_beat_string;
	MidiFile_t midi_file;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &midi_file_pyobject, &measure_beat_string)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	tick = MidiFile_getTickFromMeasureBeatString(midi_file, measure_beat_string);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_getMeasureBeatTickStringFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	MidiFile_t midi_file;
	char *measure_beat_tick_string;
	PyObject *measure_beat_tick_string_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &midi_file_pyobject, &tick)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	measure_beat_tick_string = MidiFile_getMeasureBeatTickStringFromTick(midi_file, tick);
	measure_beat_tick_string_pyobject = Py_BuildValue("s", measure_beat_tick_string);
	return measure_beat_tick_string_pyobject;
}

static PyObject *CMidiFile_getTickFromMeasureBeatTickString(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	char *measure_beat_tick_string;
	MidiFile_t midi_file;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &midi_file_pyobject, &measure_beat_tick_string)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	tick = MidiFile_getTickFromMeasureBeatTickString(midi_file, measure_beat_tick_string);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_getHourMinuteSecondStringFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	MidiFile_t midi_file;
	char *hour_minute_second_string;
	PyObject *hour_minute_second_string_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &midi_file_pyobject, &tick)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	hour_minute_second_string = MidiFile_getHourMinuteSecondStringFromTick(midi_file, tick);
	hour_minute_second_string_pyobject = Py_BuildValue("s", hour_minute_second_string);
	return hour_minute_second_string_pyobject;
}

static PyObject *CMidiFile_getTickFromHourMinuteSecondString(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	char *hour_minute_second_string;
	MidiFile_t midi_file;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &midi_file_pyobject, &hour_minute_second_string)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	tick = MidiFile_getTickFromHourMinuteSecondString(midi_file, hour_minute_second_string);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_getHourMinuteSecondFrameStringFromTick(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	long tick;
	MidiFile_t midi_file;
	char *hour_minute_second_frame_string;
	PyObject *hour_minute_second_frame_string_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &midi_file_pyobject, &tick)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	hour_minute_second_frame_string = MidiFile_getHourMinuteSecondFrameStringFromTick(midi_file, tick);
	hour_minute_second_frame_string_pyobject = Py_BuildValue("s", hour_minute_second_frame_string);
	return hour_minute_second_frame_string_pyobject;
}

static PyObject *CMidiFile_getTickFromHourMinuteSecondFrameString(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	char *hour_minute_second_frame_string;
	MidiFile_t midi_file;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &midi_file_pyobject, &hour_minute_second_frame_string)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	tick = MidiFile_getTickFromHourMinuteSecondFrameString(midi_file, hour_minute_second_frame_string);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFile_getTickFromMarker(PyObject *self, PyObject *args)
{
	PyObject *midi_file_pyobject;
	char *marker;
	MidiFile_t midi_file;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &midi_file_pyobject, &marker)) return NULL;
	midi_file = (MidiFile_t)(PyLong_AsVoidPtr(midi_file_pyobject));
	tick = MidiFile_getTickFromMarker(midi_file, marker);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFileTrack_delete(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	res = MidiFileTrack_delete(track);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileTrack_getMidiFile(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	MidiFile_t midi_file;
	PyObject *midi_file_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	midi_file = MidiFileTrack_getMidiFile(track);
	midi_file_pyobject = PyLong_FromVoidPtr(midi_file);
	return midi_file_pyobject;
}

static PyObject *CMidiFileTrack_getNumber(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	int number;
	PyObject *number_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	number = MidiFileTrack_getNumber(track);
	number_pyobject = PyInt_FromLong(number);
	return number_pyobject;
}

static PyObject *CMidiFileTrack_getEndTick(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	long end_tick;
	PyObject *end_tick_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	end_tick = MidiFileTrack_getEndTick(track);
	end_tick_pyobject = PyLong_FromLong(end_tick);
	return end_tick_pyobject;
}

static PyObject *CMidiFileTrack_setEndTick(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long end_tick;
	MidiFileTrack_t track;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &track_pyobject, &end_tick)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	res = MidiFileTrack_setEndTick(track, end_tick);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileTrack_createTrackBefore(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	MidiFileTrack_t new_track;
	PyObject *new_track_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	new_track = MidiFileTrack_createTrackBefore(track);
	new_track_pyobject = PyLong_FromVoidPtr(new_track);
	return new_track_pyobject;
}

static PyObject *CMidiFileTrack_getPreviousTrack(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	MidiFileTrack_t previous_track;
	PyObject *previous_track_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	previous_track = MidiFileTrack_getPreviousTrack(track);
	previous_track_pyobject = PyLong_FromVoidPtr(previous_track);
	return previous_track_pyobject;
}

static PyObject *CMidiFileTrack_getNextTrack(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	MidiFileTrack_t next_track;
	PyObject *next_track_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	next_track = MidiFileTrack_getNextTrack(track);
	next_track_pyobject = PyLong_FromVoidPtr(next_track);
	return next_track_pyobject;
}

static PyObject *CMidiFileTrack_createNoteOffEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int channel;
	int note;
	int velocity;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Oliii", &track_pyobject, &tick, &channel, &note, &velocity)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createNoteOffEvent(track, tick, channel, note, velocity);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createNoteOnEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int channel;
	int note;
	int velocity;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Oliii", &track_pyobject, &tick, &channel, &note, &velocity)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createNoteOnEvent(track, tick, channel, note, velocity);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createKeyPressureEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int channel;
	int note;
	int amount;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Oliii", &track_pyobject, &tick, &channel, &note, &amount)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createKeyPressureEvent(track, tick, channel, note, amount);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createControlChangeEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int channel;
	int number;
	int value;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Oliii", &track_pyobject, &tick, &channel, &number, &value)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createControlChangeEvent(track, tick, channel, number, value);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createProgramChangeEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int channel;
	int number;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Olii", &track_pyobject, &tick, &channel, &number)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createProgramChangeEvent(track, tick, channel, number);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createChannelPressureEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int channel;
	int amount;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Olii", &track_pyobject, &tick, &channel, &amount)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createChannelPressureEvent(track, tick, channel, amount);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createPitchWheelEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int channel;
	int value;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Olii", &track_pyobject, &tick, &channel, &value)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createPitchWheelEvent(track, tick, channel, value);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createSysexEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int data_length;
	unsigned char *data_buffer;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Ols#", &track_pyobject, &tick, &data_buffer, &data_length)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createSysexEvent(track, tick, data_length, data_buffer);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createMetaEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int number;
	int data_length;
	unsigned char *data_buffer;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Olis#", &track_pyobject, &tick, &number, &data_buffer, &data_length)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createMetaEvent(track, tick, number, data_length, data_buffer);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createNoteStartAndEndEvents(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long start_tick;
	long end_tick;
	int channel;
	int note;
	int start_velocity;
	int end_velocity;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Olliiii", &track_pyobject, &start_tick, &end_tick, &channel, &note, &start_velocity, &end_velocity)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createNoteStartAndEndEvents(track, start_tick, end_tick, channel, note, start_velocity, end_velocity);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createTempoEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	float tempo;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Olf", &track_pyobject, &tick, &tempo)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createTempoEvent(track, tick, tempo);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createTimeSignatureEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	int numerator;
	int denominator;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Olii", &track_pyobject, &tick, &numerator, &denominator)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createTimeSignatureEvent(track, tick, numerator, denominator);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createLyricEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	char *text;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Ols", &track_pyobject, &tick, &text)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createLyricEvent(track, tick, text);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createMarkerEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	char *text;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Ols", &track_pyobject, &tick, &text)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createMarkerEvent(track, tick, text);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_createVoiceEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	long tick;
	unsigned long data;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "Olk", &track_pyobject, &tick, &data)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_createVoiceEvent(track, tick, data);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_getFirstEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_getFirstEvent(track);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_getLastEvent(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	MidiFileTrack_t track;
	MidiFileEvent_t event;
	PyObject *event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &track_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	event = MidiFileTrack_getLastEvent(track);
	event_pyobject = PyLong_FromVoidPtr(event);
	return event_pyobject;
}

static PyObject *CMidiFileTrack_visitEvents(PyObject *self, PyObject *args)
{
	PyObject *track_pyobject;
	PyObject *visitor_callback_pyobject;
	MidiFileTrack_t track;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "OO", &track_pyobject, &visitor_callback_pyobject)) return NULL;
	track = (MidiFileTrack_t)(PyLong_AsVoidPtr(track_pyobject));
	res = MidiFileTrack_visitEvents(track, event_visitor_callback_adapter, visitor_callback_pyobject);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_delete(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_delete(event);
	res_pyobject = PyLong_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_getTrack(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	MidiFileTrack_t track;
	PyObject *track_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	track = MidiFileEvent_getTrack(event);
	track_pyobject = PyLong_FromVoidPtr(track);
	return track_pyobject;
}

static PyObject *CMidiFileEvent_getPreviousEventInTrack(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	MidiFileEvent_t previous_event;
	PyObject *previous_event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	previous_event = MidiFileEvent_getPreviousEventInTrack(event);
	previous_event_pyobject = PyLong_FromVoidPtr(previous_event);
	return previous_event_pyobject;
}

static PyObject *CMidiFileEvent_getNextEventInTrack(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	MidiFileEvent_t next_event;
	PyObject *next_event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	next_event = MidiFileEvent_getNextEventInTrack(event);
	next_event_pyobject = PyLong_FromVoidPtr(next_event);
	return next_event_pyobject;
}

static PyObject *CMidiFileEvent_getPreviousEventInFile(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	MidiFileEvent_t previous_event;
	PyObject *previous_event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	previous_event = MidiFileEvent_getPreviousEventInFile(event);
	previous_event_pyobject = PyLong_FromVoidPtr(previous_event);
	return previous_event_pyobject;
}

static PyObject *CMidiFileEvent_getNextEventInFile(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	MidiFileEvent_t next_event;
	PyObject *next_event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	next_event = MidiFileEvent_getNextEventInFile(event);
	next_event_pyobject = PyLong_FromVoidPtr(next_event);
	return next_event_pyobject;
}

static PyObject *CMidiFileEvent_getTick(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	long tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	tick = MidiFileEvent_getTick(event);
	tick_pyobject = PyLong_FromLong(tick);
	return tick_pyobject;
}

static PyObject *CMidiFileEvent_setTick(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	long tick;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &event_pyobject, &tick)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_setTick(event, tick);
	res_pyobject = PyLong_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_getType(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	MidiFileEventType_t type;
	PyObject *type_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	type = MidiFileEvent_getType(event);
	type_pyobject = PyInt_FromLong(type);
	return type_pyobject;
}

static PyObject *CMidiFileEvent_isNoteStartEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_isNoteStartEvent(event);
	res_pyobject = PyBool_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_isNoteEndEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_isNoteEndEvent(event);
	res_pyobject = PyBool_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_isTempoEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_isTempoEvent(event);
	res_pyobject = PyBool_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_isTimeSignatureEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_isTimeSignatureEvent(event);
	res_pyobject = PyBool_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_isLyricEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_isLyricEvent(event);
	res_pyobject = PyBool_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_isMarkerEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_isMarkerEvent(event);
	res_pyobject = PyBool_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileEvent_isVoiceEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileEvent_isVoiceEvent(event);
	res_pyobject = PyBool_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteOffEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFileNoteOffEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFileNoteOffEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteOffEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteOffEvent_getNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int note;
	PyObject *note_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	note = MidiFileNoteOffEvent_getNote(event);
	note_pyobject = PyInt_FromLong(note);
	return note_pyobject;
}

static PyObject *CMidiFileNoteOffEvent_setNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int note;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &note)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteOffEvent_setNote(event, note);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteOffEvent_getVelocity(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int velocity;
	PyObject *velocity_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	velocity = MidiFileNoteOffEvent_getVelocity(event);
	velocity_pyobject = PyInt_FromLong(velocity);
	return velocity_pyobject;
}

static PyObject *CMidiFileNoteOffEvent_setVelocity(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int velocity;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &velocity)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteOffEvent_setVelocity(event, velocity);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteOnEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFileNoteOnEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFileNoteOnEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteOnEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteOnEvent_getNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int note;
	PyObject *note_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	note = MidiFileNoteOnEvent_getNote(event);
	note_pyobject = PyInt_FromLong(note);
	return note_pyobject;
}

static PyObject *CMidiFileNoteOnEvent_setNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int note;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &note)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteOnEvent_setNote(event, note);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteOnEvent_getVelocity(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int velocity;
	PyObject *velocity_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	velocity = MidiFileNoteOnEvent_getVelocity(event);
	velocity_pyobject = PyInt_FromLong(velocity);
	return velocity_pyobject;
}

static PyObject *CMidiFileNoteOnEvent_setVelocity(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int velocity;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &velocity)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteOnEvent_setVelocity(event, velocity);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileKeyPressureEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFileKeyPressureEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFileKeyPressureEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileKeyPressureEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileKeyPressureEvent_getNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int note;
	PyObject *note_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	note = MidiFileKeyPressureEvent_getNote(event);
	note_pyobject = PyInt_FromLong(note);
	return note_pyobject;
}

static PyObject *CMidiFileKeyPressureEvent_setNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int note;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &note)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileKeyPressureEvent_setNote(event, note);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileKeyPressureEvent_getAmount(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int amount;
	PyObject *amount_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	amount = MidiFileKeyPressureEvent_getAmount(event);
	amount_pyobject = PyInt_FromLong(amount);
	return amount_pyobject;
}

static PyObject *CMidiFileKeyPressureEvent_setAmount(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int amount;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &amount)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileKeyPressureEvent_setAmount(event, amount);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileControlChangeEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFileControlChangeEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFileControlChangeEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileControlChangeEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileControlChangeEvent_getNumber(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int number;
	PyObject *number_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	number = MidiFileControlChangeEvent_getNumber(event);
	number_pyobject = PyInt_FromLong(number);
	return number_pyobject;
}

static PyObject *CMidiFileControlChangeEvent_setNumber(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int number;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &number)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileControlChangeEvent_setNumber(event, number);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileControlChangeEvent_getValue(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int value;
	PyObject *value_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	value = MidiFileControlChangeEvent_getValue(event);
	value_pyobject = PyInt_FromLong(value);
	return value_pyobject;
}

static PyObject *CMidiFileControlChangeEvent_setValue(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int value;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &value)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileControlChangeEvent_setValue(event, value);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileProgramChangeEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFileProgramChangeEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFileProgramChangeEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileProgramChangeEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileProgramChangeEvent_getNumber(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int number;
	PyObject *number_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	number = MidiFileProgramChangeEvent_getNumber(event);
	number_pyobject = PyInt_FromLong(number);
	return number_pyobject;
}

static PyObject *CMidiFileProgramChangeEvent_setNumber(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int number;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &number)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileProgramChangeEvent_setNumber(event, number);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileChannelPressureEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFileChannelPressureEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFileChannelPressureEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileChannelPressureEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileChannelPressureEvent_getAmount(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int amount;
	PyObject *amount_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	amount = MidiFileChannelPressureEvent_getAmount(event);
	amount_pyobject = PyInt_FromLong(amount);
	return amount_pyobject;
}

static PyObject *CMidiFileChannelPressureEvent_setAmount(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int amount;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &amount)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileChannelPressureEvent_setAmount(event, amount);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFilePitchWheelEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFilePitchWheelEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFilePitchWheelEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFilePitchWheelEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFilePitchWheelEvent_getValue(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int value;
	PyObject *value_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	value = MidiFilePitchWheelEvent_getValue(event);
	value_pyobject = PyInt_FromLong(value);
	return value_pyobject;
}

static PyObject *CMidiFilePitchWheelEvent_setValue(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int value;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &value)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFilePitchWheelEvent_setValue(event, value);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileSysexEvent_getData(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int data_length;
	unsigned char *data_buffer;
	PyObject *data_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	data_length = MidiFileSysexEvent_getDataLength(event);
	data_buffer = MidiFileSysexEvent_getData(event);
	data_pyobject = Py_BuildValue("s#", data_buffer, data_length);
	return data_pyobject;
}

static PyObject *CMidiFileSysexEvent_setData(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int data_length;
	unsigned char *data_buffer;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os#", &event_pyobject, &data_buffer, &data_length)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileSysexEvent_setData(event, data_length, data_buffer);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMetaEvent_getNumber(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int number;
	PyObject *number_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	number = MidiFileMetaEvent_getNumber(event);
	number_pyobject = PyInt_FromLong(number);
	return number_pyobject;
}

static PyObject *CMidiFileMetaEvent_setNumber(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int number;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &number)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileMetaEvent_setNumber(event, number);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMetaEvent_getData(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int data_length;
	unsigned char *data_buffer;
	PyObject *data_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	data_length = MidiFileMetaEvent_getDataLength(event);
	data_buffer = MidiFileMetaEvent_getData(event);
	data_pyobject = Py_BuildValue("s#", data_buffer, data_length);
	return data_pyobject;
}

static PyObject *CMidiFileMetaEvent_setData(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int data_length;
	unsigned char *data_buffer;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os#", &event_pyobject, &data_buffer, &data_length)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileMetaEvent_setData(event, data_length, data_buffer);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteStartEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFileNoteStartEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFileNoteStartEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteStartEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteStartEvent_getNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int note;
	PyObject *note_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	note = MidiFileNoteStartEvent_getNote(event);
	note_pyobject = PyInt_FromLong(note);
	return note_pyobject;
}

static PyObject *CMidiFileNoteStartEvent_setNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int note;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &note)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteStartEvent_setNote(event, note);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteStartEvent_getVelocity(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int velocity;
	PyObject *velocity_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	velocity = MidiFileNoteStartEvent_getVelocity(event);
	velocity_pyobject = PyInt_FromLong(velocity);
	return velocity_pyobject;
}

static PyObject *CMidiFileNoteStartEvent_setVelocity(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int velocity;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &velocity)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteStartEvent_setVelocity(event, velocity);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteStartEvent_getNoteEndEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	MidiFileEvent_t note_end_event;
	PyObject *note_end_event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	note_end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
	note_end_event_pyobject = PyLong_FromVoidPtr(note_end_event);
	return note_end_event_pyobject;
}

static PyObject *CMidiFileNoteEndEvent_getChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int channel;
	PyObject *channel_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	channel = MidiFileNoteEndEvent_getChannel(event);
	channel_pyobject = PyInt_FromLong(channel);
	return channel_pyobject;
}

static PyObject *CMidiFileNoteEndEvent_setChannel(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int channel;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &channel)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteEndEvent_setChannel(event, channel);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteEndEvent_getNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int note;
	PyObject *note_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	note = MidiFileNoteEndEvent_getNote(event);
	note_pyobject = PyInt_FromLong(note);
	return note_pyobject;
}

static PyObject *CMidiFileNoteEndEvent_setNote(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int note;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &note)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteEndEvent_setNote(event, note);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteEndEvent_getVelocity(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int velocity;
	PyObject *velocity_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	velocity = MidiFileNoteEndEvent_getVelocity(event);
	velocity_pyobject = PyInt_FromLong(velocity);
	return velocity_pyobject;
}

static PyObject *CMidiFileNoteEndEvent_setVelocity(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int velocity;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oi", &event_pyobject, &velocity)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileNoteEndEvent_setVelocity(event, velocity);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileNoteEndEvent_getNoteStartEvent(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	MidiFileEvent_t note_end_event;
	PyObject *note_end_event_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	note_end_event = MidiFileNoteEndEvent_getNoteStartEvent(event);
	note_end_event_pyobject = PyLong_FromVoidPtr(note_end_event);
	return note_end_event_pyobject;
}

static PyObject *CMidiFileTempoEvent_getTempo(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	float tempo;
	PyObject *tempo_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	tempo = MidiFileTempoEvent_getTempo(event);
	tempo_pyobject = PyFloat_FromDouble(tempo);
	return tempo_pyobject;
}

static PyObject *CMidiFileTempoEvent_setTempo(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	float tempo;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Of", &event_pyobject, &tempo)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileTempoEvent_setTempo(event, tempo);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileTimeSignatureEvent_getNumerator(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int numerator;
	PyObject *numerator_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	numerator = MidiFileTimeSignatureEvent_getNumerator(event);
	numerator_pyobject = PyInt_FromLong(numerator);
	return numerator_pyobject;
}

static PyObject *CMidiFileTimeSignatureEvent_getDenominator(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	int denominator;
	PyObject *denominator_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	denominator = MidiFileTimeSignatureEvent_getDenominator(event);
	denominator_pyobject = PyInt_FromLong(denominator);
	return denominator_pyobject;
}

static PyObject *CMidiFileTimeSignatureEvent_setTimeSignature(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	int numerator;
	int denominator;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Oii", &event_pyobject, &numerator, &denominator)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileTimeSignatureEvent_setTimeSignature(event, numerator, denominator);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileLyricEvent_getText(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	char *text;
	PyObject *text_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	text = MidiFileLyricEvent_getText(event);
	text_pyobject = Py_BuildValue("s", text);
	return text_pyobject;
}

static PyObject *CMidiFileLyricEvent_setText(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	char *text;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &event_pyobject, &text)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileLyricEvent_setText(event, text);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMarkerEvent_getText(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	char *text;
	PyObject *text_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	text = MidiFileMarkerEvent_getText(event);
	text_pyobject = Py_BuildValue("s", text);
	return text_pyobject;
}

static PyObject *CMidiFileMarkerEvent_setText(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	char *text;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &event_pyobject, &text)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileMarkerEvent_setText(event, text);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileVoiceEvent_getData(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	MidiFileEvent_t event;
	unsigned long data;
	PyObject *data_pyobject;

	if (!PyArg_ParseTuple(args, "O", &event_pyobject)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	data = MidiFileVoiceEvent_getData(event);
	data_pyobject = PyLong_FromUnsignedLong(data);
	return data_pyobject;
}

static PyObject *CMidiFileVoiceEvent_setData(PyObject *self, PyObject *args)
{
	PyObject *event_pyobject;
	unsigned long data;
	MidiFileEvent_t event;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ok", &event_pyobject, &data)) return NULL;
	event = (MidiFileEvent_t)(PyLong_AsVoidPtr(event_pyobject));
	res = MidiFileVoiceEvent_setData(event, data);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeat_new(PyObject *self, PyObject *args)
{
	MidiFileMeasureBeat_t measure_beat;
	PyObject *measure_beat_pyobject;

	if (!PyArg_ParseTuple(args, "")) return NULL;
	measure_beat = MidiFileMeasureBeat_new();
	measure_beat_pyobject = PyLong_FromVoidPtr(measure_beat);
	return measure_beat_pyobject;
}

static PyObject *CMidiFileMeasureBeat_free(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_pyobject;
	MidiFileMeasureBeat_t measure_beat;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_pyobject)) return NULL;
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	res = MidiFileMeasureBeat_free(measure_beat);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeat_getMeasure(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_pyobject;
	MidiFileMeasureBeat_t measure_beat;
	long measure;
	PyObject *measure_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_pyobject)) return NULL;
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	measure = MidiFileMeasureBeat_getMeasure(measure_beat);
	measure_pyobject = PyLong_FromLong(measure);
	return measure_pyobject;
}

static PyObject *CMidiFileMeasureBeat_setMeasure(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_pyobject;
	long measure;
	MidiFileMeasureBeat_t measure_beat;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &measure_beat_pyobject, &measure)) return NULL;
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	res = MidiFileMeasureBeat_setMeasure(measure_beat, measure);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeat_getBeat(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_pyobject;
	MidiFileMeasureBeat_t measure_beat;
	float beat;
	PyObject *beat_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_pyobject)) return NULL;
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	beat = MidiFileMeasureBeat_getBeat(measure_beat);
	beat_pyobject = PyFloat_FromDouble(beat);
	return beat_pyobject;
}

static PyObject *CMidiFileMeasureBeat_setBeat(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_pyobject;
	float beat;
	MidiFileMeasureBeat_t measure_beat;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Of", &measure_beat_pyobject, &beat)) return NULL;
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	res = MidiFileMeasureBeat_setBeat(measure_beat, beat);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeat_toString(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_pyobject;
	MidiFileMeasureBeat_t measure_beat;
	char *string;
	PyObject *string_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_pyobject)) return NULL;
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	string = MidiFileMeasureBeat_toString(measure_beat);
	string_pyobject = Py_BuildValue("s", string);
	return string_pyobject;
}

static PyObject *CMidiFileMeasureBeat_parse(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_pyobject;
	char *string;
	MidiFileMeasureBeat_t measure_beat;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &measure_beat_pyobject, &string)) return NULL;
	measure_beat = (MidiFileMeasureBeat_t)(PyLong_AsVoidPtr(measure_beat_pyobject));
	res = MidiFileMeasureBeat_parse(measure_beat, string);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_new(PyObject *self, PyObject *args)
{
	MidiFileMeasureBeatTick_t measure_beat_tick;
	PyObject *measure_beat_tick_pyobject;

	if (!PyArg_ParseTuple(args, "")) return NULL;
	measure_beat_tick = MidiFileMeasureBeatTick_new();
	measure_beat_tick_pyobject = PyLong_FromVoidPtr(measure_beat_tick);
	return measure_beat_tick_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_free(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_tick_pyobject)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	res = MidiFileMeasureBeatTick_free(measure_beat_tick);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_getMeasure(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	long measure;
	PyObject *measure_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_tick_pyobject)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	measure = MidiFileMeasureBeatTick_getMeasure(measure_beat_tick);
	measure_pyobject = PyLong_FromLong(measure);
	return measure_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_setMeasure(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	long measure;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &measure_beat_tick_pyobject, &measure)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	res = MidiFileMeasureBeatTick_setMeasure(measure_beat_tick, measure);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_getBeat(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	long beat;
	PyObject *beat_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_tick_pyobject)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	beat = MidiFileMeasureBeatTick_getBeat(measure_beat_tick);
	beat_pyobject = PyLong_FromLong(beat);
	return beat_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_setBeat(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	long beat;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &measure_beat_tick_pyobject, &beat)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	res = MidiFileMeasureBeatTick_setBeat(measure_beat_tick, beat);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_getTick(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	float tick;
	PyObject *tick_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_tick_pyobject)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	tick = MidiFileMeasureBeatTick_getTick(measure_beat_tick);
	tick_pyobject = PyFloat_FromDouble(tick);
	return tick_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_setTick(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	float tick;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Of", &measure_beat_tick_pyobject, &tick)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	res = MidiFileMeasureBeatTick_setTick(measure_beat_tick, tick);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_toString(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	char *string;
	PyObject *string_pyobject;

	if (!PyArg_ParseTuple(args, "O", &measure_beat_tick_pyobject)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	string = MidiFileMeasureBeatTick_toString(measure_beat_tick);
	string_pyobject = Py_BuildValue("s", string);
	return string_pyobject;
}

static PyObject *CMidiFileMeasureBeatTick_parse(PyObject *self, PyObject *args)
{
	PyObject *measure_beat_tick_pyobject;
	char *string;
	MidiFileMeasureBeatTick_t measure_beat_tick;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &measure_beat_tick_pyobject, &string)) return NULL;
	measure_beat_tick = (MidiFileMeasureBeatTick_t)(PyLong_AsVoidPtr(measure_beat_tick_pyobject));
	res = MidiFileMeasureBeatTick_parse(measure_beat_tick, string);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_new(PyObject *self, PyObject *args)
{
	MidiFileHourMinuteSecond_t hour_minute_second;
	PyObject *hour_minute_second_pyobject;

	if (!PyArg_ParseTuple(args, "")) return NULL;
	hour_minute_second = MidiFileHourMinuteSecond_new();
	hour_minute_second_pyobject = PyLong_FromVoidPtr(hour_minute_second);
	return hour_minute_second_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_free(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	MidiFileHourMinuteSecond_t hour_minute_second;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_pyobject)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	res = MidiFileHourMinuteSecond_free(hour_minute_second);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_getHour(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	MidiFileHourMinuteSecond_t hour_minute_second;
	long hour;
	PyObject *hour_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_pyobject)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	hour = MidiFileHourMinuteSecond_getHour(hour_minute_second);
	hour_pyobject = PyLong_FromLong(hour);
	return hour_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_setHour(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	long hour;
	MidiFileHourMinuteSecond_t hour_minute_second;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &hour_minute_second_pyobject, &hour)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	res = MidiFileHourMinuteSecond_setHour(hour_minute_second, hour);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_getMinute(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	MidiFileHourMinuteSecond_t hour_minute_second;
	long minute;
	PyObject *minute_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_pyobject)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	minute = MidiFileHourMinuteSecond_getMinute(hour_minute_second);
	minute_pyobject = PyLong_FromLong(minute);
	return minute_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_setMinute(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	long minute;
	MidiFileHourMinuteSecond_t hour_minute_second;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &hour_minute_second_pyobject, &minute)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	res = MidiFileHourMinuteSecond_setMinute(hour_minute_second, minute);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_getSecond(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	MidiFileHourMinuteSecond_t hour_minute_second;
	float second;
	PyObject *second_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_pyobject)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	second = MidiFileHourMinuteSecond_getSecond(hour_minute_second);
	second_pyobject = PyFloat_FromDouble(second);
	return second_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_setSecond(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	float second;
	MidiFileHourMinuteSecond_t hour_minute_second;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Of", &hour_minute_second_pyobject, &second)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	res = MidiFileHourMinuteSecond_setSecond(hour_minute_second, second);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_toString(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	MidiFileHourMinuteSecond_t hour_minute_second;
	char *string;
	PyObject *string_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_pyobject)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	string = MidiFileHourMinuteSecond_toString(hour_minute_second);
	string_pyobject = Py_BuildValue("s", string);
	return string_pyobject;
}

static PyObject *CMidiFileHourMinuteSecond_parse(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_pyobject;
	char *string;
	MidiFileHourMinuteSecond_t hour_minute_second;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &hour_minute_second_pyobject, &string)) return NULL;
	hour_minute_second = (MidiFileHourMinuteSecond_t)(PyLong_AsVoidPtr(hour_minute_second_pyobject));
	res = MidiFileHourMinuteSecond_parse(hour_minute_second, string);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_new(PyObject *self, PyObject *args)
{
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	PyObject *hour_minute_second_frame_pyobject;

	if (!PyArg_ParseTuple(args, "")) return NULL;
	hour_minute_second_frame = MidiFileHourMinuteSecondFrame_new();
	hour_minute_second_frame_pyobject = PyLong_FromVoidPtr(hour_minute_second_frame);
	return hour_minute_second_frame_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_free(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_frame_pyobject)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	res = MidiFileHourMinuteSecondFrame_free(hour_minute_second_frame);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_getHour(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	long hour;
	PyObject *hour_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_frame_pyobject)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	hour = MidiFileHourMinuteSecondFrame_getHour(hour_minute_second_frame);
	hour_pyobject = PyLong_FromLong(hour);
	return hour_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_setHour(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	long hour;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &hour_minute_second_frame_pyobject, &hour)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	res = MidiFileHourMinuteSecondFrame_setHour(hour_minute_second_frame, hour);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_getMinute(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	long minute;
	PyObject *minute_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_frame_pyobject)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	minute = MidiFileHourMinuteSecondFrame_getMinute(hour_minute_second_frame);
	minute_pyobject = PyLong_FromLong(minute);
	return minute_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_setMinute(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	long minute;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &hour_minute_second_frame_pyobject, &minute)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	res = MidiFileHourMinuteSecondFrame_setMinute(hour_minute_second_frame, minute);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_getSecond(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	long second;
	PyObject *second_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_frame_pyobject)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	second = MidiFileHourMinuteSecondFrame_getSecond(hour_minute_second_frame);
	second_pyobject = PyLong_FromLong(second);
	return second_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_setSecond(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	long second;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Ol", &hour_minute_second_frame_pyobject, &second)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	res = MidiFileHourMinuteSecondFrame_setSecond(hour_minute_second_frame, second);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_getFrame(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	float frame;
	PyObject *frame_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_frame_pyobject)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	frame = MidiFileHourMinuteSecondFrame_getFrame(hour_minute_second_frame);
	frame_pyobject = PyFloat_FromDouble(frame);
	return frame_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_setFrame(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	float frame;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Of", &hour_minute_second_frame_pyobject, &frame)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	res = MidiFileHourMinuteSecondFrame_setFrame(hour_minute_second_frame, frame);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_toString(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	char *string;
	PyObject *string_pyobject;

	if (!PyArg_ParseTuple(args, "O", &hour_minute_second_frame_pyobject)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	string = MidiFileHourMinuteSecondFrame_toString(hour_minute_second_frame);
	string_pyobject = Py_BuildValue("s", string);
	return string_pyobject;
}

static PyObject *CMidiFileHourMinuteSecondFrame_parse(PyObject *self, PyObject *args)
{
	PyObject *hour_minute_second_frame_pyobject;
	char *string;
	MidiFileHourMinuteSecondFrame_t hour_minute_second_frame;
	int res;
	PyObject *res_pyobject;

	if (!PyArg_ParseTuple(args, "Os", &hour_minute_second_frame_pyobject, &string)) return NULL;
	hour_minute_second_frame = (MidiFileHourMinuteSecondFrame_t)(PyLong_AsVoidPtr(hour_minute_second_frame_pyobject));
	res = MidiFileHourMinuteSecondFrame_parse(hour_minute_second_frame, string);
	res_pyobject = PyInt_FromLong(res);
	return res_pyobject;
}

static PyMethodDef CMidiFileMethods[] = {
	{"MidiFile_load", CMidiFile_load, METH_VARARGS, ""},
	{"MidiFile_save", CMidiFile_save, METH_VARARGS, ""},

	{"MidiFile_new", CMidiFile_new, METH_VARARGS, ""},
	{"MidiFile_free", CMidiFile_free, METH_VARARGS, ""},
	{"MidiFile_getFileFormat", CMidiFile_getFileFormat, METH_VARARGS, ""},
	{"MidiFile_setFileFormat", CMidiFile_setFileFormat, METH_VARARGS, ""},
	{"MidiFile_getDivisionType", CMidiFile_getDivisionType, METH_VARARGS, ""},
	{"MidiFile_setDivisionType", CMidiFile_setDivisionType, METH_VARARGS, ""},
	{"MidiFile_getResolution", CMidiFile_getResolution, METH_VARARGS, ""},
	{"MidiFile_setResolution", CMidiFile_setResolution, METH_VARARGS, ""},
	{"MidiFile_getNumberOfTicksPerBeat", CMidiFile_getNumberOfTicksPerBeat, METH_VARARGS, ""},
	{"MidiFile_setNumberOfTicksPerBeat", CMidiFile_setNumberOfTicksPerBeat, METH_VARARGS, ""},
	{"MidiFile_getNumberOfFramesPerSecond", CMidiFile_getNumberOfFramesPerSecond, METH_VARARGS, ""},
	{"MidiFile_setNumberOfFramesPerSecond", CMidiFile_setNumberOfFramesPerSecond, METH_VARARGS, ""},
	{"MidiFile_createTrack", CMidiFile_createTrack, METH_VARARGS, ""},
	{"MidiFile_getNumberOfTracks", CMidiFile_getNumberOfTracks, METH_VARARGS, ""},
	{"MidiFile_getTrackByNumber", CMidiFile_getTrackByNumber, METH_VARARGS, ""},
	{"MidiFile_getFirstTrack", CMidiFile_getFirstTrack, METH_VARARGS, ""},
	{"MidiFile_getLastTrack", CMidiFile_getLastTrack, METH_VARARGS, ""},
	{"MidiFile_getFirstEvent", CMidiFile_getFirstEvent, METH_VARARGS, ""},
	{"MidiFile_getLastEvent", CMidiFile_getLastEvent, METH_VARARGS, ""},
	{"MidiFile_visitEvents", CMidiFile_visitEvents, METH_VARARGS, ""},

	{"MidiFile_getBeatFromTick", CMidiFile_getBeatFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromBeat", CMidiFile_getTickFromBeat, METH_VARARGS, ""},
	{"MidiFile_getTimeFromTick", CMidiFile_getTimeFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromTime", CMidiFile_getTickFromTime, METH_VARARGS, ""},
	{"MidiFile_setMeasureBeatFromTick", CMidiFile_setMeasureBeatFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromMeasureBeat", CMidiFile_getTickFromMeasureBeat, METH_VARARGS, ""},
	{"MidiFile_setMeasureBeatTickFromTick", CMidiFile_setMeasureBeatTickFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromMeasureBeatTick", CMidiFile_getTickFromMeasureBeatTick, METH_VARARGS, ""},
	{"MidiFile_setHourMinuteSecondFromTick", CMidiFile_setHourMinuteSecondFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromHourMinuteSecond", CMidiFile_getTickFromHourMinuteSecond, METH_VARARGS, ""},
	{"MidiFile_setHourMinuteSecondFrameFromTick", CMidiFile_setHourMinuteSecondFrameFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromHourMinuteSecondFrame", CMidiFile_getTickFromHourMinuteSecondFrame, METH_VARARGS, ""},
	{"MidiFile_getMeasureBeatStringFromTick", CMidiFile_getMeasureBeatStringFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromMeasureBeatString", CMidiFile_getTickFromMeasureBeatString, METH_VARARGS, ""},
	{"MidiFile_getMeasureBeatTickStringFromTick", CMidiFile_getMeasureBeatTickStringFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromMeasureBeatTickString", CMidiFile_getTickFromMeasureBeatTickString, METH_VARARGS, ""},
	{"MidiFile_getHourMinuteSecondStringFromTick", CMidiFile_getHourMinuteSecondStringFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromHourMinuteSecondString", CMidiFile_getTickFromHourMinuteSecondString, METH_VARARGS, ""},
	{"MidiFile_getHourMinuteSecondFrameStringFromTick", CMidiFile_getHourMinuteSecondFrameStringFromTick, METH_VARARGS, ""},
	{"MidiFile_getTickFromHourMinuteSecondFrameString", CMidiFile_getTickFromHourMinuteSecondFrameString, METH_VARARGS, ""},
	{"MidiFile_getTickFromMarker", CMidiFile_getTickFromMarker, METH_VARARGS, ""},

	{"MidiFileTrack_delete", CMidiFileTrack_delete, METH_VARARGS, ""},
	{"MidiFileTrack_getMidiFile", CMidiFileTrack_getMidiFile, METH_VARARGS, ""},
	{"MidiFileTrack_getNumber", CMidiFileTrack_getNumber, METH_VARARGS, ""},
	{"MidiFileTrack_getEndTick", CMidiFileTrack_getEndTick, METH_VARARGS, ""},
	{"MidiFileTrack_setEndTick", CMidiFileTrack_setEndTick, METH_VARARGS, ""},
	{"MidiFileTrack_createTrackBefore", CMidiFileTrack_createTrackBefore, METH_VARARGS, ""},
	{"MidiFileTrack_getPreviousTrack", CMidiFileTrack_getPreviousTrack, METH_VARARGS, ""},
	{"MidiFileTrack_getNextTrack", CMidiFileTrack_getNextTrack, METH_VARARGS, ""},
	{"MidiFileTrack_createNoteOffEvent", CMidiFileTrack_createNoteOffEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createNoteOnEvent", CMidiFileTrack_createNoteOnEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createKeyPressureEvent", CMidiFileTrack_createKeyPressureEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createControlChangeEvent", CMidiFileTrack_createControlChangeEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createProgramChangeEvent", CMidiFileTrack_createProgramChangeEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createChannelPressureEvent", CMidiFileTrack_createChannelPressureEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createPitchWheelEvent", CMidiFileTrack_createPitchWheelEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createSysexEvent", CMidiFileTrack_createSysexEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createMetaEvent", CMidiFileTrack_createMetaEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createNoteStartAndEndEvents", CMidiFileTrack_createNoteStartAndEndEvents, METH_VARARGS, ""},
	{"MidiFileTrack_createTempoEvent", CMidiFileTrack_createTempoEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createTimeSignatureEvent", CMidiFileTrack_createTimeSignatureEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createLyricEvent", CMidiFileTrack_createLyricEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createMarkerEvent", CMidiFileTrack_createMarkerEvent, METH_VARARGS, ""},
	{"MidiFileTrack_createVoiceEvent", CMidiFileTrack_createVoiceEvent, METH_VARARGS, ""},
	{"MidiFileTrack_getFirstEvent", CMidiFileTrack_getFirstEvent, METH_VARARGS, ""},
	{"MidiFileTrack_getLastEvent", CMidiFileTrack_getLastEvent, METH_VARARGS, ""},
	{"MidiFileTrack_visitEvents", CMidiFileTrack_visitEvents, METH_VARARGS, ""},

	{"MidiFileEvent_delete", CMidiFileEvent_delete, METH_VARARGS, ""},
	{"MidiFileEvent_getTrack", CMidiFileEvent_getTrack, METH_VARARGS, ""},
	{"MidiFileEvent_getPreviousEventInTrack", CMidiFileEvent_getPreviousEventInTrack, METH_VARARGS, ""},
	{"MidiFileEvent_getNextEventInTrack", CMidiFileEvent_getNextEventInTrack, METH_VARARGS, ""},
	{"MidiFileEvent_getPreviousEventInFile", CMidiFileEvent_getPreviousEventInFile, METH_VARARGS, ""},
	{"MidiFileEvent_getNextEventInFile", CMidiFileEvent_getNextEventInFile, METH_VARARGS, ""},
	{"MidiFileEvent_getTick", CMidiFileEvent_getTick, METH_VARARGS, ""},
	{"MidiFileEvent_setTick", CMidiFileEvent_setTick, METH_VARARGS, ""},
	{"MidiFileEvent_getType", CMidiFileEvent_getType, METH_VARARGS, ""},
	{"MidiFileEvent_isNoteStartEvent", CMidiFileEvent_isNoteStartEvent, METH_VARARGS, ""},
	{"MidiFileEvent_isNoteEndEvent", CMidiFileEvent_isNoteEndEvent, METH_VARARGS, ""},
	{"MidiFileEvent_isTempoEvent", CMidiFileEvent_isTempoEvent, METH_VARARGS, ""},
	{"MidiFileEvent_isTimeSignatureEvent", CMidiFileEvent_isTimeSignatureEvent, METH_VARARGS, ""},
	{"MidiFileEvent_isLyricEvent", CMidiFileEvent_isLyricEvent, METH_VARARGS, ""},
	{"MidiFileEvent_isMarkerEvent", CMidiFileEvent_isMarkerEvent, METH_VARARGS, ""},
	{"MidiFileEvent_isVoiceEvent", CMidiFileEvent_isVoiceEvent, METH_VARARGS, ""},

	{"MidiFileNoteOffEvent_getChannel", CMidiFileNoteOffEvent_getChannel, METH_VARARGS, ""},
	{"MidiFileNoteOffEvent_setChannel", CMidiFileNoteOffEvent_setChannel, METH_VARARGS, ""},
	{"MidiFileNoteOffEvent_getNote", CMidiFileNoteOffEvent_getNote, METH_VARARGS, ""},
	{"MidiFileNoteOffEvent_setNote", CMidiFileNoteOffEvent_setNote, METH_VARARGS, ""},
	{"MidiFileNoteOffEvent_getVelocity", CMidiFileNoteOffEvent_getVelocity, METH_VARARGS, ""},
	{"MidiFileNoteOffEvent_setVelocity", CMidiFileNoteOffEvent_setVelocity, METH_VARARGS, ""},

	{"MidiFileNoteOnEvent_getChannel", CMidiFileNoteOnEvent_getChannel, METH_VARARGS, ""},
	{"MidiFileNoteOnEvent_setChannel", CMidiFileNoteOnEvent_setChannel, METH_VARARGS, ""},
	{"MidiFileNoteOnEvent_getNote", CMidiFileNoteOnEvent_getNote, METH_VARARGS, ""},
	{"MidiFileNoteOnEvent_setNote", CMidiFileNoteOnEvent_setNote, METH_VARARGS, ""},
	{"MidiFileNoteOnEvent_getVelocity", CMidiFileNoteOnEvent_getVelocity, METH_VARARGS, ""},
	{"MidiFileNoteOnEvent_setVelocity", CMidiFileNoteOnEvent_setVelocity, METH_VARARGS, ""},

	{"MidiFileKeyPressureEvent_getChannel", CMidiFileKeyPressureEvent_getChannel, METH_VARARGS, ""},
	{"MidiFileKeyPressureEvent_setChannel", CMidiFileKeyPressureEvent_setChannel, METH_VARARGS, ""},
	{"MidiFileKeyPressureEvent_getNote", CMidiFileKeyPressureEvent_getNote, METH_VARARGS, ""},
	{"MidiFileKeyPressureEvent_setNote", CMidiFileKeyPressureEvent_setNote, METH_VARARGS, ""},
	{"MidiFileKeyPressureEvent_getAmount", CMidiFileKeyPressureEvent_getAmount, METH_VARARGS, ""},
	{"MidiFileKeyPressureEvent_setAmount", CMidiFileKeyPressureEvent_setAmount, METH_VARARGS, ""},

	{"MidiFileControlChangeEvent_getChannel", CMidiFileControlChangeEvent_getChannel, METH_VARARGS, ""},
	{"MidiFileControlChangeEvent_setChannel", CMidiFileControlChangeEvent_setChannel, METH_VARARGS, ""},
	{"MidiFileControlChangeEvent_getNumber", CMidiFileControlChangeEvent_getNumber, METH_VARARGS, ""},
	{"MidiFileControlChangeEvent_setNumber", CMidiFileControlChangeEvent_setNumber, METH_VARARGS, ""},
	{"MidiFileControlChangeEvent_getValue", CMidiFileControlChangeEvent_getValue, METH_VARARGS, ""},
	{"MidiFileControlChangeEvent_setValue", CMidiFileControlChangeEvent_setValue, METH_VARARGS, ""},

	{"MidiFileProgramChangeEvent_getChannel", CMidiFileProgramChangeEvent_getChannel, METH_VARARGS, ""},
	{"MidiFileProgramChangeEvent_setChannel", CMidiFileProgramChangeEvent_setChannel, METH_VARARGS, ""},
	{"MidiFileProgramChangeEvent_getNumber", CMidiFileProgramChangeEvent_getNumber, METH_VARARGS, ""},
	{"MidiFileProgramChangeEvent_setNumber", CMidiFileProgramChangeEvent_setNumber, METH_VARARGS, ""},

	{"MidiFileChannelPressureEvent_getChannel", CMidiFileChannelPressureEvent_getChannel, METH_VARARGS, ""},
	{"MidiFileChannelPressureEvent_setChannel", CMidiFileChannelPressureEvent_setChannel, METH_VARARGS, ""},
	{"MidiFileChannelPressureEvent_getAmount", CMidiFileChannelPressureEvent_getAmount, METH_VARARGS, ""},
	{"MidiFileChannelPressureEvent_setAmount", CMidiFileChannelPressureEvent_setAmount, METH_VARARGS, ""},

	{"MidiFilePitchWheelEvent_getChannel", CMidiFilePitchWheelEvent_getChannel, METH_VARARGS, ""},
	{"MidiFilePitchWheelEvent_setChannel", CMidiFilePitchWheelEvent_setChannel, METH_VARARGS, ""},
	{"MidiFilePitchWheelEvent_getValue", CMidiFilePitchWheelEvent_getValue, METH_VARARGS, ""},
	{"MidiFilePitchWheelEvent_setValue", CMidiFilePitchWheelEvent_setValue, METH_VARARGS, ""},

	{"MidiFileSysexEvent_getData", CMidiFileSysexEvent_getData, METH_VARARGS, ""},
	{"MidiFileSysexEvent_setData", CMidiFileSysexEvent_setData, METH_VARARGS, ""},

	{"MidiFileMetaEvent_getNumber", CMidiFileMetaEvent_getNumber, METH_VARARGS, ""},
	{"MidiFileMetaEvent_setNumber", CMidiFileMetaEvent_setNumber, METH_VARARGS, ""},
	{"MidiFileMetaEvent_getData", CMidiFileMetaEvent_getData, METH_VARARGS, ""},
	{"MidiFileMetaEvent_setData", CMidiFileMetaEvent_setData, METH_VARARGS, ""},

	{"MidiFileNoteStartEvent_getChannel", CMidiFileNoteStartEvent_getChannel, METH_VARARGS, ""},
	{"MidiFileNoteStartEvent_setChannel", CMidiFileNoteStartEvent_setChannel, METH_VARARGS, ""},
	{"MidiFileNoteStartEvent_getNote", CMidiFileNoteStartEvent_getNote, METH_VARARGS, ""},
	{"MidiFileNoteStartEvent_setNote", CMidiFileNoteStartEvent_setNote, METH_VARARGS, ""},
	{"MidiFileNoteStartEvent_getVelocity", CMidiFileNoteStartEvent_getVelocity, METH_VARARGS, ""},
	{"MidiFileNoteStartEvent_setVelocity", CMidiFileNoteStartEvent_setVelocity, METH_VARARGS, ""},
	{"MidiFileNoteStartEvent_getNoteEndEvent", CMidiFileNoteStartEvent_getNoteEndEvent, METH_VARARGS, ""},

	{"MidiFileNoteEndEvent_getChannel", CMidiFileNoteEndEvent_getChannel, METH_VARARGS, ""},
	{"MidiFileNoteEndEvent_setChannel", CMidiFileNoteEndEvent_setChannel, METH_VARARGS, ""},
	{"MidiFileNoteEndEvent_getNote", CMidiFileNoteEndEvent_getNote, METH_VARARGS, ""},
	{"MidiFileNoteEndEvent_setNote", CMidiFileNoteEndEvent_setNote, METH_VARARGS, ""},
	{"MidiFileNoteEndEvent_getVelocity", CMidiFileNoteEndEvent_getVelocity, METH_VARARGS, ""},
	{"MidiFileNoteEndEvent_setVelocity", CMidiFileNoteEndEvent_setVelocity, METH_VARARGS, ""},
	{"MidiFileNoteEndEvent_getNoteStartEvent", CMidiFileNoteEndEvent_getNoteStartEvent, METH_VARARGS, ""},

	{"MidiFileTempoEvent_getTempo", CMidiFileTempoEvent_getTempo, METH_VARARGS, ""},
	{"MidiFileTempoEvent_setTempo", CMidiFileTempoEvent_setTempo, METH_VARARGS, ""},

	{"MidiFileTimeSignatureEvent_getNumerator", CMidiFileTimeSignatureEvent_getNumerator, METH_VARARGS, ""},
	{"MidiFileTimeSignatureEvent_getDenominator", CMidiFileTimeSignatureEvent_getDenominator, METH_VARARGS, ""},
	{"MidiFileTimeSignatureEvent_setTimeSignature", CMidiFileTimeSignatureEvent_setTimeSignature, METH_VARARGS, ""},

	{"MidiFileLyricEvent_getText", CMidiFileLyricEvent_getText, METH_VARARGS, ""},
	{"MidiFileLyricEvent_setText", CMidiFileLyricEvent_setText, METH_VARARGS, ""},

	{"MidiFileMarkerEvent_getText", CMidiFileMarkerEvent_getText, METH_VARARGS, ""},
	{"MidiFileMarkerEvent_setText", CMidiFileMarkerEvent_setText, METH_VARARGS, ""},

	{"MidiFileVoiceEvent_getData", CMidiFileVoiceEvent_getData, METH_VARARGS, ""},
	{"MidiFileVoiceEvent_setData", CMidiFileVoiceEvent_setData, METH_VARARGS, ""},

	{"MidiFileMeasureBeat_new", CMidiFileMeasureBeat_new, METH_VARARGS, ""},
	{"MidiFileMeasureBeat_free", CMidiFileMeasureBeat_free, METH_VARARGS, ""},
	{"MidiFileMeasureBeat_getMeasure", CMidiFileMeasureBeat_getMeasure, METH_VARARGS, ""},
	{"MidiFileMeasureBeat_setMeasure", CMidiFileMeasureBeat_setMeasure, METH_VARARGS, ""},
	{"MidiFileMeasureBeat_getBeat", CMidiFileMeasureBeat_getBeat, METH_VARARGS, ""},
	{"MidiFileMeasureBeat_setBeat", CMidiFileMeasureBeat_setBeat, METH_VARARGS, ""},
	{"MidiFileMeasureBeat_toString", CMidiFileMeasureBeat_toString, METH_VARARGS, ""},
	{"MidiFileMeasureBeat_parse", CMidiFileMeasureBeat_parse, METH_VARARGS, ""},

	{"MidiFileMeasureBeatTick_new", CMidiFileMeasureBeatTick_new, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_free", CMidiFileMeasureBeatTick_free, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_getMeasure", CMidiFileMeasureBeatTick_getMeasure, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_setMeasure", CMidiFileMeasureBeatTick_setMeasure, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_getBeat", CMidiFileMeasureBeatTick_getBeat, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_setBeat", CMidiFileMeasureBeatTick_setBeat, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_getTick", CMidiFileMeasureBeatTick_getTick, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_setTick", CMidiFileMeasureBeatTick_setTick, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_toString", CMidiFileMeasureBeatTick_toString, METH_VARARGS, ""},
	{"MidiFileMeasureBeatTick_parse", CMidiFileMeasureBeatTick_parse, METH_VARARGS, ""},

	{"MidiFileHourMinuteSecond_new", CMidiFileHourMinuteSecond_new, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_free", CMidiFileHourMinuteSecond_free, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_getHour", CMidiFileHourMinuteSecond_getHour, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_setHour", CMidiFileHourMinuteSecond_setHour, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_getMinute", CMidiFileHourMinuteSecond_getMinute, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_setMinute", CMidiFileHourMinuteSecond_setMinute, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_getSecond", CMidiFileHourMinuteSecond_getSecond, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_setSecond", CMidiFileHourMinuteSecond_setSecond, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_toString", CMidiFileHourMinuteSecond_toString, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecond_parse", CMidiFileHourMinuteSecond_parse, METH_VARARGS, ""},

	{"MidiFileHourMinuteSecondFrame_new", CMidiFileHourMinuteSecondFrame_new, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_free", CMidiFileHourMinuteSecondFrame_free, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_getHour", CMidiFileHourMinuteSecondFrame_getHour, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_setHour", CMidiFileHourMinuteSecondFrame_setHour, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_getMinute", CMidiFileHourMinuteSecondFrame_getMinute, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_setMinute", CMidiFileHourMinuteSecondFrame_setMinute, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_getSecond", CMidiFileHourMinuteSecondFrame_getSecond, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_setSecond", CMidiFileHourMinuteSecondFrame_setSecond, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_getFrame", CMidiFileHourMinuteSecondFrame_getFrame, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_setFrame", CMidiFileHourMinuteSecondFrame_setFrame, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_toString", CMidiFileHourMinuteSecondFrame_toString, METH_VARARGS, ""},
	{"MidiFileHourMinuteSecondFrame_parse", CMidiFileHourMinuteSecondFrame_parse, METH_VARARGS, ""},

	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initCMidiFile(void)
{
	PyObject *module;

	module = Py_InitModule("CMidiFile", CMidiFileMethods);
	if (module == NULL) return;

	PyModule_AddIntConstant(module, "DIVISION_TYPE_INVALID", MIDI_FILE_DIVISION_TYPE_INVALID);
	PyModule_AddIntConstant(module, "DIVISION_TYPE_PPQ", MIDI_FILE_DIVISION_TYPE_PPQ);
	PyModule_AddIntConstant(module, "DIVISION_TYPE_SMPTE24", MIDI_FILE_DIVISION_TYPE_SMPTE24);
	PyModule_AddIntConstant(module, "DIVISION_TYPE_SMPTE25", MIDI_FILE_DIVISION_TYPE_SMPTE25);
	PyModule_AddIntConstant(module, "DIVISION_TYPE_SMPTE30DROP", MIDI_FILE_DIVISION_TYPE_SMPTE30DROP);
	PyModule_AddIntConstant(module, "DIVISION_TYPE_SMPTE30", MIDI_FILE_DIVISION_TYPE_SMPTE30);

	PyModule_AddIntConstant(module, "EVENT_TYPE_INVALID", MIDI_FILE_EVENT_TYPE_INVALID);
	PyModule_AddIntConstant(module, "EVENT_TYPE_NOTE_OFF", MIDI_FILE_EVENT_TYPE_NOTE_OFF);
	PyModule_AddIntConstant(module, "EVENT_TYPE_NOTE_ON", MIDI_FILE_EVENT_TYPE_NOTE_ON);
	PyModule_AddIntConstant(module, "EVENT_TYPE_KEY_PRESSURE", MIDI_FILE_EVENT_TYPE_KEY_PRESSURE);
	PyModule_AddIntConstant(module, "EVENT_TYPE_CONTROL_CHANGE", MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE);
	PyModule_AddIntConstant(module, "EVENT_TYPE_PROGRAM_CHANGE", MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE);
	PyModule_AddIntConstant(module, "EVENT_TYPE_CHANNEL_PRESSURE", MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE);
	PyModule_AddIntConstant(module, "EVENT_TYPE_PITCH_WHEEL", MIDI_FILE_EVENT_TYPE_PITCH_WHEEL);
	PyModule_AddIntConstant(module, "EVENT_TYPE_SYSEX", MIDI_FILE_EVENT_TYPE_SYSEX);
	PyModule_AddIntConstant(module, "EVENT_TYPE_META", MIDI_FILE_EVENT_TYPE_META);
}

