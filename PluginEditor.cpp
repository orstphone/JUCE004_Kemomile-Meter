/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KemomileMeterAudioProcessorEditor::KemomileMeterAudioProcessorEditor (KemomileMeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    extended = false;

    setSize (400, 300);
    startTimerHz(24);

    addAndMakeVisible(numericLabel);
    numericLabel.setColour(juce::Label::textColourId, juce::Colours::azure);
    numericLabel.setFont(juce::Font("Times New Roman", 20.0f, juce::Font::plain));
    numericLabel.setSize(60, 30);
    numericLabel.setEditable(false);
    startTimer(100.f);

}

KemomileMeterAudioProcessorEditor::~KemomileMeterAudioProcessorEditor()
{
}

void KemomileMeterAudioProcessorEditor::timerCallback()
{
    numericLabel.setText(juce::String(audioProcessor.getLevelTruePeak()), juce::dontSendNotification);
}

//==============================================================================
void KemomileMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::darkgrey);

}

void KemomileMeterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    numericLabel.setBounds(100, 100, 200, 15);
}
