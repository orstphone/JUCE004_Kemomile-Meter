/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    reference materials
    https://www.itu.int/dms_pubrec/itu-r/rec/bs/R-REC-BS.1770-4-201510-I!!PDF-E.pdf
    https://github.com/unusual-audio/loudness-meter

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
//==============================================================================
/**
*/
class KemomileMeterAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    KemomileMeterAudioProcessorEditor (KemomileMeterAudioProcessor&);
    ~KemomileMeterAudioProcessorEditor() override;

    //==============================================================================
    int integratedLoudnessPrecision = 0;
    int maximumShortTermLoudnessPrecision = 0;
    int maximumTruePeakLevelPrecision = 1;
    bool extended;


    //==================

    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KemomileMeterAudioProcessor& audioProcessor;

    juce::Label numericLabel{ "numericLabel", "--" };


    //void resetIntegratedLoudness();
    //void choosePreset();
    //void hide();


    //Gui::
    //Gui::

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KemomileMeterAudioProcessorEditor)
};
