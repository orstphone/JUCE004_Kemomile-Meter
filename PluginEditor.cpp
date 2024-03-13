/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KemomileMeterAudioProcessorEditor::KemomileMeterAudioProcessorEditor (KemomileMeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    extended = false;

    setSize (1200, 600);


    startTimerHz(24);
    startTimer(100.f);


}

KemomileMeterAudioProcessorEditor::~KemomileMeterAudioProcessorEditor()
{
}

void KemomileMeterAudioProcessorEditor::timerCallback()
{
    //empty atm before you introduce meters
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
}
