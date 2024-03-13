/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

    reference materials
    https://www.itu.int/dms_pubrec/itu-r/rec/bs/R-REC-BS.1770-4-201510-I!!PDF-E.pdf
    https://github.com/unusual-audio/loudness-meter

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum MeterType
{
    Peak,
    RMS,
    VU,
    LU
};

struct MeterSettings
{
    float inputTrim{ 0.0f }, outputTrim{ 0.0f };
    float targetLevel{ 0.0f }, referenceLevel{ 0.0f };
    float autoCalibrate{ 0.0f };
    MeterType meterType{ MeterType::Peak };
};

MeterSettings getMeterSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class KemomileMeterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    KemomileMeterAudioProcessor();
    ~KemomileMeterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };
    //==============================================================================
    float getLevelPeak(const int channel) const;
    //float getLevelRms(const int channel) const;
    //float getLevelVu(const int channel) const;
    //float getLevelLu(const int CHannel) const;

private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    using Gain = juce::dsp::Gain<float>;
    using MonoChain = juce::dsp::ProcessorChain<Gain, Gain, Gain, Gain>; //In, Out, Ref, Target

    MonoChain chainLeft, chainRight;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KemomileMeterAudioProcessor)
};
