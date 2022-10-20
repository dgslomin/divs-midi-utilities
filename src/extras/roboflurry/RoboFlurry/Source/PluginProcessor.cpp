/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RoboFlurryAudioProcessor::RoboFlurryAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
	addParameter(mode = new juce::AudioParameterChoice("mode", "Mode", { "Strum", "Pluck", "Bypass" }, MODE_STRUM));
	addParameter(latch = new juce::AudioParameterBool("latch", "Latch", false));
	addParameter(humanChannel = new juce::AudioParameterInt("humanChannel", "Human channel", 1, 16, 1));
	addParameter(robotChannel = new juce::AudioParameterInt("robotChannel", "Robot channel", 1, 16, 2));
	addParameter(outputChannel = new juce::AudioParameterInt("outputChannel", "Output channel", 1, 16, 1));
	addParameter(velocitySensitivity = new juce::AudioParameterFloat("velocitySensitivity", "Velocity sensitivity", 0.0, 1.0, 0.25));
}

RoboFlurryAudioProcessor::~RoboFlurryAudioProcessor()
{
}

//==============================================================================
const juce::String RoboFlurryAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool RoboFlurryAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool RoboFlurryAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool RoboFlurryAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double RoboFlurryAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int RoboFlurryAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int RoboFlurryAudioProcessor::getCurrentProgram()
{
	return 0;
}

void RoboFlurryAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String RoboFlurryAudioProcessor::getProgramName(int index)
{
	return {};
}

void RoboFlurryAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void RoboFlurryAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	// Use this method as the place to do any pre-playback
	// initialisation that you need..
}

void RoboFlurryAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RoboFlurryAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void RoboFlurryAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	buffer.clear();
	juce::MidiBuffer processedMidi;

	for (auto metadata : midiMessages)
	{
		auto message = metadata.getMessage();
		auto samplePosition = metadata.samplePosition;
		auto channel = message.getChannel();

		if (message.isNoteOn() && (channel == humanChannel->get()))
		{
			auto humanNote = message.getNoteNumber();
			auto humanVelocity = message.getVelocity();
			humanNoteOn(processedMidi, samplePosition, humanNote, humanVelocity);
		}
		else if (message.isNoteOn() && (channel == robotChannel->get()))
		{
			auto robotNote = message.getNoteNumber();
			auto robotVelocity = message.getVelocity();
			robotNoteOn(processedMidi, samplePosition, robotNote, robotVelocity);
		}
		else if (message.isNoteOff() && (channel == humanChannel->get()))
		{
			auto humanNote = message.getNoteNumber();

			if (latch->get())
			{
				latched[humanNote] = true;
			}
			else
			{
				humanNoteOff(processedMidi, samplePosition, humanNote);
			}
		}
		else if (message.isNoteOff() && (channel == robotChannel->get()))
		{
			auto robotNote = message.getNoteNumber();
			robotNoteOff(processedMidi, samplePosition, robotNote);
		}
		else if (message.isSustainPedalOn() && (channel == humanChannel->get()))
		{
			latch->setValueNotifyingHost(true);
		}
		else if (message.isSustainPedalOff() && (channel == humanChannel->get()))
		{
			if (latch->get())
			{
				auto humanNotesIterator = juce::SortedSet<int>(humanNotes);

				for (auto humanNote : humanNotesIterator)
				{
					if (latched[humanNote])
					{
						humanNoteOff(processedMidi, samplePosition, humanNote);
						latched[humanNote] = false;
					}
				}

				latch->setValueNotifyingHost(false);
			}
		}
		else
		{
			auto outputMessage = juce::MidiMessage(message);
			outputMessage.setChannel(outputChannel->get());
			processedMidi.addEvent(outputMessage, samplePosition);
		}
	}

	midiMessages.swapWith(processedMidi);
}

void RoboFlurryAudioProcessor::humanNoteOn(juce::MidiBuffer& processedMidi, int samplePosition, int humanNote, int humanVelocity)
{
	humanNotes.add(humanNote);
	humanVelocities[humanNote] = humanVelocity;

	if (mode->getIndex() == MODE_STRUM)
	{
		for (auto robotNote : robotNotes)
		{
			auto robotVelocity = robotVelocities[robotNote];
			auto outputNote = humanNote + robotNote - 60;
			auto outputVelocity = combineVelocities(humanVelocity, robotVelocity);
			outputNoteOn(processedMidi, samplePosition, outputNote, outputVelocity, humanNote, robotNote);
		}
	}
	else if (mode->getIndex() == MODE_PLUCK)
	{
		// TODO
#if 0
		for (auto robotNote : robotNotes)
		{
			auto robotVelocity = robotVelocities[robotNote];
			auto robotNoteNumber = robotNote % 12;
			auto robotOctave = (robotNote / 12) - 5;
			auto numberOfHumanNotes = humanNotes.size();
			auto humanNoteNumber = robotNoteNumber * numberOfHumanNotes / 12;
			auto humanNote = humanNotes[humanNoteNumber];
			auto humanVelocity = humanVelocities[humanNote];
			auto outputNote = humanNote + (robotOctave * 12);
			auto outputVelocity = combineVelocities(humanVelocity, robotVelocity);
			outputNoteOn(processedMidi, samplePosition, outputNote, outputVelocity, humanNote, robotNote);
		}
#endif
	}
	else if (mode->getIndex() == MODE_BYPASS)
	{
		auto outputNote = humanNote;
		auto outputVelocity = humanVelocity;
		auto robotNote = -1; // won't match any real ones
		outputNoteOn(processedMidi, samplePosition, outputNote, outputVelocity, humanNote, robotNote);
	}
}

void RoboFlurryAudioProcessor::robotNoteOn(juce::MidiBuffer& processedMidi, int samplePosition, int robotNote, int robotVelocity)
{
	robotNotes.add(robotNote);
	robotVelocities[robotNote] = robotVelocity;

	if (mode->getIndex() == MODE_STRUM)
	{
		for (auto humanNote : humanNotes)
		{
			auto humanVelocity = humanVelocities[humanNote];
			auto outputNote = humanNote + robotNote - 60;
			auto outputVelocity = combineVelocities(humanVelocity, robotVelocity);
			outputNoteOn(processedMidi, samplePosition, outputNote, outputVelocity, humanNote, robotNote);
		}
	}
	else if (mode->getIndex() == MODE_PLUCK)
	{
		auto robotNoteNumber = robotNote % 12;
		auto robotOctave = (robotNote / 12) - 5;
		auto numberOfHumanNotes = humanNotes.size();
		auto humanNoteNumber = robotNoteNumber * numberOfHumanNotes / 12;
		auto humanNote = humanNotes[humanNoteNumber];
		auto humanVelocity = humanVelocities[humanNote];
		auto outputNote = humanNote + (robotOctave * 12);
		auto outputVelocity = combineVelocities(humanVelocity, robotVelocity);
		outputNoteOn(processedMidi, samplePosition, outputNote, outputVelocity, humanNote, robotNote);
	}
}

void RoboFlurryAudioProcessor::humanNoteOff(juce::MidiBuffer& processedMidi, int samplePosition, int humanNote)
{
	humanNotes.removeValue(humanNote);
	humanVelocities[humanNote] = 0;
	auto outputNotesIterator = juce::SortedSet<int>(outputNotes);

	for (auto outputNote : outputNotesIterator)
	{
		if (outputHumanSourceNotes[outputNote] == humanNote)
		{
			outputNoteOff(processedMidi, samplePosition, outputNote);
		}
	}
}

void RoboFlurryAudioProcessor::robotNoteOff(juce::MidiBuffer& processedMidi, int samplePosition, int robotNote)
{
	robotNotes.removeValue(robotNote);
	robotVelocities[robotNote] = 0;
	auto outputNotesIterator = juce::SortedSet<int>(outputNotes);

	for (auto outputNote : outputNotesIterator)
	{
		if (outputRobotSourceNotes[outputNote] == robotNote)
		{
			outputNoteOff(processedMidi, samplePosition, outputNote);
		}
	}
}

int RoboFlurryAudioProcessor::combineVelocities(int humanVelocity, int robotVelocity)
{
	return ((velocitySensitivity->get() * humanVelocity / 128) + (1 - velocitySensitivity->get())) * robotVelocity;
}

void RoboFlurryAudioProcessor::outputNoteOn(juce::MidiBuffer& processedMidi, int samplePosition, int outputNote, int outputVelocity, int humanNote, int robotNote)
{
	if ((outputNote < 0) || (outputNote > 127) || (outputVelocity < outputVelocities[outputNote])) return;

	if (outputVelocities[outputNote])
	{
		processedMidi.addEvent(juce::MidiMessage::noteOff(outputChannel->get(), outputNote), samplePosition);
	}
	else
	{
		outputNotes.add(outputNote);
	}

	outputVelocities[outputNote] = outputVelocity;
	outputHumanSourceNotes[outputNote] = humanNote;
	outputRobotSourceNotes[outputNote] = robotNote;
	processedMidi.addEvent(juce::MidiMessage::noteOn(outputChannel->get(), outputNote, (juce::uint8)outputVelocity), samplePosition);
}

void RoboFlurryAudioProcessor::outputNoteOff(juce::MidiBuffer& processedMidi, int samplePosition, int outputNote)
{
	outputNotes.removeValue(outputNote);
	outputVelocities[outputNote] = 0;
	outputHumanSourceNotes[outputNote] = 0;
	outputRobotSourceNotes[outputNote] = 0;
	processedMidi.addEvent(juce::MidiMessage::noteOff(outputChannel->get(), outputNote), samplePosition);
}

//==============================================================================
bool RoboFlurryAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RoboFlurryAudioProcessor::createEditor()
{
	return new RoboFlurryAudioProcessorEditor(*this);
}

//==============================================================================
void RoboFlurryAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	juce::ValueTree state { "Parameters", {
		{ "mode", mode->getIndex() },
		{ "latch", latch->get() },
		{ "humanChannel", humanChannel->get() },
		{ "robotChannel", robotChannel->get() },
		{ "outputChannel", outputChannel->get() },
		{ "velocitySensitivity", velocitySensitivity->get() },
	}, {} };

	state.writeToStream(juce::MemoryOutputStream(destData, false));
}

void RoboFlurryAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	juce::ValueTree state = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));
	mode->setValueNotifyingHost(state.getProperty("mode", 0));
	latch->setValueNotifyingHost(state.getProperty("latch", false));
	humanChannel->setValueNotifyingHost(state.getProperty("humanChannel", 1));
	robotChannel->setValueNotifyingHost(state.getProperty("robotChannel", 2));
	outputChannel->setValueNotifyingHost(state.getProperty("outputChannel", 1));
	velocitySensitivity->setValueNotifyingHost(state.getProperty("velocitySensitivity", 0.25));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new RoboFlurryAudioProcessor();
}
