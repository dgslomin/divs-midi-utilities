/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class RoboRumpusAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RoboRumpusAudioProcessorEditor (RoboRumpusAudioProcessor&);
    ~RoboRumpusAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    juce::Label strumButton;
    juce::Label pluckButton;
    juce::Label bypassButton;
    juce::Label holdButton;
    juce::Label settingsButton;
    juce::Label humanLabel;
    juce::Label humanLights;
    juce::Label robotLabel;
    juce::Label robotLights;
    juce::Label outputLabel;
    juce::Label outputLights;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RoboRumpusAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoboRumpusAudioProcessorEditor)
};
