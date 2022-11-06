/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RoboRumpusAudioProcessorEditor::RoboRumpusAudioProcessorEditor(RoboRumpusAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    strumButton.setText("Strum", juce::dontSendNotification);
    strumButton.setJustificationType(juce::Justification::centred);
    strumButton.setColour(juce::Label::textColourId, juce::Colour(0xffd8d8d8));
    strumButton.setColour(juce::Label::backgroundColourId, juce::Colour(0xff484848));
    strumButton.setBounds(8, 8, 80, 25);
    addAndMakeVisible(strumButton);

    pluckButton.setText("Pluck", juce::dontSendNotification);
    pluckButton.setJustificationType(juce::Justification::centred);
    pluckButton.setColour(juce::Label::textColourId, juce::Colour(0xffc0c0c0));
    pluckButton.setColour(juce::Label::backgroundColourId, juce::Colour(0xff303030));
    pluckButton.setBounds(96, 8, 80, 25);
    addAndMakeVisible(pluckButton);

    bypassButton.setText("Bypass", juce::dontSendNotification);
    bypassButton.setJustificationType(juce::Justification::centred);
    bypassButton.setColour(juce::Label::textColourId, juce::Colour(0xffc0c0c0));
    bypassButton.setColour(juce::Label::backgroundColourId, juce::Colour(0xff303030));
    bypassButton.setBounds(184, 8, 80, 25);
    addAndMakeVisible(bypassButton);

    holdButton.setText("Hold", juce::dontSendNotification);
    holdButton.setJustificationType(juce::Justification::centred);
    holdButton.setColour(juce::Label::textColourId, juce::Colour(0xffc0c0c0));
    holdButton.setColour(juce::Label::backgroundColourId, juce::Colour(0xff303030));
    holdButton.setBounds(272, 8, 80, 25);
    addAndMakeVisible(holdButton);

    settingsButton.setText("Settings", juce::dontSendNotification);
    settingsButton.setJustificationType(juce::Justification::centred);
    settingsButton.setColour(juce::Label::textColourId, juce::Colour(0xffc0c0c0));
    settingsButton.setColour(juce::Label::backgroundColourId, juce::Colour(0xff303030));
    settingsButton.setBounds(360, 8, 80, 25);
    addAndMakeVisible(settingsButton);

    humanLabel.setText("Human", juce::dontSendNotification);
    humanLabel.setJustificationType(juce::Justification::centred);
    humanLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff00ff));
    humanLabel.setBounds(40, 66, 80, 25);
    addAndMakeVisible(humanLabel);

    humanLights.setColour(juce::Label::textColourId, juce::Colour(0xffff00ff));
    humanLights.setColour(juce::Label::backgroundColourId, juce::Colour(0xff600060));
    humanLights.setBounds(128, 66, 272, 25);
    addAndMakeVisible(humanLights);

    robotLabel.setText("Robot", juce::dontSendNotification);
    robotLabel.setJustificationType(juce::Justification::centred);
    robotLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ffff));
    robotLabel.setBounds(40, 99, 80, 25);
    addAndMakeVisible(robotLabel);

    robotLights.setColour(juce::Label::textColourId, juce::Colour(0xff00ffff));
    robotLights.setColour(juce::Label::backgroundColourId, juce::Colour(0xff006060));
    robotLights.setBounds(128, 99, 272, 25);
    addAndMakeVisible(robotLights);

    outputLabel.setText("Output", juce::dontSendNotification);
    outputLabel.setJustificationType(juce::Justification::centred);
    outputLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc0c0c0));
    outputLabel.setBounds(40, 132, 80, 25);
    addAndMakeVisible(outputLabel);

    outputLights.setColour(juce::Label::textColourId, juce::Colours::white);
    outputLights.setColour(juce::Label::backgroundColourId, juce::Colour(0xff404040));
    outputLights.setBounds(128, 132, 272, 25);
    addAndMakeVisible(outputLights);

    setSize(448, 189);
}

RoboRumpusAudioProcessorEditor::~RoboRumpusAudioProcessorEditor()
{
}

//==============================================================================
void RoboRumpusAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff202020));
}

void RoboRumpusAudioProcessorEditor::resized()
{
}
