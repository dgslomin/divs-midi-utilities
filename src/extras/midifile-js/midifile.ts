
export class MidiFile
{
	static _numberOfFramesPerSecondForDivisionType = { "PPQ": 30.0, "SMPTE24": 24.0, "SMPTE25": 25.0, "SMPTE30DROP": 29.97, "SMPTE30": 30.0 };

	_fileFormat: number;
	_divisionType: DivisionType;
	_resolution: number;
	_numberOfFramesPerSecond: number;
	_numberOfTracks = 0;
	_firstTrack: Track | null = null;
	_lastTrack: Track | null = null;
	_firstEvent: Event | null = null;
	_lastEvent: Event | null = null;

	constructor(fileFormat: number = 1, divisionType: DivisionType = "PPQ", resolution: number = 960)
	{
		this._fileFormat = fileFormat;
		this._divisionType = divisionType;
		this._resolution = resolution;
		this._numberOfFramesPerSecond = MidiFile._numberOfFramesPerSecondForDivisionType[divisionType];
	}

	static loadFromBuffer(buffer: Uint8Array): MidiFile
	{
		let midiFile: MidiFile | null = null;
		let io = new IO(buffer);
		let numberOfTracksRead = 0;

		let chunkId = io.readUint32();
		let chunkSize = io.readUint32();
		let chunkStart = io.tell();

		// check for the RMID variation on SMF

		if (chunkId == 0x52494646) // "RIFF"
		{
			let typeId = io.readUint32();

			if (typeId != 0x524D4943) // "RMID"
			{
				throw new Error("invalid file");
			}

			chunkId = io.readUint32();
			chunkSize = io.readUint32();

			if (chunkId != 0x64617461) // "data"
			{
				throw new Error("invalid file");
			}

			chunkId = io.readUint32();
			chunkSize = io.readUint32();
			chunkStart = io.tell();
		}

		if (chunkId != 0x4D546864) // "MThd"
		{
			throw new Error("invalid file");
		}

		let fileFormat = io.readUint16();
		let numberOfTracks = io.readUint16();
		let divisionType = io.getc();

		switch (divisionType)
		{
			case 0xE8:
			{
				let resolution = io.getc();
				midiFile = new MidiFile(fileFormat, "SMPTE24", resolution);
				break;
			}
			case 0xE7:
			{
				let resolution = io.getc();
				midiFile = new MidiFile(fileFormat, "SMPTE25", resolution);
				break;
			}
			case 0xE3:
			{
				let resolution = io.getc();
				midiFile = new MidiFile(fileFormat, "SMPTE30DROP", resolution);
				break;
			}
			case 0xE2:
			{
				let resolution = io.getc();
				midiFile = new MidiFile(fileFormat, "SMPTE30", resolution);
				break;
			}
			default:
			{
				io.seek(-1, "CUR");
				let resolution = io.readUint16();
				midiFile = new MidiFile(fileFormat, "PPQ", resolution);
				break;
			}
		}

		// forwards compatibility:  skip over any extra header data
		io.seek(chunkStart + chunkSize, "SET");

		while (numberOfTracksRead < numberOfTracks)
		{
			let chunkId = io.readUint32();
			let chunkSize = io.readUint32();
			let chunkStart = io.tell();

			if (chunkId == 0x4D54726B) // "MTrk"
			{
				let track = midiFile.createTrack();
				let previousTick = 0;
				let runningStatus = 0;
				let atEndOfTrack = false;

				while ((io.tell() < chunkStart + chunkSize) && !atEndOfTrack)
				{
					let tick = io.readVariableLengthQuantity() + previousTick;
					previousTick = tick;

					let status = io.getc();

					if ((status & 0x80) == 0x00)
					{
						status = runningStatus;
						io.seek(-1, "CUR");
					}
					else
					{
						runningStatus = status;
					}

					switch (status & 0xF0)
					{
						case 0x80:
						{
							let channel = status & 0x0F;
							let note = io.getc();
							let velocity = io.getc();
							track.createNoteOffEvent(tick, channel, note, velocity);
							break;
						}
						case 0x90:
						{
							let channel = status & 0x0F;
							let note = io.getc();
							let velocity = io.getc();
							track.createNoteOnEvent(tick, channel, note, velocity);
							break;
						}
						case 0xA0:
						{
							let channel = status & 0x0F;
							let note = io.getc();
							let amount = io.getc();
							track.createKeyPressureEvent(tick, channel, note, amount);
							break;
						}
						case 0xB0:
						{
							let channel = status & 0x0F;
							let number = io.getc();
							let value = io.getc();
							track.createControlChangeEvent(tick, channel, number, value);
							break;
						}
						case 0xC0:
						{
							let channel = status & 0x0F;
							let number = io.getc();
							track.createProgramChangeEvent(tick, channel, number);
							break;
						}
						case 0xD0:
						{
							let channel = status & 0x0F;
							let amount = io.getc();
							track.createChannelPressureEvent(tick, channel, amount);
							break;
						}
						case 0xE0:
						{
							let channel = status & 0x0F;
							let value = io.getc();
							value |= (io.getc() << 7);
							track.createPitchWheelEvent(tick, channel, value);
							break;
						}
						case 0xF0:
						{
							switch (status)
							{
								case 0xF0:
								case 0xF7:
								{
									let dataLength = io.readVariableLengthQuantity();
									io.seek(-1, "CUR");
									let data = io.read(dataLength + 1);
									data[0] = status;
									track.createSysexEvent(tick, data);
									break;
								}
								case 0xFF:
								{
									let number = io.getc();
									let dataLength = io.readVariableLengthQuantity();
									let data = io.read(dataLength);

									if (number == 0x2F)
									{
										track.endTick = tick;
										atEndOfTrack = true;
									}
									else
									{
										track.createMetaEvent(tick, number, data);
									}

									break;
								}
							}

							break;
						}
					}
				}

				numberOfTracksRead++;
			}

			// forwards compatibility:  skip over any unrecognized chunks, or extra data at the end of tracks
			io.seek(chunkStart + chunkSize, "SET");
		}

		return midiFile;
	}

	saveToBuffer(): Uint8Array
	{
		function saveToIO(midiFile: MidiFile, io: IO)
		{
			io.writeUint32(0x4D546864); // "MThd"
			io.writeUint32(6);
			io.writeUint16(midiFile.fileFormat);
			io.writeUint16(midiFile.numberOfTracks);

			switch (midiFile.divisionType)
			{
				case "PPQ":
				{
					io.writeUint16(midiFile.resolution);
					break;
				}
				case "SMPTE24":
				{
					io.putc(0xE8);
					io.putc(midiFile.resolution);
					break;
				}
				case "SMPTE25":
				{
					io.putc(0xE7);
					io.putc(midiFile.resolution);
					break;
				}
				case "SMPTE30DROP":
				{
					io.putc(0xE3);
					io.putc(midiFile.resolution);
					break;
				}
				case "SMPTE30":
				{
					io.putc(0xE2);
					io.putc(midiFile.resolution);
					break;
				}
			}

			for (let track = midiFile.firstTrack; track; track = track.nextTrack)
			{
				io.writeUint32(0x4D54726B) // "MTrk"

				let trackSizeOffset = io.tell();
				io.writeUint32(0);
				let trackStartOffset = io.tell();

				let previousTick = 0;

				for (let event = track.firstEvent; event; event = event.nextEventInTrack)
				{
					let tick = event.tick;
					io.writeVariableLengthQuantity(tick - previousTick);

					if (event instanceof VoiceEvent)
					{
						io.write(event.messageData);
					}
					else if (event instanceof SysexEvent)
					{
						let data = event.data;
						io.putc(data[0]);
						io.writeVariableLengthQuantity(data.length - 1);
						io.write(data.subarray(1, data.length - 1));
					}
					else if (event instanceof MetaEvent)
					{
						let data = event.data;
						io.putc(0xFF);
						io.putc(event.number);
						io.writeVariableLengthQuantity(data.length);
						io.write(data);
					}
					else
					{
						// skip pseudo-events; they must be converted to standard ones first
					}

					previousTick = tick;
				}

				io.writeVariableLengthQuantity(track.endTick - previousTick);
				io.putc(0xFF);
				io.putc(0x2F);
				io.putc(0x00);

				let trackEndOffset = io.tell();
				io.seek(trackSizeOffset, "SET");
				io.writeUint32(trackEndOffset - trackStartOffset);
				io.seek(trackEndOffset, "SET");
			}
		}

		let io = new IO(null);
		saveToIO(this, io);
		let bufferSize = io.tell();

		let buffer = new Uint8Array(bufferSize);
		io = new IO(buffer);
		saveToIO(this, io);
		return buffer;
	}

	createMidiFileWithSameFormat(): MidiFile
	{
		return new MidiFile(this.fileFormat, this.divisionType, this.resolution);
	}

	get fileFormat(): number
	{
		return this._fileFormat;
	}

	set fileFormat(fileFormat: number)
	{
		this._fileFormat = fileFormat;
	}

	get divisionType(): DivisionType
	{
		return this._divisionType;
	}

	set divisionType(divisionType: DivisionType)
	{
		this._divisionType = divisionType;
		this._numberOfFramesPerSecond = MidiFile._numberOfFramesPerSecondForDivisionType[divisionType];
	}

	get resolution(): number
	{
		return this._resolution;
	}

	set resolution(resolution: number)
	{
		this._resolution = resolution;
	}

	get numberOfFramesPerSecond(): number
	{
		return this._numberOfFramesPerSecond;
	}

	set numberOfFramesPerSecond(numberOfFramesPerSecond: number)
	{
		this._numberOfFramesPerSecond = numberOfFramesPerSecond;
	}

	createTrack(): Track
	{
		let newTrack = Track._createTrack(this);
		newTrack._number = this._numberOfTracks;
		newTrack._previousTrack = this._lastTrack;
		this._lastTrack = newTrack;

		if (newTrack._previousTrack)
		{
			newTrack._previousTrack._nextTrack = newTrack;
		}
		else
		{
			this._firstTrack = newTrack;
		}

		this._numberOfTracks++;
		return newTrack;
	}

	get numberOfTracks(): number
	{
		return this._numberOfTracks;
	}

	getTrackByNumber(number: number, create: boolean): Track
	{
		let track: Track | null = null;

		for (let currentTrackNumber = 0; currentTrackNumber <= number; currentTrackNumber++)
		{
			if (track)
			{
				track = track.nextTrack;
			}
			else
			{
				track = this.firstTrack;
			}

			if (!track)
			{
				if (create)
				{
					track = this.createTrack();
				}
				else
				{
					throw new Error("no such track");
				}
			}
		}

		return track!;
	}

	get firstTrack(): Track | null
	{
		return this._firstTrack;
	}

	get lastTrack(): Track | null
	{
		return this._lastTrack;
	}

	get tracks(): Array<Track>
	{
		let tracks: Array<Track> = [];
		for (let track = this.firstTrack; track; track = track.nextTrack) tracks.push(track);
		return tracks;
	}

	get firstEvent(): Event | null
	{
		return this._firstEvent;
	}

	get lastEvent(): Event | null
	{
		return this._lastEvent;
	}

	get events(): Array<Event>
	{
		let events: Array<Event> = [];
		for (let event = this.firstEvent; event; event = event.nextEventInFile) events.push(event);
		return events;
	}

	getBeatFromTick(tick: number): number
	{
		if (this.divisionType == "PPQ")
		{
			return tick / this.resolution;
		}
		else
		{
			let conductorTrack = this.firstTrack;
			let tempoEventBeat = 0;
			let tempoEventTick = 0;
			let tempo = 120.0;

			for (let event = conductorTrack?.firstEvent; event && (event.tick < tick); event = event.nextEventInTrack)
			{
				if ((event instanceof MetaEvent) && event.isTempoEvent)
				{
					tempoEventBeat += ((event.tick - tempoEventTick) / this.resolution / this.numberOfFramesPerSecond / 60.0 * tempo);
					tempoEventTick = event.tick;
					tempo = event.tempoEventTempo;
				}
			}

			return tempoEventBeat + ((tick - tempoEventTick) / this.resolution / this.numberOfFramesPerSecond / 60.0 * tempo);
		}
	}

	getTickFromBeat(beat: number): number
	{
		if (this.divisionType == "PPQ")
		{
			return beat * this.resolution;
		}
		else
		{
			let conductorTrack = this.firstTrack;
			let tempoEventBeat = 0;
			let tempoEventTick = 0;
			let tempo = 120.0;

			for (let event = conductorTrack?.firstEvent; event; event = event.nextEventInTrack)
			{
				if ((event instanceof MetaEvent) && event.isTempoEvent)
				{
					let nextTempoEventBeat = tempoEventBeat + ((event.tick - tempoEventTick) / this.resolution / this.numberOfFramesPerSecond / 60.0 * tempo);
					if (nextTempoEventBeat >= beat) break;
					tempoEventBeat = nextTempoEventBeat;
					tempoEventTick = event.tick;
					tempo = event.tempoEventTempo;
				}
			}

			return tempoEventTick + ((beat - tempoEventBeat) / tempo * 60.0 * this.numberOfFramesPerSecond * this.resolution);
		}
	}

	getTimeFromTick(tick: number): number
	{
		if (this.divisionType == "PPQ")
		{
			let conductorTrack = this.firstTrack;
			let tempoEventTime = 0;
			let tempoEventTick = 0;
			let tempo = 120.0;

			for (let event = conductorTrack?.firstEvent; event && (event.tick < tick); event = event.nextEventInTrack)
			{
				if ((event instanceof MetaEvent) && event.isTempoEvent)
				{
					tempoEventTime += ((event.tick - tempoEventTick) / this.resolution / (tempo / 60));
					tempoEventTick = event.tick;
					tempo = event.tempoEventTempo;
				}
			}

			return tempoEventTime + ((tick - tempoEventTick) / this.resolution / (tempo / 60));
		}
		else
		{
			return tick / (this.resolution * this.numberOfFramesPerSecond);
		}
	}

	getTickFromTime(time: number): number
	{
		if (this.divisionType == "PPQ")
		{
			let conductorTrack = this.firstTrack;
			let tempoEventTime = 0;
			let tempoEventTick = 0;
			let tempo = 120.0;

			for (let event = conductorTrack?.firstEvent; event; event = event.nextEventInTrack)
			{
				if ((event instanceof MetaEvent) && event.isTempoEvent)
				{
					let nextTempoEventTime: number = tempoEventTime + ((event.tick - tempoEventTick) / this.resolution / (tempo / 60));
					if (nextTempoEventTime >= time) break;
					tempoEventTime = nextTempoEventTime;
					tempoEventTick = event.tick;
					tempo = event.tempoEventTempo;
				}
			}

			return tempoEventTick + ((time - tempoEventTime) * (tempo / 60) * this.resolution);
		}
		else
		{
			return time * this.resolution * this.numberOfFramesPerSecond;
		}
	}

	getMeasureBeatFromTick(tick: number): MeasureBeat
	{
		let conductorTrack = this.firstTrack;
		let timeSignatureEventBeat = 0;
		let measure = 0;
		let numerator = 4;
		let denominator = 4;

		for (let event = conductorTrack?.firstEvent; event && (event.tick < tick); event = event.nextEventInTrack)
		{
			if ((event instanceof MetaEvent) && event.isTimeSignatureEvent)
			{
				let nextTimeSignatureEventBeat = this.getBeatFromTick(event.tick);
				measure += ((nextTimeSignatureEventBeat - timeSignatureEventBeat) * (denominator / numerator / 4));
				timeSignatureEventBeat = nextTimeSignatureEventBeat;
				numerator = event.timeSignatureEventNumerator;
				denominator = event.timeSignatureEventDenominator;
			}
		}

		measure += ((this.getBeatFromTick(tick) - timeSignatureEventBeat) * (denominator / numerator / 4));
		let wholeMeasure = Math.floor(measure);
		let beat = (measure - wholeMeasure) * numerator;
		return { measure: wholeMeasure + 1, beat: beat + 1.0 };
	}

	getTickFromMeasureBeat(measureBeat: MeasureBeat): number
	{
		let conductorTrack = this.firstTrack;
		let timeSignatureEventBeat = 0;
		let timeSignatureEventMeasure = 0;
		let timeSignatureEventVisibleMeasure = 1;
		let timeSignatureEventVisibleBeat = 1;
		let numerator = 4;
		let denominator = 4;

		for (let event = conductorTrack?.firstEvent; event; event = event.nextEventInTrack)
		{
			if ((event instanceof MetaEvent) && event.isTimeSignatureEvent)
			{
				let nextTimeSignatureEventBeat = this.getBeatFromTick(event.tick);
				let nextTimeSignatureEventMeasure = timeSignatureEventMeasure + ((nextTimeSignatureEventBeat - timeSignatureEventBeat) * (denominator / numerator / 4));
				let nextTimeSignatureEventVisibleMeasure = Math.floor(nextTimeSignatureEventMeasure) + 1;
				let nextTimeSignatureEventVisibleBeat = ((nextTimeSignatureEventMeasure - (nextTimeSignatureEventVisibleMeasure - 1)) * numerator) + 1.0;
				if ((nextTimeSignatureEventVisibleMeasure > measureBeat.measure) || ((nextTimeSignatureEventVisibleMeasure == measureBeat.measure) && (nextTimeSignatureEventVisibleBeat >= measureBeat.beat))) break;
				timeSignatureEventBeat = nextTimeSignatureEventBeat;
				timeSignatureEventMeasure = nextTimeSignatureEventMeasure;
				timeSignatureEventVisibleMeasure = nextTimeSignatureEventVisibleMeasure;
				timeSignatureEventVisibleBeat = nextTimeSignatureEventVisibleBeat;
				numerator = event.timeSignatureEventNumerator;
				denominator = event.timeSignatureEventDenominator;
			}
		}

		return this.getTickFromBeat(timeSignatureEventBeat + ((((measureBeat.measure - timeSignatureEventVisibleMeasure) * numerator) + (measureBeat.beat - timeSignatureEventVisibleBeat)) * 4 / denominator));
	}

	getHourMinuteSecondFromTick(tick: number): HourMinuteSecond
	{
		let time = this.getTimeFromTick(tick);
		let wholeHour = Math.floor(time / 3600.0);
		let wholeMinute = Math.floor((time - (wholeHour * 3600.0)) / 60.0);
		let second = time - (wholeHour * 3600.0) - (wholeMinute * 60.0);
		return { hour: wholeHour, minute: wholeMinute, second };
	}

	getTickFromHourMinuteSecond(hourMinuteSecond: HourMinuteSecond): number
	{
		return this.getTickFromTime((hourMinuteSecond.hour * 3600.0) + (hourMinuteSecond.minute * 60.0) + hourMinuteSecond.second);
	}

	getHourMinuteSecondFrameFromTick(tick: number): HourMinuteSecondFrame
	{
		let time = this.getTimeFromTick(tick);
		let wholeHour = Math.floor(time / 3600.0);
		let wholeMinute = Math.floor((time - (wholeHour * 3600.0)) / 60.0);
		let wholeSecond = Math.floor(time - (wholeHour * 3600.0) - (wholeMinute * 60.0));
		let frame = time - (wholeHour * 3600.0) - (wholeMinute * 60.0) - (wholeSecond * this.numberOfFramesPerSecond);
		return { hour: wholeHour, minute: wholeMinute, second: wholeSecond, frame };
	}

	getTickFromHourMinuteSecondFrame(hourMinuteSecondFrame: HourMinuteSecondFrame): number
	{
		return this.getTickFromTime((hourMinuteSecondFrame.hour * 3600.0) + (hourMinuteSecondFrame.minute * 60.0) + hourMinuteSecondFrame.second + (hourMinuteSecondFrame.frame / this.numberOfFramesPerSecond));
	}

	getTickFromMarker(marker: string): number | null
	{
		for (let event = this.firstEvent; event; event = event.nextEventInFile)
		{
			if ((event instanceof MetaEvent) && event.isMarkerEvent && (event.markerEventText == marker)) return event.tick;
		}

		return null;
	}

	getFirstEventForTick(tick: number): Event | null
	{
		let firstEventForTick: Event | null = null;

		for (let event = this.lastEvent; event && (event.tick >= tick); event = event.previousEventInFile)
		{
			if (event.tick == tick) firstEventForTick = event;
		}

		return firstEventForTick;
	}

	getLastEventForTick(tick: number): Event | null
	{
		let lastEventForTick: Event | null = null;

		for (let event = this.firstEvent; event && (event.tick <= tick); event = event.nextEventInFile)
		{
			if (event.tick == tick) lastEventForTick = event;
		}

		return lastEventForTick;
	}

	getLatestTempoEventForTick(tick: number): MetaEvent | null
	{
		let tempoEvent: MetaEvent | null = null;
		let conductorTrack = this.firstTrack;

		for (let event = conductorTrack?.firstEvent; event && (event.tick <= tick); event = event.nextEventInTrack)
		{
			if ((event instanceof MetaEvent) && event.isTempoEvent) tempoEvent = event;
		}

		return tempoEvent;
	}

	getLatestTimeSignatureEventForTick(tick: number): MetaEvent | null
	{
		let timeSignatureEvent: MetaEvent | null = null;
		let conductorTrack = this.firstTrack;

		for (let event = conductorTrack?.firstEvent; event && (event.tick <= tick); event = event.nextEventInTrack)
		{
			if ((event instanceof MetaEvent) && event.isTimeSignatureEvent) timeSignatureEvent = event;
		}

		return timeSignatureEvent;
	}

	getLatestKeySignatureEventForTick(tick: number): MetaEvent | null
	{
		let keySignatureEvent: MetaEvent | null = null;
		let conductorTrack = this.firstTrack;

		for (let event = conductorTrack?.firstEvent; event && (event.tick <= tick); event = event.nextEventInTrack)
		{
			if ((event instanceof MetaEvent) && event.isKeySignatureEvent) keySignatureEvent = event;
		}

		return keySignatureEvent;
	}

	convertStandardEventsToNoteEvents(): void
	{
		for (let track = this.firstTrack; track; track = track.nextTrack)
		{
			for (let event = track.firstEvent; event; event = event.nextEventInTrack)
			{
				if ((event instanceof NoteOnEvent) && (event.velocity > 0))
				{
					let endEvent: NoteOffEvent | NoteOnEvent | null = null;

					for (let subsequentEvent = event.nextEventInTrack; subsequentEvent; subsequentEvent = subsequentEvent.nextEventInTrack)
					{
						if (((subsequentEvent instanceof NoteOffEvent) || ((subsequentEvent instanceof NoteOnEvent) && (subsequentEvent.velocity == 0))) && (subsequentEvent.channel == event.channel) && (subsequentEvent.note == event.note))
						{
							endEvent = subsequentEvent;
							break;
						}
					}

					if (endEvent)
					{
						let newEvent = track.createNoteEvent(event.tick, endEvent.tick - event.tick, event.channel, event.note, event.velocity, endEvent.velocity);
						newEvent.isSelected = event.isSelected;
						newEvent.moveImmediatelyAfter(event);
						event.delete();
						endEvent.delete();
						event = newEvent;
					}
				}
			}
		}
	}

	convertNoteEventsToStandardEvents(): void
	{
		for (let track = this.firstTrack; track; track = track.nextTrack)
		{
			for (let event = track.firstEvent; event; event = event.nextEventInTrack)
			{
				if (event instanceof NoteEvent)
				{
					let noteOnEvent = track.createNoteOnEvent(event.tick, event.channel, event.note, event.velocity);
					let noteOffEvent = track.createNoteOffEvent(event.tick + event.durationTicks, event.channel, event.note, event.endVelocity);
					noteOnEvent.isSelected = event.isSelected;
					noteOffEvent.isSelected = event.isSelected;
					noteOnEvent.moveImmediatelyAfter(event);
					event.delete();
					event = noteOnEvent;
				}
			}
		}
	}

	convertSelectionFlagsToTextEvents(label: string): void
	{
		for (let track = this.firstTrack; track; track = track.nextTrack)
		{
			for (let event = track.firstEvent; event; event = event.nextEventInTrack)
			{
				if (event.isSelected)
				{
					track.createTextEvent(event.tick, label).moveImmediatelyBefore(event);
					event.isSelected = false;
				}
			}
		}
	}

	convertTextEventsToSelectionFlags(label: string): void
	{
		for (let track = this.firstTrack; track; track = track.nextTrack)
		{
			let nextEvent: Event | null = null;

			for (let event = track.firstEvent; event; event = nextEvent)
			{
				nextEvent = event.nextEventInTrack;

				if ((event instanceof MetaEvent) && event.isTextEvent && (event.textEventText == label))
				{
					if (nextEvent) nextEvent.isSelected = true;
					event.delete();
				}
			}
		}
	}
}

export class Track
{
	_midiFile: MidiFile;
	_number = 0;
	_endTick = 0;
	_previousTrack: Track | null = null;
	_nextTrack: Track | null = null;
	_firstEvent: Event | null = null;
	_lastEvent: Event | null = null;

	protected constructor(midiFile: MidiFile)
	{
		this._midiFile = midiFile;
	}

	static _createTrack(midiFile: MidiFile): Track
	{
		return new Track(midiFile);
	}

	get midiFile(): MidiFile
	{
		return this._midiFile;
	}

	get number(): number
	{
		return this._number;
	}

	get endTick(): number
	{
		return this._endTick;
	}

	set endTick(endTick: number)
	{
		if (this._lastEvent && (endTick < this._lastEvent.tick)) return;
		this._endTick = endTick;
	}

	get previousTrack(): Track | null
	{
		return this._previousTrack;
	}

	get nextTrack(): Track | null
	{
		return this._nextTrack;
	}

	get firstEvent(): Event | null
	{
		return this._firstEvent;
	}

	get lastEvent(): Event | null
	{
		return this._lastEvent;
	}

	get events(): Array<Event>
	{
		let events: Array<Event> = [];
		for (let event = this.firstEvent; event; event = event.nextEventInTrack) events.push(event);
		return events;
	}

	delete(): void
	{
		for (let subsequentTrack = this._nextTrack; subsequentTrack; subsequentTrack = subsequentTrack.nextTrack) subsequentTrack._number--;
	
		this._midiFile._numberOfTracks--;

		if (this._previousTrack)
		{
			this._previousTrack._nextTrack = this._nextTrack;
		}
		else
		{
			this._midiFile._firstTrack = this._nextTrack;
		}

		if (this._nextTrack)
		{
			this._nextTrack._previousTrack = this._previousTrack;
		}
		else
		{
			this._midiFile._lastTrack = this._previousTrack;
		}

		let nextEventInTrack: Event | null = null;

		for (let event = this._firstEvent; event; event = nextEventInTrack)
		{
			nextEventInTrack = event._nextEventInTrack;
			event.delete();
		}
	}

	getFirstEventForTick(tick: number): Event | null
	{
		let firstEventForTick: Event | null = null;

		for (let event = this.lastEvent; event && (event.tick >= tick); event = event.previousEventInTrack)
		{
			if (event.tick == tick) firstEventForTick = event;
		}

		return firstEventForTick;
	}

	getLastEventForTick(tick: number): Event | null
	{
		let lastEventForTick: Event | null = null;

		for (let event = this.firstEvent; event && (event.tick <= tick); event = event.nextEventInTrack)
		{
			if (event.tick == tick) lastEventForTick = event;
		}

		return lastEventForTick;
	}

	createTrackBefore(): Track
	{
		let newTrack: Track = Track._createTrack(this._midiFile);
		newTrack._number = this._number;
		newTrack._previousTrack = this._previousTrack;
		newTrack._nextTrack = this;
		this._previousTrack = newTrack;

		if (newTrack._previousTrack)
		{
			newTrack._previousTrack._nextTrack = newTrack;
		}
		else
		{
			this._midiFile._firstTrack = newTrack;
		}

		for (let subsequentTrack: Track | null = this; subsequentTrack; subsequentTrack = subsequentTrack._nextTrack) subsequentTrack._number++;

		return newTrack;
	}

	createTrackAfter(): Track
	{
		if (this._nextTrack)
		{
			return this._nextTrack.createTrackBefore();
		}
		else
		{
			return this._midiFile.createTrack();
		}
	}

	createVoiceEvent(tick: number, messageData: Uint8Array): VoiceEvent
	{
		return VoiceEvent._createVoiceEvent(this, tick, messageData);
	}

	createNoteOffEvent(tick: number, channel: number, note: number, velocity: number): NoteOffEvent
	{
		return NoteOffEvent._createNoteOffEvent(this, tick, channel, note, velocity);
	}

	createNoteOnEvent(tick: number, channel: number, note: number, velocity: number): NoteOnEvent
	{
		return NoteOnEvent._createNoteOnEvent(this, tick, channel, note, velocity);
	}

	createKeyPressureEvent(tick: number, channel: number, note: number, amount: number): KeyPressureEvent
	{
		return KeyPressureEvent._createKeyPressureEvent(this, tick, channel, note, amount);
	}

	createControlChangeEvent(tick: number, channel: number, number: number, value: number): ControlChangeEvent
	{
		return ControlChangeEvent._createControlChangeEvent(this, tick, channel, number, value);
	}

	createProgramChangeEvent(tick: number, channel: number, number: number): ProgramChangeEvent
	{
		return ProgramChangeEvent._createProgramChangeEvent(this, tick, channel, number);
	}

	createChannelPressureEvent(tick: number, channel: number, amount: number): ChannelPressureEvent
	{
		return ChannelPressureEvent._createChannelPressureEvent(this, tick, channel, amount);
	}

	createPitchWheelEvent(tick: number, channel: number, value: number): PitchWheelEvent
	{
		return PitchWheelEvent._createPitchWheelEvent(this, tick, channel, value);
	}

	createSysexEvent(tick: number, data: Uint8Array): SysexEvent
	{
		return SysexEvent._createSysexEvent(this, tick, data);
	}

	createMetaEvent(tick: number, number: number, data: Uint8Array): MetaEvent
	{
		return MetaEvent._createMetaEvent(this, tick, number, data);
	}
  
	createTextEvent(tick: number, text: string): MetaEvent
	{
		return MetaEvent._createTextEvent(this, tick, text);
	}

	createLyricEvent(tick: number, text: string): MetaEvent
	{
		return MetaEvent._createLyricEvent(this, tick, text);
	}

	createMarkerEvent(tick: number, text: string): MetaEvent
	{
		return MetaEvent._createMarkerEvent(this, tick, text);
	}

	createPortEvent(tick: number, name: string): MetaEvent
	{
		return MetaEvent._createPortEvent(this, tick, name);
	}

	createTempoEvent(tick: number, tempo: number): MetaEvent
	{
		return MetaEvent._createTempoEvent(this, tick, tempo);
	}

	createTimeSignatureEvent(tick: number, numerator: number, denominator: number): MetaEvent
	{
		return MetaEvent._createTimeSignatureEvent(this, tick, numerator, denominator);
	}

	createKeySignatureEvent(tick: number, numberOfSharps: number, isMinor: boolean): MetaEvent
	{
		return MetaEvent._createKeySignatureEvent(this, tick, numberOfSharps, isMinor);
	}

	createNoteEvent(tick: number, durationTicks: number, channel: number, note: number, velocity: number, endVelocity: number): NoteEvent
	{
		return NoteEvent._createNoteEvent(this, tick, durationTicks, channel, note, velocity, endVelocity);
	}
}

export abstract class Event
{
	_tick: number;
	_track: Track | null = null;
	_previousEventInTrack: Event | null = null;
	_nextEventInTrack: Event | null = null;
	_previousEventInFile: Event | null = null;
	_nextEventInFile: Event | null = null;
	_isSelected = false;

	protected constructor(track: Track | null, tick: number)
	{
		this._tick = tick;
		this.track = track;
	}

	get tick(): number
	{
		return this._tick;
	}

	set tick(tick: number)
	{
		this._tick = tick;
		this.track = this._track; // re-insert at the new tick-sorted location
	}

	get track(): Track | null
	{
		return this._track;
	}

	set track(track: Track | null)
	{
		if (this._track)
		{
			if (this._previousEventInTrack)
			{
				this._previousEventInTrack._nextEventInTrack = this._nextEventInTrack;
				this._previousEventInTrack = null;
			}
			else
			{
				this._track._firstEvent = this._nextEventInTrack;
			}

			if (this._nextEventInTrack)
			{
				this._nextEventInTrack._previousEventInTrack = this._previousEventInTrack;
				this._nextEventInTrack = null;
			}
			else
			{
				this._track._lastEvent = this._previousEventInTrack;
			}

			if (this._previousEventInFile)
			{
				this._previousEventInFile._nextEventInFile = this._nextEventInFile;
				this._previousEventInFile = null;
			}
			else
			{
				this._track._midiFile._firstEvent = this._nextEventInFile;
			}

			if (this._nextEventInFile)
			{
				this._nextEventInFile._previousEventInFile = this._previousEventInFile;
				this._nextEventInFile = null;
			}
			else
			{
				this._track._midiFile._lastEvent = this._previousEventInFile;
			}

			this._track = null;
		}

		if (track)
		{
			this._track = track;

			let event: Event | null = null;

			for (event = this._track._lastEvent; event && (event._tick > this._tick); event = event._previousEventInTrack) {}

			this._previousEventInTrack = event;

			if (event)
			{
				this._nextEventInTrack = event._nextEventInTrack;
				event._nextEventInTrack = this;
			}
			else
			{
				this._nextEventInTrack = this._track._firstEvent;
				this._track._firstEvent = this;
			}

			if (this._nextEventInTrack)
			{
				this._nextEventInTrack._previousEventInTrack = this;
			}
			else
			{
				this._track._lastEvent = this;
			}

			for (event = this._track._midiFile._lastEvent; event && (event._tick > this._tick); event = event._previousEventInFile) {}

			this._previousEventInFile = event;

			if (event)
			{
				this._nextEventInFile = event._nextEventInFile;
				event._nextEventInFile = this;
			}
			else
			{
				this._nextEventInFile = this._track._midiFile._firstEvent;
				this._track._midiFile._firstEvent = this;
			}

			if (this._nextEventInFile)
			{
				this._nextEventInFile._previousEventInFile = this;
			}
			else
			{
				this._track._midiFile._lastEvent = this;
			}

			if (this._tick > this._track._endTick) this._track._endTick = this._tick;
		}
	}

	get previousEventInTrack(): Event | null
	{
		return this._previousEventInTrack;
	}

	get nextEventInTrack(): Event | null
	{
		return this._nextEventInTrack;
	}

	get previousEventInFile(): Event | null
	{
		return this._previousEventInFile;
	}

	get nextEventInFile(): Event | null
	{
		return this._nextEventInFile;
	}

	delete(): void
	{
		this.track = null;
	}

	moveImmediatelyBefore(nextEvent: Event): void
	{
		if (!nextEvent._track) throw Error("target event is detached");

		this.track = null;
		this._tick = nextEvent._tick;
		this._track = nextEvent._track;
		this._nextEventInTrack = nextEvent;
		this._previousEventInTrack = nextEvent._previousEventInTrack;
		nextEvent._previousEventInTrack = this;

		if (this._previousEventInTrack)
		{
			this._previousEventInTrack._nextEventInTrack = this;
		}
		else
		{
			this._track._firstEvent = this;
		}

		this._nextEventInFile = nextEvent;
		this._previousEventInFile = nextEvent._previousEventInFile;
		nextEvent._previousEventInFile = this;

		if (this._previousEventInFile)
		{
			this._previousEventInFile._nextEventInFile = this;
		}
		else
		{
			this._track._firstEvent = this;
		}
	}

	moveImmediatelyAfter(previousEvent: Event): void
	{
		if (!previousEvent._track) throw Error("target event is detached");

		this.track = null;
		this._tick = previousEvent._tick;
		this._track = previousEvent._track;
		this._previousEventInTrack = previousEvent;
		this._nextEventInTrack = previousEvent._nextEventInTrack;
		previousEvent._nextEventInTrack = this;

		if (this._nextEventInTrack)
		{
			this._nextEventInTrack._previousEventInTrack = this;
		}
		else
		{
			this._track._lastEvent = this;
		}

		this._previousEventInFile = previousEvent;
		this._nextEventInFile = previousEvent._nextEventInFile;
		previousEvent._nextEventInFile = this;

		if (this._nextEventInFile)
		{
			this._nextEventInFile._previousEventInFile = this;
		}
		else
		{
			this._track._midiFile._lastEvent = this;
		}
	}

	abstract clone(): Event;

	get isSelected(): boolean
	{
		return this._isSelected;
	}

	set isSelected(isSelected: boolean)
	{
		this._isSelected = isSelected;
	}
}

export abstract class VoiceEvent extends Event
{
	_channel: number;

	protected constructor(track: Track | null, tick: number, channel: number)
	{
		super(track, tick);
		this._channel = channel;
	}

	static _createVoiceEvent(track: Track | null, tick: number, messageData: Uint8Array): VoiceEvent
	{
		switch (messageData[0] & 0xF0)
		{
			case 0x80:
			{
				return NoteOffEvent._createNoteOffEvent(track, tick, messageData[0] & 0x0F, messageData[1], messageData[2]);
			}
			case 0x90:
			{
				return NoteOnEvent._createNoteOnEvent(track, tick, messageData[0] & 0x0F, messageData[1], messageData[2]);
			}
			case 0xA0:
			{
				return KeyPressureEvent._createKeyPressureEvent(track, tick, messageData[0] & 0x0F, messageData[1], messageData[2]);
			}
			case 0xB0:
			{
				return ControlChangeEvent._createControlChangeEvent(track, tick, messageData[0] & 0x0F, messageData[1], messageData[2]);
			}
			case 0xC0:
			{
				return ProgramChangeEvent._createProgramChangeEvent(track, tick, messageData[0] & 0x0F, messageData[1]);
			}
			case 0xD0:
			{
				return ChannelPressureEvent._createChannelPressureEvent(track, tick, messageData[0] & 0x0F, messageData[1]);
			}
			case 0xE0:
			{
				return PitchWheelEvent._createPitchWheelEvent(track, tick, messageData[0] & 0x0F, (messageData[2] << 7) | messageData[1]);
			}
			default:
			{
				throw new Error("unrecognized message");
			}
		}
	}

	get channel(): number
	{
		return this._channel;
	}

	set channel(channel: number)
	{
		this._channel = channel;
	}

	abstract get messageData(): Uint8Array;
}

export class NoteOffEvent extends VoiceEvent
{
	_note: number;
	_velocity: number;

	protected constructor(track: Track | null, tick: number, channel: number, note: number, velocity: number)
	{
		super(track, tick, channel);
		this._note = note;
		this._velocity = velocity;
	}

	static _createNoteOffEvent(track: Track | null, tick: number, channel: number, note: number, velocity: number): NoteOffEvent
	{
		return new NoteOffEvent(track, tick, channel, note, velocity);
	}

	get note(): number
	{
		return this._note;
	}

	set note(note: number)
	{
		this._note = note;
	}

	get velocity(): number
	{
		return this._velocity;
	}

	set velocity(velocity: number)
	{
		this._velocity = velocity;
	}

	get messageData(): Uint8Array
	{
		return new Uint8Array([0x80 | this.channel, this.note, this.velocity]);
	}

	clone(): NoteOffEvent
	{
		return NoteOffEvent._createNoteOffEvent(null, this.tick, this.channel, this.note, this.velocity);
	}
}

export class NoteOnEvent extends VoiceEvent
{
	_note: number;
	_velocity: number;

	protected constructor(track: Track | null, tick: number, channel: number, note: number, velocity: number)
	{
		super(track, tick, channel);
		this._note = note;
		this._velocity = velocity;
	}

	static _createNoteOnEvent(track: Track | null, tick: number, channel: number, note: number, velocity: number): NoteOnEvent
	{
		return new NoteOnEvent(track, tick, channel, note, velocity);
	}

	get note(): number
	{
		return this._note;
	}

	set note(note: number)
	{
		this._note = note;
	}

	get velocity(): number
	{
		return this._velocity;
	}

	set velocity(velocity: number)
	{
		this._velocity = velocity;
	}

	get messageData(): Uint8Array
	{
		return new Uint8Array([0x90 | this.channel, this.note, this.velocity]);
	}

	clone(): NoteOnEvent
	{
		return NoteOnEvent._createNoteOnEvent(null, this.tick, this.channel, this.note, this.velocity);
	}
}

export class KeyPressureEvent extends VoiceEvent
{
	_note: number;
	_amount: number;

	protected constructor(track: Track | null, tick: number, channel: number, note: number, amount: number)
	{
		super(track, tick, channel);
		this._note = note;
		this._amount = amount;
	}

	static _createKeyPressureEvent(track: Track | null, tick: number, channel: number, note: number, amount: number): KeyPressureEvent
	{
		return new KeyPressureEvent(track, tick, channel, note, amount);
	}

	get note(): number
	{
		return this._note;
	}

	set note(note: number)
	{
		this._note = note;
	}

	get amount(): number
	{
		return this._amount;
	}

	set amount(amount: number)
	{
		this._amount = amount;
	}

	get messageData(): Uint8Array
	{
		return new Uint8Array([0xA0 | this.channel, this.note, this.amount]);
	}

	clone(): KeyPressureEvent
	{
		return KeyPressureEvent._createKeyPressureEvent(null, this.tick, this.channel, this.note, this.amount);
	}
}

export class ControlChangeEvent extends VoiceEvent
{
	_number: number;
	_value: number;

	protected constructor(track: Track | null, tick: number, channel: number, number: number, value: number)
	{
		super(track, tick, channel);
		this._number = number;
		this._value = value;
	}

	static _createControlChangeEvent(track: Track | null, tick: number, channel: number, number: number, value: number): ControlChangeEvent
	{
		return new ControlChangeEvent(track, tick, channel, number, value);
	}

	get number(): number
	{
		return this._number;
	}

	set number(number: number)
	{
		this._number = number;
	}

	get value(): number
	{
		return this._value;
	}

	set value(value: number)
	{
		this._value = value;
	}

	get messageData(): Uint8Array
	{
		return new Uint8Array([0xB0 | this.channel, this.number, this.value]);
	}

	clone(): ControlChangeEvent
	{
		return ControlChangeEvent._createControlChangeEvent(null, this.tick, this.channel, this.number, this.value);
	}
}

export class ProgramChangeEvent extends VoiceEvent
{
	_number: number;

	protected constructor(track: Track | null, tick: number, channel: number, number: number)
	{
		super(track, tick, channel);
		this._number = number;
	}

	static _createProgramChangeEvent(track: Track | null, tick: number, channel: number, number: number): ProgramChangeEvent
	{
		return new ProgramChangeEvent(track, tick, channel, number);
	}

	get number(): number
	{
		return this._number;
	}

	set number(number: number)
	{
		this._number = number;
	}

	get messageData(): Uint8Array
	{
		return new Uint8Array([0xC0 | this.channel, this.number]);
	}

	clone(): ProgramChangeEvent
	{
		return ProgramChangeEvent._createProgramChangeEvent(null, this.tick, this.channel, this.number);
	}
}

export class ChannelPressureEvent extends VoiceEvent
{
	_amount: number;

	protected constructor(track: Track | null, tick: number, channel: number, amount: number)
	{
		super(track, tick, channel);
		this._amount = amount;
	}

	static _createChannelPressureEvent(track: Track | null, tick: number, channel: number, amount: number): ChannelPressureEvent
	{
		return new ChannelPressureEvent(track, tick, channel, amount);
	}

	get amount(): number
	{
		return this._amount;
	}

	set amount(amount: number)
	{
		this._amount = amount;
	}

	get messageData(): Uint8Array
	{
		return new Uint8Array([0xD0 | this.channel, this.amount]);
	}

	clone(): ChannelPressureEvent
	{
		return ChannelPressureEvent._createChannelPressureEvent(null, this.tick, this.channel, this.amount);
	}
}

export class PitchWheelEvent extends VoiceEvent
{
	_value: number;

	protected constructor(track: Track | null, tick: number, channel: number, value: number)
	{
		super(track, tick, channel);
		this._value = value;
	}

	static _createPitchWheelEvent(track: Track | null, tick: number, channel: number, value: number): PitchWheelEvent
	{
		return new PitchWheelEvent(track, tick, channel, value);
	}

	get value(): number
	{
		return this._value;
	}

	set value(value: number)
	{
		this._value = value;
	}

	get messageData(): Uint8Array
	{
		return new Uint8Array([0xE0 | this.channel, this.value & 0x0F, this.value >> 7]);
	}

	clone(): PitchWheelEvent
	{
		return PitchWheelEvent._createPitchWheelEvent(null, this.tick, this.channel, this.value);
	}
}

export class SysexEvent extends Event
{
	_data: Uint8Array;

	protected constructor(track: Track | null, tick: number, data: Uint8Array)
	{
		super(track, tick);
		this._data = data;
	}

	static _createSysexEvent(track: Track | null, tick: number, data: Uint8Array): SysexEvent
	{
		return new SysexEvent(track, tick, data);
	}

	get data(): Uint8Array
	{
		return this._data;
	}

	set data(data: Uint8Array)
	{
		this._data = data;
	}

	clone(): SysexEvent
	{
		return SysexEvent._createSysexEvent(null, this.tick, this.data);
	}
}

export class MetaEvent extends Event
{
	_number: number;
	_data: Uint8Array;

	protected constructor(track: Track | null, tick: number, number: number, data: Uint8Array)
	{
		super(track, tick);
		this._number = number;
		this._data = data
	}

	static _createMetaEvent(track: Track | null, tick: number, number: number, data: Uint8Array): MetaEvent
	{
		return new MetaEvent(track, tick, number, data);
	}

	get number(): number
	{
		return this._number;
	}

	get data(): Uint8Array
	{
		return this._data;
	}

	set data(data: Uint8Array)
	{
		this._data = data;
	}

	static _createTextEvent(track: Track | null, tick: number, text: string): MetaEvent
	{
		let event = new MetaEvent(track, tick, 0x1, new Uint8Array(0));
		event.textEventText = text;
		return event;
	}

	get isTextEvent(): boolean
	{
		return this.number == 0x1;
	}

	get textEventText(): string
	{
		if (!this.isTextEvent) throw new Error("not a text event");
		return new TextDecoder().decode(this.data);
	}

	set textEventText(text: string)
	{
		if (!this.isTextEvent) throw new Error("not a text event");
		this.data = new TextEncoder().encode(text);
	}

	static _createLyricEvent(track: Track | null, tick: number, text: string): MetaEvent
	{
		let event = new MetaEvent(track, tick, 0x5, new Uint8Array(0));
		event.lyricEventText = text;
		return event;
	}

	get isLyricEvent(): boolean
	{
		return this.number == 0x5;
	}

	get lyricEventText(): string
	{
		if (!this.isLyricEvent) throw new Error("not a lyric event");
		return new TextDecoder().decode(this.data);
	}

	set lyricEventText(text: string)
	{
		if (!this.isLyricEvent) throw new Error("not a lyric event");
		this.data = new TextEncoder().encode(text);
	}

	static _createMarkerEvent(track: Track | null, tick: number, text: string): MetaEvent
	{
		let event = new MetaEvent(track, tick, 0x6, new Uint8Array(0));
		event.markerEventText = text;
		return event;
	}

	get isMarkerEvent(): boolean
	{
		return this.number == 0x6;
	}

	get markerEventText(): string
	{
		if (!this.isMarkerEvent) throw new Error("not a marker event");
		return new TextDecoder().decode(this.data);
	}

	set markerEventText(text: string)
	{
		if (!this.isMarkerEvent) throw new Error("not a marker event");
		this.data = new TextEncoder().encode(text);
	}

	static _createPortEvent(track: Track | null, tick: number, name: string): MetaEvent
	{
		let event = new MetaEvent(track, tick, 0x9, new Uint8Array(0));
		event.portEventName = name;
		return event;
	}

	get isPortEvent(): boolean
	{
		return this.number == 0x9;
	}

	get portEventName(): string
	{
		if (!this.isPortEvent) throw new Error("not a port event");
		return new TextDecoder().decode(this.data);
	}

	set portEventName(name: string)
	{
		if (!this.isPortEvent) throw new Error("not a port event");
		this.data = new TextEncoder().encode(name);
	}

	static _createTempoEvent(track: Track | null, tick: number, tempo: number): MetaEvent
	{
		let event = new MetaEvent(track, tick, 0x51, new Uint8Array(3));
		event.tempoEventTempo = tempo;
		return event;
	}

	get isTempoEvent(): boolean
	{
		return this.number == 0x51;
	}

	get tempoEventTempo(): number
	{
		if (!this.isTempoEvent) throw new Error("not a tempo event");
		let midiTempo: number = (this.data[0] << 16) | (this.data[1] << 8) | this.data[2];
		return (60000000.0 / midiTempo);
	}

	set tempoEventTempo(tempo: number)
	{
		if (!this.isTempoEvent) throw new Error("not a tempo event");
		let midiTempo: number = Math.floor(60000000 / tempo);
		this.data = new Uint8Array([(midiTempo >> 16) & 0xFF, (midiTempo >> 8) & 0xFF, midiTempo && 0xFF]);
	}

	static _createTimeSignatureEvent(track: Track | null, tick: number, numerator: number, denominator: number): MetaEvent
	{
		let event = new MetaEvent(track, tick, 0x58, new Uint8Array(4));
		event.timeSignatureEventNumerator = numerator;
		event.timeSignatureEventDenominator = denominator;
		return event;
	}

	get isTimeSignatureEvent(): boolean
	{
		return this.number == 0x58;
	}

	get timeSignatureEventNumerator(): number
	{
		if (!this.isTimeSignatureEvent) throw new Error("not a time signature event");
		return this.data[0];
	}

	set timeSignatureEventNumerator(numerator: number)
	{
		if (!this.isTimeSignatureEvent) throw new Error("not a time signature event");
		this.data[0] = numerator;
	}

	get timeSignatureEventDenominator(): number
	{
		if (!this.isTimeSignatureEvent) throw new Error("not a time signature event");
		return Math.pow(2, this.data[1]);
	}

	set timeSignatureEventDenominator(denominator: number)
	{
		if (!this.isTimeSignatureEvent) throw new Error("not a time signature event");
		this.data[1] = Math.log2(denominator);
		this.data[2] = 96 / denominator;
		this.data[3] = 8;
	}

	static _createKeySignatureEvent(track: Track | null, tick: number, numberOfSharps: number, isMinor: boolean): MetaEvent
	{
		let event = new MetaEvent(track, tick, 0x59, new Uint8Array(2));
		event.keySignatureEventNumberOfSharps = numberOfSharps;
		event.keySignatureEventIsMinor = isMinor;
		return event;
	}

	get isKeySignatureEvent(): boolean
	{
		return this.number == 0x59;
	}

	get keySignatureEventNumberOfSharps(): number
	{
		if (!this.isKeySignatureEvent) throw new Error("not a key signature event");
		return new Int8Array(this.data.buffer)[0];
	}

	set keySignatureEventNumberOfSharps(numberOfSharps: number)
	{
		if (!this.isKeySignatureEvent) throw new Error("not a key signature event");
		new Int8Array(this.data.buffer)[0] = numberOfSharps;
	}

	get keySignatureEventIsMinor(): boolean
	{
		if (!this.isKeySignatureEvent) throw new Error("not a key signature event");
		return (this.data[1] == 1);
	}

	set keySignatureEventIsMinor(isMinor: boolean)
	{
		if (!this.isKeySignatureEvent) throw new Error("not a key signature event");
		this.data[1] = isMinor ? 1 : 0;
	}

	clone(): MetaEvent
	{
		return MetaEvent._createMetaEvent(null, this.tick, this.number, this.data);
	}
}

export class NoteEvent extends Event
{
	_durationTicks = 0;
	_channel = 0;
	_note = 0;
	_velocity = 0;
	_endVelocity = 0;

	protected constructor(track: Track | null, tick: number, durationTicks: number, channel: number, note: number, velocity: number, endVelocity: number)
	{
		super(track, tick);
		this._durationTicks = durationTicks;
		this._channel = channel;
		this._note = note;
		this._velocity = velocity;
		this._endVelocity = endVelocity;
	}

	static _createNoteEvent(track: Track | null, tick: number, durationTicks: number, channel: number, note: number, velocity: number, endVelocity: number): NoteEvent
	{
		return new NoteEvent(track, tick, durationTicks, channel, note, velocity, endVelocity);
	}

	get durationTicks(): number
	{
		return this._durationTicks;
	}

	set durationTicks(durationTicks: number)
	{
		this._durationTicks = durationTicks;
	}

	get channel(): number
	{
		return this._channel;
	}

	set channel(channel: number)
	{
		this._channel = channel;
	}

	get note(): number
	{
		return this._note;
	}

	set note(note: number)
	{
		this._note = note;
	}

	get velocity(): number
	{
		return this._velocity;
	}

	set velocity(velocity: number)
	{
		this._velocity = velocity;
	}

	get endVelocity(): number
	{
		return this._endVelocity;
	}

	set endVelocity(endVelocity: number)
	{
		this._endVelocity = endVelocity;
	}

	clone(): NoteEvent
	{
		return NoteEvent._createNoteEvent(null, this.tick, this.durationTicks, this.channel, this.note, this.velocity, this.endVelocity);
	}
}

export interface MeasureBeat
{
	measure: number;
	beat: number;
}

export interface HourMinuteSecond
{
	hour: number;
	minute: number;
	second: number;
}

export interface HourMinuteSecondFrame
{
	hour: number;
	minute: number;
	second: number;
	frame: number;
}

export type DivisionType = "PPQ" | "SMPTE24" | "SMPTE25" | "SMPTE30DROP" | "SMPTE30";

class IO
{
	buffer: Uint8Array | null;
	offset = 0;

	constructor(buffer: Uint8Array | null)
	{
		this.buffer = buffer;
	}

	getc(): number
	{
		let offset = this.offset++;
		return this.buffer ? this.buffer[offset] : 0;
	}

	putc(c: number): void
	{
		let offset = this.offset++;
		if (this.buffer) this.buffer[offset] = c;
	}

	read(length: number): Uint8Array
	{
		let offset = this.offset;
		this.offset += length;
		return this.buffer ? this.buffer.slice(offset, offset + length) : new Uint8Array(length);
	}

	write(buffer: Uint8Array): void
	{
		if (this.buffer) this.buffer.set(buffer, this.offset);
		this.offset += buffer.length;
	}

	tell(): number
	{
		return this.offset;
	}

	seek(offset: number, whence: "SET" | "CUR"): void
	{
		switch (whence)
		{
			case "SET":
			{
				this.offset = offset;
				break;
			}
			case "CUR":
			{
				this.offset += offset;
				break;
			}
		}
	}

	readUint16(): number
	{
		return ((this.getc() << 8) | this.getc());
	}

	writeUint16(value: number): void
	{
		this.putc((value >> 8) & 0xFF);
		this.putc(value & 0xFF);
	}

	readUint32(): number
	{
		return (this.getc() << 24) | (this.getc() << 16) | (this.getc() << 8) | this.getc();
	}

	writeUint32(value: number): void
	{
		this.putc((value >> 24) & 0xFF);
		this.putc((value >> 16) & 0xFF);
		this.putc((value >> 8) & 0xFF);
		this.putc(value & 0xFF);
	}

	readVariableLengthQuantity(): number
	{
		let b = 0;
		let value = 0;

		do
		{
			b = this.getc();
			value = (value << 7) | (b & 0x7F);
		}
		while ((b & 0x80) == 0x80);

		return value;
	}

	writeVariableLengthQuantity(value: number): void
	{
		let buffer: Array<number> = [];

		do
		{
			buffer.unshift((value & 0x7F) | (buffer.length == 0 ? 0 : 0x80));
			value >>= 7;
		}
		while (value > 0);

		this.write(new Uint8Array(buffer));
	}
}
