/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KemomileMeterAudioProcessorEditor::KemomileMeterAudioProcessorEditor(KemomileMeterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    integratedLoudnessSliderAttachment(*p.targetIntegratedLoudness, integratedLoudnessSlider),
    maximumShortTermLoudnessSliderAttachment(*p.targetMaximumShortTermLoudness, maximumShortTermLoudnessSlider),
    maximumTruePeakLevelLoudnessSliderAttachment(*p.targetMaximumTruePeakLevel, maximumTruePeakLevelSlider)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    extended = false;

    setSize (800, 460);
    
    resetButton.onClick = [this]() {resetIntegratedLoudness(); };
    addAndMakeVisible(resetButton);

    presetButton.onClick = [this]() {choosePreset(); };
    addAndMakeVisible(presetButton);

    setButton.onClick = [this]() {hide(); };
    addAndMakeVisible(setButton);

    startTimer(100);
    addAndMakeVisible(integratedLoudnessSlider);
    integratedLoudnessSlider.setTextValueSuffix(" LUFS");
    integratedLoudnessSlider.setColour(juce::Slider::ColourIds::backgroundColourId, grey);
    integratedLoudnessSlider.setColour(juce::Slider::ColourIds::thumbColourId, red);
    integratedLoudnessSlider.setColour(juce::Slider::ColourIds::trackColourId, grey);
    integratedLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, grey);
    integratedLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, black);
    integratedLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxHighlightColourId, red);
    integratedLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, black);

    addAndMakeVisible(maximumShortTermLoudnessSlider);
    maximumShortTermLoudnessSlider.setTextValueSuffix(" LUFS");
    maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::backgroundColourId, grey);
    maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::thumbColourId, red);
    maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::trackColourId, grey);
    maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, grey);
    maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, black);
    maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxHighlightColourId, red);
    maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, black);

    addAndMakeVisible(maximumTruePeakLevelSlider);
    maximumTruePeakLevelSlider.setTextValueSuffix(" dB");
    maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::backgroundColourId, grey);
    maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::thumbColourId, red);
    maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::trackColourId, grey);
    maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, grey);
    maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, black);
    maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::textBoxHighlightColourId, red);
    maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, black);

}


void KemomileMeterAudioProcessorEditor::resetIntegratedLoudness()
{
    audioProcessor.resetIntegratedLoudness();
}

void KemomileMeterAudioProcessorEditor::choosePreset()
{
    juce::PopupMenu presets;
    presets.addItem(1, "EBU R128");
    presets.addItem(2, "EBU R128 S1 (Advertising)");
    presets.addItem(3, "EBU R128 S2 (Streaming)");
    presets.addItem(4, "Sony");
    presets.addItem(5, "Sony (Portable)");
    presets.addItem(6, "Amazon Alexa");
    presets.addItem(7, "Amazon Music");
    presets.addItem(8, "Apple Music");
    presets.addItem(9, "Apple Podcasts");
    presets.addItem(10, "Deezer");
    presets.addItem(11, "Disney");
    presets.addItem(12, "HBO");
    presets.addItem(13, "Netflix");
    presets.addItem(14, "Spotify");
    presets.addItem(15, "Spotify (Loud)");
    presets.addItem(16, "Starz");
    presets.addItem(17, "Tidal");
    presets.addItem(18, "YouTube");
    presets.addItem(19, "None");
    presets.addItem(20, "Custom");

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
