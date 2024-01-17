
import * as fs from "fs";
import * as util from "util"
import * as zlib from "zlib";
import * as fastxmlparser from "fast-xml-parser";
import * as midifile from "@sreal/midifile";

function abletonLiveToSmf(alsFileBytes: Uint8Array): Uint8Array
{
	let alsFile = new fastxmlparser.XMLParser({
		ignoreAttributes: false,
		attributesGroupName: "_attributes",
		attributeNamePrefix: "",
		isArray: (name, jPath, isLeafNode, isAttribute) => [
			"Ableton.LiveSet.MasterTrack.AutomationEnvelopes.Envelopes.AutomationEnvelope",
			"Ableton.LiveSet.MasterTrack.AutomationEnvelopes.Envelopes.AutomationEnvelope.Automation.Events.EnumEvent",
			"Ableton.LiveSet.MasterTrack.AutomationEnvelopes.Envelopes.AutomationEnvelope.Automation.Events.FloatEvent",
			"Ableton.LiveSet.Tracks.MidiTrack",
			"Ableton.LiveSet.Tracks.MidiTrack.DeviceChain.MainSequencer.ClipTimeable.ArrangerAutomation.Events.MidiClip.Envelopes.Envelopes.ClipEnvelope",
			"Ableton.LiveSet.Tracks.MidiTrack.DeviceChain.MainSequencer.ClipTimeable.ArrangerAutomation.Events.MidiClip.Envelopes.Envelopes.ClipEnvelope.Automation.Events.FloatEvent",
			"Ableton.LiveSet.Tracks.MidiTrack.DeviceChain.MainSequencer.ClipTimeable.ArrangerAutomation.Events.MidiClip.Notes.KeyTracks.KeyTrack",
			"Ableton.LiveSet.Tracks.MidiTrack.DeviceChain.MainSequencer.ClipTimeable.ArrangerAutomation.Events.MidiClip.Notes.KeyTracks.KeyTrack.Notes.MidiNoteEvent"
		].includes(jPath)
	}).parse(zlib.gunzipSync(alsFileBytes));

	if (!((alsFile?.Ableton?._attributes?.SchemaChangeCount == 3) && alsFile?.Ableton?.LiveSet)) throw new Error("not a recognized Ableton Live Set file");
	let midiFile = new midifile.MidiFile();
	let conductorTrack = midiFile.createTrack();
	let alsTempoAutomationId = alsFile?.Ableton?.LiveSet?.MasterTrack?.DeviceChain?.Mixer?.Tempo?.AutomationTarget?._attributes?.Id;
	let alsTimeSignatureAutomationId = alsFile?.Ableton?.LiveSet?.MasterTrack?.DeviceChain?.Mixer?.TimeSignature?.AutomationTarget?._attributes?.Id;

	for (let alsAutomationEnvelope of alsFile?.Ableton?.LiveSet?.MasterTrack?.AutomationEnvelopes?.Envelopes?.AutomationEnvelope ?? [])
	{
		if (alsAutomationEnvelope?.EnvelopeTarget?.PointeeId?._attributes?.Value == alsTempoAutomationId)
		{
			for (let alsFloatEvent of alsAutomationEnvelope?.Automation?.Events?.FloatEvent ?? [])
			{
				let beat = Math.max(parseFloat(alsFloatEvent?._attributes?.Time), 0);
				let tempo = parseFloat(alsFloatEvent?._attributes?.Value);
				conductorTrack.createTempoEvent(midiFile.getTickFromBeat(beat), tempo);
			}
		}
		else if (alsAutomationEnvelope?.EnvelopeTarget?.PointeeId?._attributes?.Value == alsTimeSignatureAutomationId)
		{
			for (let alsEnumEvent of alsAutomationEnvelope?.Automation?.Events?.EnumEvent ?? [])
			{
				let beat = Math.max(parseFloat(alsEnumEvent?._attributes?.Time), 0);
				let alsTimeSignature = parseInt(alsEnumEvent?._attributes?.Value);
				let numerator = (alsTimeSignature % 99) + 1;
				let denominator = Math.pow(2, Math.floor(alsTimeSignature / 99));
				conductorTrack.createTimeSignatureEvent(midiFile.getTickFromBeat(beat), numerator, denominator);
			}
		}
	}

	for (let alsMidiTrack of alsFile?.Ableton?.LiveSet?.Tracks?.MidiTrack ?? [])
	{
		let track = midiFile.createTrack();
	
		for (let alsKeyTrack of alsMidiTrack?.DeviceChain?.MainSequencer?.ClipTimeable?.ArrangerAutomation?.Events?.MidiClip?.Notes?.KeyTracks?.KeyTrack ?? [])
		{
			let note = alsKeyTrack?.MidiKey?._attributes?.Value;

			for (let alsMidiNoteEvent of alsKeyTrack?.Notes?.MidiNoteEvent ?? [])
			{
				let beat = Math.max(parseFloat(alsMidiNoteEvent?._attributes?.Time), 0);
				let durationBeats = parseFloat(alsMidiNoteEvent?._attributes?.Duration);
				let velocity = parseInt(alsMidiNoteEvent?._attributes?.Velocity);
				let releaseVelocity = parseInt(alsMidiNoteEvent?._attributes?.OffVelocity);
				let channel = 0;
				track.createNoteOnEvent(midiFile.getTickFromBeat(beat), channel, note, velocity);
				track.createNoteOffEvent(midiFile.getTickFromBeat(beat + durationBeats), channel, note, releaseVelocity);
			}
		}

		for (let alsClipEnvelope of alsMidiTrack?.DeviceChain?.MainSequencer?.ClipTimeable?.ArrangerAutomation?.Events?.MidiClip?.Envelopes?.Envelopes?.ClipEnvelope ?? [])
		{
			let alsControllerId = alsClipEnvelope?.EnvelopeTarget?.PointeeId?._attributes?.Value;

			for (let alsControllerNumber = 0; alsControllerNumber < 130; alsControllerNumber++)
			{
				if (alsMidiTrack?.DeviceChain?.MainSequencer?.MidiControllers?.["ControllerTargets." + alsControllerNumber]?._attributes?.Id == alsControllerId)
				{
					for (let alsFloatEvent of alsClipEnvelope?.Automation?.Events?.FloatEvent ?? [])
					{
						let beat = Math.max(parseFloat(alsFloatEvent?._attributes?.Time), 0);
						let value = parseInt(alsFloatEvent?._attributes?.Value);
						let channel = 0;

						switch (alsControllerNumber)
						{
							case 0:
							{
								track.createPitchWheelEvent(midiFile.getTickFromBeat(beat), channel, value);
								break;
							}
							case 1:
							{
								track.createChannelPressureEvent(midiFile.getTickFromBeat(beat), channel, value);
								break;
							}
							default:
							{
								let controllerNumber = alsControllerNumber - 2;
								track.createControlChangeEvent(midiFile.getTickFromBeat(beat), channel, controllerNumber, value);
								break;
							}
						}
					}

					break;
				}
			}
		}
	}

	return midiFile.saveToBuffer();
}

let filename = util.parseArgs({ allowPositionals: true }).positionals[0]
fs.writeFileSync(filename + ".mid", abletonLiveToSmf(fs.readFileSync(filename)));
