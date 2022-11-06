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
class RoboFlurryAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RoboFlurryAudioProcessorEditor (RoboFlurryAudioProcessor&);
    ~RoboFlurryAudioProcessorEditor() override;

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
    RoboFlurryAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoboFlurryAudioProcessorEditor)
};
