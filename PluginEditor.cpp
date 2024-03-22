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





    startTimerHz(24);
}


void KemomileMeterAudioProcessorEditor::resetIntegratedLoudness()
{
    audioProcessor.resetIntegratedLoudness();
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
    startTimerHz(guiUpdateRate);
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
    auto vuLevelLeft = audioProcessor.getMonoVuLevels(0);
    auto vuLevelRight = audioProcessor.getMonoVuLevels(1);
    float levelPeak = audioProcessor.levelPeak;

    //try averaging the array


    horizontalBarMeterL.setLevel(juce::Decibels::gainToDecibels(vuLevelLeft));
    horizontalBarMeterR.setLevel(juce::Decibels::gainToDecibels(vuLevelRight));

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
