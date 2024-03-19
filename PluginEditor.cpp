/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KemomileMeterAudioProcessorEditor::KemomileMeterAudioProcessorEditor(KemomileMeterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
  /*  integratedLoudnessSliderAttachment(*p.targetIntegratedLoudness, integratedLoudnessSlider),
    maximumShortTermLoudnessSliderAttachment(*p.targetMaximumShortTermLoudness, maximumShortTermLoudnessSlider),
    maximumTruePeakLevelLoudnessSliderAttachment(*p.targetMaximumTruePeakLevel, maximumTruePeakLevelSlider)*/

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    extended = false;
    //setSize (800, 460);
    setSize(1012, 230);
    //load image assets
    facePlateGui.loadAssets();


    addAndMakeVisible(horizontalBarMeterL);
    addAndMakeVisible(horizontalBarMeterR);




    //resetButton.onClick = [this]() {resetIntegratedLoudness(); };
    //addAndMakeVisible(resetButton);

    //presetButton.onClick = [this]() {ShowAndChoosePreset(); };
    //addAndMakeVisible(presetButton);

    //setButton.onClick = [this]() {hide(); };
    //addAndMakeVisible(setButton);


    //startTimer(100);
    //addAndMakeVisible(integratedLoudnessSlider);
    //integratedLoudnessSlider.setTextValueSuffix(" LUFS");
    //integratedLoudnessSlider.setColour(juce::Slider::ColourIds::backgroundColourId, grey);
    //integratedLoudnessSlider.setColour(juce::Slider::ColourIds::thumbColourId, red);
    //integratedLoudnessSlider.setColour(juce::Slider::ColourIds::trackColourId, grey);
    //integratedLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, grey);
    //integratedLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, black);
    //integratedLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxHighlightColourId, red);
    //integratedLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, black);

    //addAndMakeVisible(maximumShortTermLoudnessSlider);
    //maximumShortTermLoudnessSlider.setTextValueSuffix(" LUFS");
    //maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::backgroundColourId, grey);
    //maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::thumbColourId, red);
    //maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::trackColourId, grey);
    //maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, grey);
    //maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, black);
    //maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxHighlightColourId, red);
    //maximumShortTermLoudnessSlider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, black);

    //addAndMakeVisible(maximumTruePeakLevelSlider);
    //maximumTruePeakLevelSlider.setTextValueSuffix(" dB");
    //maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::backgroundColourId, grey);
    //maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::thumbColourId, red);
    //maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::trackColourId, grey);
    //maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, grey);
    //maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, black);
    //maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::textBoxHighlightColourId, red);
    //maximumTruePeakLevelSlider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, black);


    startTimerHz(24);
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
    /*        resetButton.setEnabled(false);
            presetButton.setEnabled(false);
            extended = true;
            integratedLoudnessSlider.setBounds(180, 130, 540, 30);
            maximumShortTermLoudnessSlider.setBounds(180, 200, 540, 30);
            maximumTruePeakLevelSlider.setBounds(180, 270, 540, 30);
            setButton.setBounds(510, 360, 200, 42);*/
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
    //resetButton.setEnabled(true);
    //presetButton.setEnabled(true);
    //integratedLoudnessSlider.setBounds(180, 10000, 540, 30);
    //maximumShortTermLoudnessSlider.setBounds(180, 10000, 540, 30);
    //maximumTruePeakLevelSlider.setBounds(180, 10000, 540, 30);
    //setButton.setBounds(510, 10000, 200, 42);
}



KemomileMeterAudioProcessorEditor::~KemomileMeterAudioProcessorEditor()
{
    //empty
}



//=====================================prettiest unicorn=========================================
//=====================================prettiest unicorn=========================================
//=====================================prettiest unicorn=========================================

void KemomileMeterAudioProcessorEditor::displayTextInfo(
    juce::Graphics& g,
    int x,
    int y,
    int w,
    int h,
    float level,
    float target,
    juce::String valueLabel,
    juce::String warningLabel,
    bool over)
{
    auto colorLevelDependent = level > -120 ? (over ? juce::Colours::orange : juce::Colours::goldenrod) : juce::Colours::aliceblue;
    g.setColour(colorLevelDependent);
    g.setFont(18);
    g.drawFittedText(valueLabel, x, y, w, h, juce::Justification::right, 1);
}

//====================




void KemomileMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)

    g.fillAll(juce::Colours::green);

    g.drawImage(facePlateGui.getFaceplate(), 0, 0, getWidth(), getHeight(), 0, 0, facePlateGui.getFaceplate().getWidth(), facePlateGui.getFaceplate().getHeight());


    // print ChannelOutput (LRMS) related GUIs
    g.drawImage(facePlateGui.getChannelInfos(), 595, 84, 6, 61, 0, 0, facePlateGui.getChannelInfos().getWidth(), facePlateGui.getChannelInfos().getHeight());
    // print metric related GUIs
    g.drawImage(facePlateGui.getMetricsVu(), 114, 105, 468, 21, 0, 0, facePlateGui.getMetricsVu().getWidth(), facePlateGui.getMetricsVu().getHeight());
    g.drawImage(facePlateGui.getIconVu(), 590, 106, 16, 19, 0, 0, facePlateGui.getIconVu().getWidth(), facePlateGui.getIconVu().getHeight());



    //draw texts
    juce::Font theGotoFont("Agency FB", 14, juce::Font::italic);
    juce::Colour theGotoColor = juce::Colours::aqua;
    g.setColour(theGotoColor);
    g.setFont(theGotoFont);
    juce::String samplerateInfoString = juce::String(juce::String(audioProcessor.getSampleRate()) + " Hz");
    g.drawFittedText(samplerateInfoString, 657, 174, 150, 23, juce::Justification::centredRight, 1);





    if (!extended) //is not resized
    {
        auto placeholder = juce::String::fromUTF8(u8"-\u221E");

        float targetIntegratedLoudness = *audioProcessor.targetIntegratedLoudness;
        juce::String integratedLoudnessText = juce::String::formatted(juce::String::formatted("%%.%df LUFS", _integratedLoudnessPrecision), audioProcessor.integratedLoudness);
        integratedLoudnessText = audioProcessor.integratedLoudness < -120 ? placeholder + " LUFS" : integratedLoudnessText;

      /*  displayTextInfo(g, 30, 130, 680, -50, -0.0f, audioProcessor.integratedLoudness, targetIntegratedLoudness,
            integratedLoudnessText,
            juce::String::formatted("%.0f", targetIntegratedLoudness),
            juce::roundToInt(audioProcessor.integratedLoudness * pow(10, _integratedLoudnessPrecision)) > targetIntegratedLoudness * pow(10, _integratedLoudnessPrecision));
    
        float targetMaximumShortTermLoudness = *audioProcessor.targetMaximumShortTermLoudness;
        juce::String maximumShortTermLoudnessText = juce::String::formatted(juce::String::formatted("%%.%f LUFS", _maximumShortTermLoudnessPrecision), audioProcessor.maximumShortTermLoudness);
        maximumShortTermLoudnessText = audioProcessor.maximumShortTermLoudness < -120 ? placeholder + " LUFS" : maximumShortTermLoudnessText;
        displayTextInfo(g, 30, 200, 680, -50, -0.0f, audioProcessor.maximumShortTermLoudness, targetMaximumShortTermLoudness,
            maximumShortTermLoudnessText,
            juce::String::formatted("%.0f", targetMaximumShortTermLoudness),
            juce::roundToInt(audioProcessor.maximumShortTermLoudness * pow(10, _maximumShortTermLoudnessPrecision)) > targetMaximumShortTermLoudness * pow(10, _maximumShortTermLoudnessPrecision));

        float targetMaximumTruePeakLevel = *audioProcessor.targetMaximumTruePeakLevel;
        juce::String targetMaximumTruePeakLevelText = juce::String::formatted(juce::String::formatted("%%.%f dB", _maximumTruePeakLevelPrecision), audioProcessor.maximumTruePeakLevel);
        targetMaximumTruePeakLevelText = audioProcessor.maximumTruePeakLevel < -120 ? placeholder + " dB" : targetMaximumTruePeakLevelText;
        displayTextInfo(g, 30, 270, 680, -50, +6.0f, audioProcessor.maximumTruePeakLevel, targetMaximumTruePeakLevel,
            targetMaximumTruePeakLevelText, juce::String::formatted(juce::String::formatted("%%.%df dB", _maximumTruePeakLevelPrecision), targetMaximumTruePeakLevel),
            juce::roundToInt(audioProcessor.maximumTruePeakLevel * pow(10, _maximumTruePeakLevelPrecision)) > targetMaximumTruePeakLevel * pow(10, _maximumTruePeakLevelPrecision));*/
    }


    g.setFont(12);

    if (!extended)
    {
        g.setColour(yellow);
        g.fillRect(510, 360, 200, 42);
        g.setColour(juce::Colours::azure);
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

    g.setFont(theGotoFont);
    g.setColour(theGotoColor);
    g.drawFittedText("@deccauvw 2024", 128,174, 400, 30, juce::Justification::left, 1);
}

void KemomileMeterAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
}

void KemomileMeterAudioProcessorEditor::timerCallback()
{
    horizontalBarMeterL.setLevel(audioProcessor.peakLevel);
    horizontalBarMeterR.setLevel(audioProcessor.peakLevel);

    horizontalBarMeterL.repaint();
    horizontalBarMeterR.repaint();
    //repaint();
}

void KemomileMeterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    horizontalBarMeterL.setBounds(124, 87, 455, 8);
    horizontalBarMeterR.setBounds(124, 135, 455, 8);


    //resetButton.setAlpha(0.0f);
    //presetButton.setAlpha(0.0f);
    //setButton.setAlpha(0.0f);

    //resetButton.setBounds(510, 360, 200, 42);
    //presetButton.setBounds(300, 360, 200, 42);
    //setButton.setBounds(510, extended ? 360 : 10000, 200, 42);

    //integratedLoudnessSlider.setBounds(180, extended ? 130 : 10000, 540, 30);
    //maximumShortTermLoudnessSlider.setBounds(180, extended ? 200 : 10000, 540, 30);
    //maximumTruePeakLevelSlider.setBounds(180, extended ? 270 : 10000, 540, 30);
}
