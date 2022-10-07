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
class RoboFlurryAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    RoboFlurryAudioProcessor();
    ~RoboFlurryAudioProcessor() override;

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
    void outputNoteOn(juce::MidiBuffer& processedMidi, int samplePosition, int outputNote, int humanVelocity, int robotVelocity);
    void outputNoteOff(juce::MidiBuffer& processedMidi, int samplePosition, int outputNote, int humanNote, int robotNote);

    const int MODE_STRUM = 0;
    const int MODE_PLUCK = 1;
    const int MODE_BYPASS = 2;
    int mode = MODE_STRUM;
    int humanChannel = 1;
    int robotChannel = 2;
    int outputChannel = 1;
    float velocitySensitivity = 0.25;
    juce::SortedSet<int> humanNotes;
    juce::uint8 humanVelocities[128] = {0};
    juce::SortedSet<int> robotNotes;
    juce::uint8 robotVelocities[128] = {0};
    juce::SortedSet<int> outputNotes;
    juce::uint8 outputVelocities[128] = {0};
    int outputHumanSourceNotes[128] = {0};
    int outputRobotSourceNotes[128] = {0};

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoboFlurryAudioProcessor)
};
