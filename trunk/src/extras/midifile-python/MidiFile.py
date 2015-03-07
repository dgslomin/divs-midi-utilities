
import weakref
import CMidiFile

DIVISION_TYPE_PPQ = CMidiFile.DIVISION_TYPE_PPQ
DIVISION_TYPE_SMPTE24 = CMidiFile.DIVISION_TYPE_SMPTE24
DIVISION_TYPE_SMPTE25 = CMidiFile.DIVISION_TYPE_SMPTE25
DIVISION_TYPE_SMPTE30DROP = CMidiFile.DIVISION_TYPE_SMPTE30DROP
DIVISION_TYPE_SMPTE30 = CMidiFile.DIVISION_TYPE_SMPTE30

EVENT_TYPE_NOTE_OFF = CMidiFile.EVENT_TYPE_NOTE_OFF
EVENT_TYPE_NOTE_ON = CMidiFile.EVENT_TYPE_NOTE_ON
EVENT_TYPE_KEY_PRESSURE = CMidiFile.EVENT_TYPE_KEY_PRESSURE
EVENT_TYPE_CONTROL_CHANGE = CMidiFile.EVENT_TYPE_CONTROL_CHANGE
EVENT_TYPE_PROGRAM_CHANGE = CMidiFile.EVENT_TYPE_PROGRAM_CHANGE
EVENT_TYPE_CHANNEL_PRESSURE = CMidiFile.EVENT_TYPE_CHANNEL_PRESSURE
EVENT_TYPE_PITCH_WHEEL = CMidiFile.EVENT_TYPE_PITCH_WHEEL
EVENT_TYPE_SYSEX = CMidiFile.EVENT_TYPE_SYSEX
EVENT_TYPE_META = CMidiFile.EVENT_TYPE_META

_midi_file_wrappers = weakref.WeakValueDictionary()
_track_wrappers = weakref.WeakValueDictionary()
_event_wrappers = weakref.WeakValueDictionary()

class MidiFile:
	def __init__(self, filename = None, file_format = 1, division_type = CMidiFile.DIVISION_TYPE_PPQ, resolution = 960):
		if filename == None:
			self._midi_file = _raise_error_if_equal(CMidiFile.MidiFile_new(file_format, division_type, resolution), 0)
		else:
			self._midi_file = _raise_error_if_equal(CMidiFile.MidiFile_load(filename), 0)

	def __del__(self):
		CMidiFile.MidiFile_free(self._midi_file)

	def Save(self, filename):
		_raise_error_if_negative(CMidiFile.MidiFile_save(filename))

	def GetFileFormat(self):
		return _raise_error_if_negative(CMidiFile.MidiFile_getFileFormat(self._midi_file))

	def SetFileFormat(self, file_format):
		_raise_error_if_negative(CMidiFile.MidiFile_setFileFormat(self._midi_file, value))

	def GetDivisionType(self):
		return _raise_error_if_equal(CMidiFile.MidiFile_getDivisionType(self._midi_file), CMidiFile.DIVISION_TYPE_INVALID)

	def SetDivisionType(self, division_type):
		_raise_error_if_negative(CMidiFile.MidiFile_setDivisionType(self._midi_file, value))

	def GetResolution(self):
		return _raise_error_if_negative(CMidiFile.MidiFile_getResolution(self._midi_file))

	def SetResolution(self, resolution):
		_raise_error_if_negative(CMidiFile.MidiFile_setResolution(self._midi_file, value))

	def GetNumberOfTicksPerBeat(self):
		return _raise_error_if_negative(CMidiFile.MidiFile_getNumberOfTicksPerBeat(self._midi_file))

	def SetNumberOfTicksPerBeat(self, number_of_ticks_per_beat):
		_raise_error_if_negative(CMidiFile.MidiFile_setNumberOfTicksPerBeat(self._midi_file, value))

	def GetNumberOfFramesPerSecond(self):
		return _raise_error_if_negative(CMidiFile.MidiFile_getNumberOfFramesPerSecond(self._midi_file))

	def SetNumberOfFramesPerSecond(self, number_of_frames_per_second):
		_raise_error_if_negative(CMidiFile.MidiFile_setNumberOfFramesPerSecond(self._midi_file, value))

	def CreateTrack(self):
		return _wrap_track(_raise_error_if_equal(CMidiFile.MidiFile_createTrack(self._midi_file), 0))

	def GetNumberOfTracks(self):
		return _raise_error_if_negative(CMidiFile.MidiFile_getNumberOfTracks(self._midi_file))

	def GetTrackByNumber(self, number, create):
		return _wrap_track(_raise_error_if_equal(CMidiFile.MidiFile_getTrackByNumber(self._midi_file, number, create), 0))

	def GetFirstTrack(self):
		return _wrap_track(CMidiFile.MidiFile_getFirstTrack(self._midi_file))

	def GetLastTrack(self):
		return _wrap_track(CMidiFile.MidiFile_getLastTrack(self._midi_file))

	def GetFirstEvent(self):
		return _wrap_event(CMidiFile.MidiFile_getFirstEvent(self._midi_file))

	def GetLastEvent(self):
		return _wrap_event(CMidiFile.MidiFile_getLastEvent(self._midi_file))

	def VisitEvents(self, callback):
		_raise_error_if_negative(CMidiFile.MidiFile_visitEvents(self._midi_file, lambda _event: callback(_wrap_event(_event))))

	def GetBeatFromTick(self, tick):
		return _raise_error_if_negative(CMidiFile.MidiFile_getBeatFromTick(self._midi_file, tick))

	def GetTickFromBeat(self, beat):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromBeat(self._midi_file, beat))

	def GetTimeFromTick(self, tick):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTimeFromTick(self._midi_file, tick))

	def GetTickFromTime(self, time):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromTime(self._midi_file, time))

	def GetMeasureBeatFromTick(self, tick):
		measure_beat = MeasureBeat()
		_raise_error_if_negative(CMidiFile.MidiFile_setMeasureBeatFromTick(self._midi_file, tick, measure_beat._measure_beat))
		return measure_beat

	def GetTickFromMeasureBeat(self, measure_beat):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromMeasureBeat(self._midi_file, measure_beat._measure_beat))

	def GetMeasureBeatTickFromTick(self, tick):
		measure_beat_tick = MeasureBeatTick()
		_raise_error_if_negative(CMidiFile.MidiFile_setMeasureBeatTickFromTick(self._midi_file, tick, measure_beat_tick._measure_beat_tick))
		return measure_beat_tick

	def GetTickFromMeasureBeatTick(self, measure_beat_tick):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromMeasureBeatTick(self._midi_file, measure_beat_tick._measure_beat_tick))

	def GetHourMinuteSecondFromTick(self, tick):
		hour_minute_second = HourMinuteSecond()
		_raise_error_if_negative(CMidiFile.MidiFile_setHourMinuteSecondFromTick(self._midi_file, tick, hour_minute_second._hour_minute_second))
		return hour_minute_second

	def GetTickFromHourMinuteSecond(self, hour_minute_second):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromHourMinuteSecond(self._midi_file, hour_minute_second._hour_minute_second))

	def GetHourMinuteSecondFrameFromTick(self, tick):
		hour_minute_second_frame = HourMinuteSecondFrame()
		_raise_error_if_negative(CMidiFile.MidiFile_setHourMinuteSecondFrameFromTick(self._midi_file, tick, hour_minute_second_frame._hour_minute_second_frame))
		return hour_minute_second_frame

	def GetTickFromHourMinuteSecondFrame(self, hour_minute_second):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromHourMinuteSecondFrame(self._midi_file, hour_minute_second_frame._hour_minute_second_frame))

	def GetMeasureBeatStringFromTick(self, tick):
		return _raise_error_if_equal(CMidiFile.MidiFile_getMeasureBeatStringFromTick(self._midi_file, tick), None)

	def GetTickFromMeasureBeatString(self, measure_beat_string):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromMeasureBeatString(self._midi_file, measure_beat_string))

	def GetMeasureBeatTickStringFromTick(self, tick):
		return _raise_error_if_equal(CMidiFile.MidiFile_getMeasureBeatTickStringFromTick(self._midi_file, tick), None)

	def GetTickFromMeasureBeatTickString(self, measure_beat_tick_string):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromMeasureBeatTickString(self._midi_file, measure_beat_tick_string))

	def GetHourMinuteSecondStringFromTick(self, tick):
		return _raise_error_if_equal(CMidiFile.MidiFile_getHourMinuteSecondStringFromTick(self._midi_file, tick), None)

	def GetTickFromHourMinuteSecondString(self, hour_minute_second_string):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromHourMinuteSecondString(self._midi_file, hour_minute_second_string))

	def GetHourMinuteSecondFrameStringFromTick(self, tick):
		return _raise_error_if_equal(CMidiFile.MidiFile_getHourMinuteSecondFrameStringFromTick(self._midi_file, tick), None)

	def GetTickFromHourMinuteSecondFrameString(self, hour_minute_second_frame_string):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromHourMinuteSecondFrameString(self._midi_file, hour_minute_second_frame_string))

	def GetTickFromMarker(self, marker):
		return _raise_error_if_negative(CMidiFile.MidiFile_getTickFromMarker(self._midi_file, marker))

class Track:
	def __init__(self, _track = None):
		if _track == None: raise Error()
		self._track = _track

	def Delete(self):
		_raise_error_if_negative(CMidiFile.MidiFileTrack_delete(self._track))

	def GetMidiFile(self):
		return _wrap_midi_file(_raise_error_if_equal(CMidiFile.MidiFileTrack_getMidiFile(self._track), 0))

	def GetNumber(self):
		return _raise_error_if_negative(CMidiFile.MidiFileTrack_getNumber(self._track))

	def GetEndTick(self):
		return _raise_error_if_negative(CMidiFile.MidiFileTrack_getEndTick(self._track))

	def SetEndTick(self, end_tick):
		_raise_error_if_negative(CMidiFile.MidiFileTrack_setEndTick(self._track, end_tick))

	def CreateTrackBefore(self):
		return _wrap_track(_raise_error_if_equal(CMidiFile.MidiFileTrack_createTrackBefore(self._track), 0))

	def GetPreviousTrack(self):
		return _wrap_track(CMidiFile.MidiFileTrack_getPreviousTrack(self._track))

	def GetNextTrack(self):
		return _wrap_track(CMidiFile.MidiFileTrack_getNextTrack(self._track))

	def CreateNoteOffEvent(self, tick, channel, note, velocity):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createNoteOffEvent(self._track, tick, channel, note, velocity), 0))

	def CreateNoteOnEvent(self, tick, channel, note, velocity):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createNoteOnEvent(self._track, tick, channel, note, velocity), 0))

	def CreateKeyPressureEvent(self, tick, channel, note, amount):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createKeyPressureEvent(self._track, tick, channel, note, amount), 0))

	def CreateControlChangeEvent(self, tick, channel, number, value):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createControlChangeEvent(self._track, tick, channel, number, value), 0))

	def CreateProgramChangeEvent(self, tick, channel, number):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createProgramChangeEvent(self._track, tick, channel, number), 0))

	def CreateChannelPressureEvent(self, tick, channel, amount):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createChannelPressureEvent(self._track, tick, channel, amount), 0))

	def CreatePitchWheelEvent(self, tick, channel, value):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createPitchWheelEvent(self._track, tick, channel, value), 0))

	def CreateSysexEvent(self, tick, data):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createSysexEvent(self._track, tick, data), 0))

	def CreateMetaEvent(self, tick, number, data):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createMetaEvent(self._track, tick, number, data), 0))

	def CreateNoteStartAndEndEvents(self, start_tick, end_tick, channel, note, start_velocity, end_velocity):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createNoteStartAndEndEvents(self._track, start_tick, end_tick, channel, note, start_velocity, end_velocity), 0))

	def CreateTempoEvent(self, tick, tempo):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createTempoEvent(self._track, tick, tempo), 0))

	def CreateTimeSignatureEvent(self, tick, numerator, denominator):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createTimeSignatureEvent(self._track, tick, numerator, denominator), 0))

	def CreateLyricEvent(self, tick, text):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createLyricEvent(self._track, tick, text), 0))

	def CreateMarkerEvent(self, tick, text):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createMarkerEvent(self._track, tick, text), 0))

	def CreateVoiceEvent(self, tick, data):
		return _wrap_event(_raise_error_if_equal(CMidiFile.MidiFileTrack_createVoiceEvent(self._track, tick, data), 0))

	def GetFirstEvent(self):
		return _wrap_event(CMidiFile.MidiFileTrack_getFirstEvent(self._track))

	def GetLastEvent(self):
		return _wrap_event(CMidiFile.MidiFileTrack_getFirstEvent(self._track))

	def VisitEvents(self, callback):
		_raise_error_if_negative(CMidiFile.MidiFileTrack_visitEvents(self._track, lambda _event: callback(_wrap_event(_event))))

class Event:
	def __init__(self, _event = None):
		if _event == None: raise Error()
		self._event = _event

	def Delete(self):
		_raise_error_if_negative(CMidiFile.MidiFileEvent_delete(self._event))

	def GetTrack(self):
		return _wrap_track(_raise_error_if_equal(CMidiFile.MidiFileEvent_getTrack(self._event), 0))

	def GetPreviousEventInTrack(self):
		return _wrap_event(CMidiFile.MidiFileEvent_getPreviousEventInTrack(self._event))

	def GetNextEventInTrack(self):
		return _wrap_event(CMidiFile.MidiFileEvent_getNextEventInTrack(self._event))

	def GetPreviousEventInFile(self):
		return _wrap_event(CMidiFile.MidiFileEvent_getPreviousEventInFile(self._event))

	def GetNextEventInFile(self):
		return _wrap_event(CMidiFile.MidiFileEvent_getNextEventInFile(self._event))

	def GetTick(self):
		return _raise_error_if_negative(CMidiFile.MidiFileEvent_getTick(self._event))

	def SetTick(self, tick):
		_raise_error_if_negative(CMidiFile.MidiFileEvent_setTick(self._event, tick))

	def GetType(self):
		return _raise_error_if_equal(CMidiFile.MidiFileEvent_getType(self._event), CMidiFile.EVENT_TYPE_INVALID)

	def IsNoteEvent(self):
		return CMidiFile.MidiFileEvent_isNoteEvent(self._event)

	def IsNoteStartEvent(self):
		return CMidiFile.MidiFileEvent_isNoteStartEvent(self._event)

	def IsNoteEndEvent(self):
		return CMidiFile.MidiFileEvent_isNoteEndEvent(self._event)

	def IsTempoEvent(self):
		return CMidiFile.MidiFileEvent_isTempoEvent(self._event)

	def IsTimeSignatureEvent(self):
		return CMidiFile.MidiFileEvent_isTimeSignatureEvent(self._event)

	def IsLyricEvent(self):
		return CMidiFile.MidiFileEvent_isLyricEvent(self._event)

	def IsMarkerEvent(self):
		return CMidiFile.MidiFileEvent_isMarkerEvent(self._event)

	def IsVoiceEvent(self):
		return CMidiFile.MidiFileEvent_isVoiceEvent(self._event)

	def NoteOffEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteOffEvent_getChannel(self._event))

	def NoteOffEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFileNoteOffEvent_setChannel(self._event, channel))

	def NoteOffEventGetNote(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteOffEvent_getNote(self._event))

	def NoteOffEventSetNote(self, note):
		_raise_error_if_negative(CMidiFile.MidiFileNoteOffEvent_setNote(self._event, note))

	def NoteOffEventGetVelocity(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteOffEvent_getVelocity(self._event))

	def NoteOffEventSetVelocity(self, velocity):
		_raise_error_if_negative(CMidiFile.MidiFileNoteOffEvent_setVelocity(self._event, velocity))

	def NoteOnEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteOnEvent_getChannel(self._event))

	def NoteOnEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFileNoteOnEvent_setChannel(self._event, channel))

	def NoteOnEventGetNote(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteOnEvent_getNote(self._event))

	def NoteOnEventSetNote(self, note):
		_raise_error_if_negative(CMidiFile.MidiFileNoteOnEvent_setNote(self._event, note))

	def NoteOnEventGetVelocity(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteOnEvent_getVelocity(self._event))

	def NoteOnEventSetVelocity(self, velocity):
		_raise_error_if_negative(CMidiFile.MidiFileNoteOnEvent_setVelocity(self._event, velocity))

	def KeyPressureEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFileKeyPressureEvent_getChannel(self._event))

	def KeyPressureEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFileKeyPressureEvent_setChannel(self._event, channel))

	def KeyPressureEventGetNote(self):
		return _raise_error_if_negative(CMidiFile.MidiFileKeyPressureEvent_getNote(self._event))

	def KeyPressureEventSetNote(self, note):
		_raise_error_if_negative(CMidiFile.MidiFileKeyPressureEvent_setNote(self._event, note))

	def KeyPressureEventGetAmount(self):
		return _raise_error_if_negative(CMidiFile.MidiFileKeyPressureEvent_getAmount(self._event))

	def KeyPressureEventSetAmount(self, amount):
		_raise_error_if_negative(CMidiFile.MidiFileKeyPressureEvent_setAmount(self._event, amount))

	def ControlChangeEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFileControlChangeEvent_getChannel(self._event))

	def ControlChangeEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFileControlChangeEvent_setChannel(self._event, channel))

	def ControlChangeEventGetNumber(self):
		return _raise_error_if_negative(CMidiFile.MidiFileControlChangeEvent_getNumber(self._event))

	def ControlChangeEventSetNumber(self, number):
		_raise_error_if_negative(CMidiFile.MidiFileControlChangeEvent_setNumber(self._event, number))

	def ControlChangeEventGetValue(self):
		return _raise_error_if_negative(CMidiFile.MidiFileControlChangeEvent_getValue(self._event))

	def ControlChangeEventSetValue(self, value):
		_raise_error_if_negative(CMidiFile.MidiFileControlChangeEvent_setValue(self._event, value))

	def ProgramChangeEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFileProgramChangeEvent_getChannel(self._event))

	def ProgramChangeEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFileProgramChangeEvent_setChannel(self._event, channel))

	def ProgramChangeEventGetNumber(self):
		return _raise_error_if_negative(CMidiFile.MidiFileProgramChangeEvent_getNumber(self._event))

	def ProgramChangeEventSetNumber(self, number):
		_raise_error_if_negative(CMidiFile.MidiFileProgramChangeEvent_setNumber(self._event, number))

	def ChannelPressureEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFileChannelPressureEvent_getChannel(self._event))

	def ChannelPressureEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFileChannelPressureEvent_setChannel(self._event, channel))

	def ChannelPressureEventGetAmount(self):
		return _raise_error_if_negative(CMidiFile.MidiFileChannelPressureEvent_getAmount(self._event))

	def ChannelPressureEventSetAmount(self, amount):
		_raise_error_if_negative(CMidiFile.MidiFileChannelPressureEvent_setAmount(self._event, amount))

	def PitchWheelEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFilePitchWheelEvent_getChannel(self._event))

	def PitchWheelEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFilePitchWheelEvent_setChannel(self._event, channel))

	def PitchWheelEventGetValue(self):
		return _raise_error_if_negative(CMidiFile.MidiFilePitchWheelEvent_getValue(self._event))

	def PitchWheelEventSetValue(self, value):
		_raise_error_if_negative(CMidiFile.MidiFilePitchWheelEvent_setValue(self._event, value))

	def SysexEventGetData(self):
		return _raise_error_if_equal(CMidiFile.MidiFileSysexEvent_getData(self._event), None)

	def SysexEventSetData(self, data):
		_raise_error_if_negative(CMidiFile.MidiFileSysexEvent_setData(self._event, data))

	def MetaEventGetNumber(self):
		return _raise_error_if_equal(CMidiFile.MidiFileMetaEvent_getNumber(self._event), None)

	def MetaEventSetNumber(self, number):
		_raise_error_if_negative(CMidiFile.MidiFileMetaEvent_setNumber(self._event, number))

	def MetaEventGetData(self):
		return _raise_error_if_equal(CMidiFile.MidiFileMetaEvent_getData(self._event), None)

	def MetaEventSetData(self, data):
		_raise_error_if_negative(CMidiFile.MidiFileMetaEvent_setData(self._event, data))

	def NoteStartEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteStartEvent_getChannel(self._event))

	def NoteStartEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFileNoteStartEvent_setChannel(self._event, channel))

	def NoteStartEventGetNote(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteStartEvent_getNote(self._event))

	def NoteStartEventSetNote(self, note):
		_raise_error_if_negative(CMidiFile.MidiFileNoteStartEvent_setNote(self._event, note))

	def NoteStartEventGetVelocity(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteStartEvent_getVelocity(self._event))

	def NoteStartEventSetVelocity(self, velocity):
		_raise_error_if_negative(CMidiFile.MidiFileNoteStartEvent_setVelocity(self._event, velocity))

	def NoteStartEventGetNoteEndEvent(self):
		return _wrap_event(CMidiFile.MidiFileNoteStartEvent_getNoteEndEvent(self._event))

	def NoteEndEventGetChannel(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteEndEvent_getChannel(self._event))

	def NoteEndEventSetChannel(self, channel):
		_raise_error_if_negative(CMidiFile.MidiFileNoteEndEvent_setChannel(self._event, channel))

	def NoteEndEventGetNote(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteEndEvent_getNote(self._event))

	def NoteEndEventSetNote(self, note):
		_raise_error_if_negative(CMidiFile.MidiFileNoteEndEvent_setNote(self._event, note))

	def NoteEndEventGetVelocity(self):
		return _raise_error_if_negative(CMidiFile.MidiFileNoteEndEvent_getVelocity(self._event))

	def NoteEndEventSetVelocity(self, velocity):
		_raise_error_if_negative(CMidiFile.MidiFileNoteEndEvent_setVelocity(self._event, velocity))

	def NoteEndEventGetNoteStartEvent(self):
		return _wrap_event(CMidiFile.MidiFileNoteEndEvent_getNoteStartEvent(self._event))

	def TempoEventGetTempo(self):
		return _raise_error_if_negative(CMidiFile.MidiFileTempoEvent_getTempo(self._event))

	def TempoEventSetTempo(self, tempo):
		_raise_error_if_negative(CMidiFile.MidiFileTempoEvent_setTempo(self._event, tempo))

	def TimeSignatureEventGetNumerator(self):
		return _raise_error_if_negative(CMidiFile.MidiFileTimeSignatureEvent_getNumerator(self._event))

	def TimeSignatureEventGetDenominator(self):
		return _raise_error_if_negative(CMidiFile.MidiFileTimeSignatureEvent_getDenominator(self._event))

	def TimeSignatureEventSetTimeSignature(self, numerator, denominator):
		_raise_error_if_negative(CMidiFile.MidiFileTimeSignatureEvent_setTimeSignature(self._event, numerator, denominator))

	def LyricEventGetText(self):
		return _raise_error_if_equal(CMidiFile.MidiFileLyricEvent_getText(self._event), None)

	def LyricEventSetText(self, text):
		_raise_error_if_negative(CMidiFile.MidiFileLyricEvent_setText(self._event, text))

	def MarkerEventGetText(self):
		return _raise_error_if_equal(CMidiFile.MidiFileMarkerEvent_getText(self._event), None)

	def MarkerEventSetText(self, text):
		_raise_error_if_negative(CMidiFile.MidiFileMarkerEvent_setText(self._event, text))

	def VoiceEventGetData(self):
		return _raise_error_if_equal(CMidiFile.MidiFileVoiceEvent_getData(self._event), 0)

	def VoiceEventSetData(self, data):
		_raise_error_if_negative(CMidiFile.MidiFileVoiceEvent_setData(self._event, data))

class MeasureBeat:
	def __init__(self, string = None, measure = None, beat = None, _measure_beat = None):
		if _measure_beat == None:
			self._measure_beat = CMidiFile.MidiFileMeasureBeat_new()

			if string == None:
				if measure != None:
					_raise_error_if_negative(CMidiFile.MidiFileMeasureBeat_setMeasure(self._measure_beat, measure))

				if beat != None:
					_raise_error_if_negative(CMidiFile.MidiFileMeasureBeat_setBeat(self._measure_beat, beat))

			else:
				_raise_error_if_negative(CMidiFile.MidiFileMeasureBeat_parse(self._measure_beat, string))

		else:
			self._measure_beat = _measure_beat

	def __del__(self):
		CMidiFile.MidiFileMeasureBeat_free(self._measure_beat)

	def __str__(self):
		return _raise_error_if_equal(CMidiFile.MidiFileMeasureBeat_toString(self._measure_beat), None)

	def GetMeasure(self):
		return _raise_error_if_negative(CMidiFile.MidiFileMeasureBeat_getMeasure(self._measure_beat))

	def SetMeasure(self, measure):
		_raise_error_if_negative(CMidiFile.MidiFileMeasureBeat_setMeasure(self._measure_beat, measure))

	def GetBeat(self):
		return _raise_error_if_negative(CMidiFile.MidiFileMeasureBeat_getBeat(self._measure_beat))

	def SetBeat(self, beat):
		_raise_error_if_negative(CMidiFile.MidiFileMeasureBeat_setBeat(self._measure_beat, beat))

class MeasureBeatTick:
	def __init__(self, string = None, measure = None, beat = None, tick = None, _measure_beat_tick = None):
		if _measure_beat_tick == None:
			self._measure_beat_tick = CMidiFile.MidiFileMeasureBeatTick_new()

			if string == None:
				if measure != None:
					_raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_setMeasure(self._measure_beat_tick, measure))

				if beat != None:
					_raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_setBeat(self._measure_beat_tick, beat))

				if tick != None:
					_raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_setTick(self._measure_beat_tick, tick))

			else:
				_raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_parse(self._measure_beat_tick, string))

		else:
			self._measure_beat_tick = _measure_beat_tick

	def __del__(self):
		CMidiFile.MidiFileMeasureBeatTick_free(self._measure_beat_tick)

	def __str__(self):
		return _raise_error_if_equal(CMidiFile.MidiFileMeasureBeatTick_toString(self._measure_beat_tick), None)

	def GetMeasure(self):
		return _raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_getMeasure(self._measure_beat_tick))

	def SetMeasure(self, measure):
		_raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_setMeasure(self._measure_beat_tick, measure))

	def GetBeat(self):
		return _raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_getBeat(self._measure_beat_tick))

	def SetBeat(self, beat):
		_raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_setBeat(self._measure_beat_tick, beat))

	def GetTick(self):
		return _raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_getTick(self._measure_beat_tick))

	def SetTick(self, tick):
		_raise_error_if_negative(CMidiFile.MidiFileMeasureBeatTick_setTick(self._measure_beat_tick, tick))

class HourMinuteSecond:
	def __init__(self, string = None, hour = None, minute = None, second = None, _hour_minute_second = None):
		if _hour_minute_second == None:
			self._hour_minute_second = CMidiFile.MidiFileHourMinuteSecond_new()

			if string == None:
				if hour != None:
					_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_setHour(self._hour_minute_second, hour))

				if minute != None:
					_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_setMinute(self._hour_minute_second, minute))

				if second != None:
					_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_setSecond(self._hour_minute_second, second))

			else:
				_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_parse(self._hour_minute_second, string))

		else:
			self._hour_minute_second = _hour_minute_second

	def __del__(self):
		CMidiFile.MidiFileHourMinuteSecond_free(self._hour_minute_second)

	def __str__(self):
		return _raise_error_if_equal(CMidiFile.MidiFileHourMinuteSecond_toString(self._hour_minute_second), None)

	def GetHour(self):
		return _raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_getHour(self._hour_minute_second))

	def SetHour(self, hour):
		_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_setHour(self._hour_minute_second, hour))

	def GetMinute(self):
		return _raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_getMinute(self._hour_minute_second))

	def SetMinute(self, minute):
		_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_setMinute(self._hour_minute_second, minute))

	def GetSecond(self):
		return _raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_getSecond(self._hour_minute_second))

	def SetSecond(self, second):
		_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecond_setSecond(self._hour_minute_second, second))

class HourMinuteSecondFrame:
	def __init__(self, string = None, hour = None, minute = None, second = None, frame = None, _hour_minute_second_frame = None):
		if _hour_minute_second_frame == None:
			_hour_minute_second_frame = CMidiFile.MidiFileHourMinuteSecondFrame_new()

			if string == None:
				if hour != None:
					_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_setHour(self._hour_minute_second_frame, hour))

				if minute != None:
					_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_setMinute(self._hour_minute_second_frame, minute))

				if second != None:
					_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_setSecond(self._hour_minute_second_frame, second))

				if frame != None:
					_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_setFrame(self._hour_minute_second_frame, frame))

			else:
				_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_parse(self._hour_minute_second_frame, string))

		else:
			self._hour_minute_second_frame = _hour_minute_second_frame

	def __del__(self):
		CMidiFile.MidiFileHourMinuteSecondFrame_free(self._hour_minute_second_frame)

	def __str__(self):
		return _raise_error_if_equal(CMidiFile.MidiFileHourMinuteSecondFrame_toString(self._hour_minute_second_frame), None)

	def GetHour(self):
		return _raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_getHour(self._hour_minute_second_frame))

	def SetHour(self, hour):
		_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_setHour(self._hour_minute_second_frame, hour))

	def GetMinute(self):
		return _raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_getMinute(self._hour_minute_second_frame))

	def SetMinute(self, minute):
		_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_setMinute(self._hour_minute_second_frame, minute))

	def GetSecond(self):
		return _raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_getSecond(self._hour_minute_second_frame))

	def SetSecond(self, second):
		_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_setSecond(self._hour_minute_second_frame, second))

	def GetFrame(self):
		return _raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_getFrame(self._hour_minute_second_frame))

	def SetFrame(self, frame):
		_raise_error_if_negative(CMidiFile.MidiFileHourMinuteSecondFrame_setFrame(self._hour_minute_second_frame, frame))

class Error(Exception):
	def __init__(self, *args, **keywords): Error.__init__(*args, **keywords)

def _wrap_midi_file(_midi_file):
	if _midi_file == 0: return None
	midi_file = _midi_file_wrappers.get(_midi_file, None)

	if midi_file == None:
		midi_file = MidiFile(_midi_file = _midi_file)
		_midi_file_wrappers[_midi_file] = midi_file

	return midi_file

def _wrap_track(_track):
	if _track == 0: return None
	track = _track_wrappers.get(_track, None)

	if track == None:
		track = Track(_track = _track)
		_track_wrappers[_track] = track

	return track

def _wrap_event(_event):
	if _event == 0: return None
	event = _event_wrappers.get(_event, None)

	if event == None:
		event = Event(_event = _event)
		_event_wrappers[_event] = event

	return event

def _raise_error_if_equal(value, error_value):
	if value == error_value: raise Error()
	return value

def _raise_error_if_negative(value):
	if value < 0: raise Error()
	return value

