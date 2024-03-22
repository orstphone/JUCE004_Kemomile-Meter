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
#include "Components/HorizontalBarMeter.h"
#include "Components/FacePlateGui.h"
//==============================================================================
/**
*/
class KemomileMeterAudioProcessorEditor :
    public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    KemomileMeterAudioProcessorEditor (KemomileMeterAudioProcessor&);
    ~KemomileMeterAudioProcessorEditor() override;

    //==============================================================================
    int _integratedLoudnessPrecision = 0;
    int _maximumShortTermLoudnessPrecision = 0;
    int _maximumTruePeakLevelPrecision = 1;
    bool extended;

    void displayTextInfo(
        juce::Graphics& g,
        int x,
        int y,
        int w,
        int h,
        float level,
        float target,
        juce::String valueLabel,
        juce::String warningLabel,
        bool over);

    //color swatches
    const juce::Colour black = juce::Colour(0xff000000);
    const juce::Colour white = juce::Colour(0xffffffff);
    const juce::Colour green = juce::Colour(0xff29c732);
    const juce::Colour red = juce::Colour(0xfffc2125);
    const juce::Colour yellow = juce::Colour(0xfffec309);
    const juce::Colour grey = juce::Colour(0xff575757);
    //==================//==================//==================//==================

   void timerCallback() override; //will be called at the desired rate.
    void paint (juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;



private:
    const int guiUpdateRate = 24;
    const int vuMeterWindowSize = 300; //ms
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KemomileMeterAudioProcessor& audioProcessor;
    Gui::FacePlateGui facePlateGui;
    Gui::HorizontalBarMeter horizontalBarMeterL, horizontalBarMeterR;


    //
    //juce::TextButton resetButton{ "reset" };
    //juce::TextButton presetButton{ "Target" };
    //juce::TextButton setButton{ "Set" };

    //juce::Slider integratedLoudnessSlider;
    //juce::SliderParameterAttachment integratedLoudnessSliderAttachment;

    //juce::Slider maximumShortTermLoudnessSlider;
    //juce::SliderParameterAttachment maximumShortTermLoudnessSliderAttachment;

    //juce::Slider maximumTruePeakLevelSlider;
    //juce::SliderParameterAttachment maximumTruePeakLevelLoudnessSliderAttachment;





    void resetIntegratedLoudness();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KemomileMeterAudioProcessorEditor)
};
