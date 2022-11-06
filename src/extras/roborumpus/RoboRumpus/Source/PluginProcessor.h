/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class RoboRumpusAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    RoboRumpusAudioProcessor();
    ~RoboRumpusAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void humanNoteOn(juce::MidiBuffer& processedMidi, int samplePosition, int humanNote, int humanVelocity);
    void robotNoteOn(juce::MidiBuffer& processedMidi, int samplePosition, int robotNote, int robotVelocity);
    void humanNoteOff(juce::MidiBuffer& processedMidi, int samplePosition, int humanNote);
    void robotNoteOff(juce::MidiBuffer& processedMidi, int samplePosition, int robotNote);
    int combineVelocities(int humanVelocity, int robotVelocity);
    void outputNoteOn(juce::MidiBuffer& processedMidi, int samplePosition, int outputNote, int outputVelocity, int humanNote, int robotNote);
    void outputNoteOff(juce::MidiBuffer& processedMidi, int samplePosition, int outputNote);

    const int MODE_STRUM = 0;
    const int MODE_PLUCK = 1;
    const int MODE_BYPASS = 2;

    juce::AudioParameterChoice* mode;
    juce::AudioParameterBool* hold;
    juce::AudioParameterInt* humanChannel;
    juce::AudioParameterInt* robotChannel;
    juce::AudioParameterInt* outputChannel;
    juce::AudioParameterFloat* velocitySensitivity;

    juce::SortedSet<int> humanNotes;
    int humanVelocities[128] = { 0 };
    bool held[128] = { false };
    juce::SortedSet<int> robotNotes;
    int robotVelocities[128] = { 0 };
    juce::SortedSet<int> outputNotes;
    int outputVelocities[128] = { 0 };
    int outputHumanSourceNotes[128] = { 0 };
    int outputRobotSourceNotes[128] = { 0 };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoboRumpusAudioProcessor)
};
