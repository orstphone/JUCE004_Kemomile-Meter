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

    presetButton.onClick = [this]() {ShowAndChoosePreset(); };
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

void KemomileMeterAudioProcessorEditor::handlePreset(int choice)
{
    //A function to handle the choice result of showSomeMenu()'s menu
    float i, s, a;
    i = *audioProcessor.targetIntegratedLoudness;
    s = *audioProcessor.targetMaximumShortTermLoudness;
    a = *audioProcessor.targetMaximumShortTermLoudness;

    switch (choice)
    {
        case 0:
            //user dismissed the menu w/o picking anything
            break;
        case 1: //EBU R128
            i = -23.0f, s = 0.0f; a = -1.0f;
            break;
        case 2: //EBU R128 S1 (advertising)
            i = -23.0f; s = -18.0f; a = -1.0f;
            break;
        case 3: //EBU R128 S2 (Streaming)
            i = -16.0f; s = 0.0f; a = -1.0f;
            break;
        case 4: //Sony
            i = -18.0f; s = 0.0f; a = 1.0f;
            break;
        case 5: //Sony(Portable)
            i = -18.0f; s = 0.0f; a = -1.0f;
            break;
        case 6: // Amazon Alexa
            i = -14.0f; s = 0.0f; a = -2.0f;
            break;
        case 7: //Amazon Music
            i = -14.0f; s = 0.0f; a = -2.0f;
            break;
        case 8: //Apple Music
            i = -16.0f; s = 0.0f; a = -1.0f;
            break;
        case 9: //Apple Podcasts
            i = -16.0f; s = 0.0f; a = -1.0f;
            break;
        case 10: //Deezer
            i = -15.0f; s = 0.0f; a = -1.0f;
            break;
        case 11: //Disney
            i = -27.0f; s = 0.0f; a = -2.0f;
            break;
        case 12: //HBO
            i = -27.0f; s = 0.0f; a = -2.0f;
            break;
        case 13: //Netflix
            i = -27.0f; s = 0.0f; a = -1.0f;
            break;
        case 14: //Spotify
            i = 14.0f; s = 0.0f; a = -1.0f;
            break;
        case 15: //Spotify (loud)
            i = -11.0f; s = 0.0f; a = -2.0f;
            break;
        case 16: //Starz
            i = -27.0f; s = 0.0f; a = -2.0f;
            break;
        case 17: //Tidal
            i = 0.0f; s = 0.0f; a = -1.0f;
            break;
        case 18: //Youtube
            i = -14.0f; s = 0.0f; a = -1.0f;
            break;
        case 19: //None
            i = 0.0f; s = 0.0f; a = 6.0f;
            break;
        case 20: //Custom //will spawn custom GUI sliders for calibration.
            resetButton.setEnabled(false);
            presetButton.setEnabled(false);
            extended = true;
            integratedLoudnessSlider.setBounds(180, 130, 540, 30);
            maximumShortTermLoudnessSlider.setBounds(180, 200, 540, 30);
            maximumTruePeakLevelSlider.setBounds(180, 270, 540, 30);
            setButton.setBounds(510, 360, 200, 42);
            break;
        default:
            //Empty
            break;
    }
}


void KemomileMeterAudioProcessorEditor::ShowAndChoosePreset()
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
    
    presets.showMenuAsync(
        juce::PopupMenu::Options(),
        [this](int selectedId) {KemomileMeterAudioProcessorEditor::handlePreset(selectedId); });
}

void KemomileMeterAudioProcessorEditor::hide()
{
    extended = false;
    resetButton.setEnabled(true);
    presetButton.setEnabled(true);
    integratedLoudnessSlider.setBounds(180, 10000, 540, 30);
    maximumShortTermLoudnessSlider.setBounds(180, 10000, 540, 30);
    maximumTruePeakLevelSlider.setBounds(180, 10000, 540, 30);
    setButton.setBounds(510, 10000, 200, 42);
}



KemomileMeterAudioProcessorEditor::~KemomileMeterAudioProcessorEditor()
{
    //empty
}



//=====================================prettiest unicorn=========================================
//=====================================prettiest unicorn=========================================
//=====================================prettiest unicorn=========================================

void KemomileMeterAudioProcessorEditor::drawBarGraph(
    juce::Graphics& g,
    int x,
    int y,
    int w,
    int minValue,
    int maxValue,
    float level,
    float target,
    juce::String valueLabel,
    juce::String warningLabel,
    bool over)
{
    g.setColour(level > -120 ? (over ? red : green) : grey);
    g.setFont(18);
    g.drawFittedText(valueLabel, x, y, 110, 30, juce::Justification::right, 1);

    g.setColour(grey);
    g.drawRect(x + 120, y + 14, w - 120, 1);

    if (target < maxValue)
    {
        int warning = (w - 120) + (w - 120) / (maxValue - minValue) * target - ((w - 120) / (maxValue - minValue) * maxValue); //normalizing
        g.drawRect(x + 120 + warning, y, 1, 29);
        g.setFont(12);
        g.drawFittedText(warningLabel, x + 120 + warning - 40, y - 30, 80, 30, juce::Justification::centred, 1);
    }

    int amplitude = (w - 120) + (w - 120) / (maxValue - minValue) * level - ((w - 120) / (maxValue - minValue) * maxValue);
    if (amplitude > 0)
    {
        g.setColour(over ? red : green);
        g.fillRect(x + 120, y + 7, amplitude > (w - 120) ? (w - 120) : amplitude, 15);
    }
}

//====================




void KemomileMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(18);
    g.drawFittedText("Loudness Meter Prototype", 60, 60, 400, 30, juce::Justification::left, 1);

    if (!extended) //is not resized
    {
        auto placeholder = juce::String::fromUTF8(u8"-\u221E");

        float targetIntegratedLoudness = *audioProcessor.targetIntegratedLoudness;
        juce::String integratedLoudnessText = juce::String::formatted(juce::String::formatted("%%.%df LUFS", _integratedLoudnessPrecision), audioProcessor.integratedLoudness);
        integratedLoudnessText = audioProcessor.integratedLoudness < -120 ? placeholder + " LUFS" : integratedLoudnessText;

        drawBarGraph(g, 30, 130, 680, -50, -0.0f, audioProcessor.integratedLoudness, targetIntegratedLoudness,
            integratedLoudnessText,
            juce::String::formatted("%.0f", targetIntegratedLoudness),
            juce::roundToInt(audioProcessor.integratedLoudness * pow(10, _integratedLoudnessPrecision)) > targetIntegratedLoudness * pow(10, _integratedLoudnessPrecision));
    
        float targetMaximumShortTermLoudness = *audioProcessor.targetMaximumShortTermLoudness;
        juce::String maximumShortTermLoudnessText = juce::String::formatted(juce::String::formatted("%%.%f LUFS", _maximumShortTermLoudnessPrecision), audioProcessor.maximumShortTermLoudness);
        maximumShortTermLoudnessText = audioProcessor.maximumShortTermLoudness < -120 ? placeholder + " LUFS" : maximumShortTermLoudnessText;
        drawBarGraph(g, 30, 200, 680, -50, -0.0f, audioProcessor.maximumShortTermLoudness, targetMaximumShortTermLoudness,
            maximumShortTermLoudnessText,
            juce::String::formatted("%.0f", targetMaximumShortTermLoudness),
            juce::roundToInt(audioProcessor.maximumShortTermLoudness * pow(10, _maximumShortTermLoudnessPrecision)) > targetMaximumShortTermLoudness * pow(10, _maximumShortTermLoudnessPrecision));

        float targetMaximumTruePeakLevel = *audioProcessor.targetMaximumTruePeakLevel;
        juce::String targetMaximumTruePeakLevelText = juce::String::formatted(juce::String::formatted("%%.%f dB", _maximumTruePeakLevelPrecision), audioProcessor.maximumTruePeakLevel);
        targetMaximumTruePeakLevelText = audioProcessor.maximumTruePeakLevel < -120 ? placeholder + " dB" : targetMaximumTruePeakLevelText;
        drawBarGraph(g, 30, 270, 680, -50, +6.0f, audioProcessor.maximumTruePeakLevel, targetMaximumTruePeakLevel,
            targetMaximumTruePeakLevelText, juce::String::formatted(juce::String::formatted("%%.%df dB", _maximumTruePeakLevelPrecision), targetMaximumTruePeakLevel),
            juce::roundToInt(audioProcessor.maximumTruePeakLevel * pow(10, _maximumTruePeakLevelPrecision)) > targetMaximumTruePeakLevel * pow(10, _maximumTruePeakLevelPrecision));
    }


    g.setFont(12);

    if (!extended)
    {
        g.setColour(yellow);
        g.fillRect(510, 360, 200, 42);
        g.setColour(black);
        g.drawFittedText("Reset", 510, 360, 200, 42, juce::Justification::centred, 10);

        g.setColour(grey);
        g.fillRect(300, 360, 200, 42);
        g.setColour(black);
        g.drawFittedText("Target", 300, 360, 200, 42, juce::Justification::centred, 1);
    }
    else
    {
        g.setColour(red);
        g.fillRect(510, 360, 200, 42);
        g.setColour(black);
        g.drawFittedText("Hide", 510, 360, 200, 42, juce::Justification::centred, 1);
        
        g.setFont(15);
        g.setColour(grey);
        g.drawFittedText("Integrated:", 650, 130, 120, 30, juce::Justification::left, 1);
        g.drawFittedText("Maximum Short Term:", 60, 200, 120, 30, juce::Justification::left, 1);
        g.drawFittedText("Maximum True Peak:", 60, 270, 120, 30, juce::Justification::left, 1);
    }

    g.setFont(12);
    g.setColour(grey);
    g.drawFittedText("_dev name goes here_", 60, getHeight() - 77, 400, 30, juce::Justification::left, 1);
}

void KemomileMeterAudioProcessorEditor::timerCallback()
{
    repaint();
}

void KemomileMeterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    resetButton.setAlpha(0.0f);
    presetButton.setAlpha(0.0f);
    setButton.setAlpha(0.0f);

    resetButton.setBounds(510, 360, 200, 42);
    presetButton.setBounds(300, 360, 200, 42);
    setButton.setBounds(510, extended ? 360 : 10000, 200, 42);

    integratedLoudnessSlider.setBounds(180, extended ? 130 : 10000, 540, 30);
    maximumShortTermLoudnessSlider.setBounds(180, extended ? 200 : 10000, 540, 30);
    maximumTruePeakLevelSlider.setBounds(180, extended ? 270 : 10000, 540, 30);
}
