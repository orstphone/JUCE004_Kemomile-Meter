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
//==============================================================================
/**
*/
class KemomileMeterAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KemomileMeterAudioProcessor& audioProcessor;

    //imageComponents for GUI displays
    juce::ImageComponent imageComponentImageFacePlateBody;
    juce::ImageComponent imageComponentImageFaceDisplayBody;
    juce::ImageComponent imageComponentImageFaceGuardRail;
    juce::ImageComponent imageComponentImageHorizontalStripDisplay;
    juce::ImageComponent imageComponentImageGlassSheenReflection;
    juce::ImageComponent imageComponentImageNumericPrintedValuesVu;



    //
    juce::TextButton resetButton{ "reset" };
    juce::TextButton presetButton{ "Target" };
    juce::TextButton setButton{ "Set" };

    juce::Slider integratedLoudnessSlider;
    juce::SliderParameterAttachment integratedLoudnessSliderAttachment;

    juce::Slider maximumShortTermLoudnessSlider;
    juce::SliderParameterAttachment maximumShortTermLoudnessSliderAttachment;

    juce::Slider maximumTruePeakLevelSlider;
    juce::SliderParameterAttachment maximumTruePeakLevelLoudnessSliderAttachment;

    Gui::HorizontalBarMeter horizontalBarMeterL, horizontalBarMeterR;



    void resetIntegratedLoudness();
    void handlePreset(int choice);
    void ShowAndChoosePreset();
    void hide();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KemomileMeterAudioProcessorEditor)
};
