/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h" 
//==============================================================================
KemomileMeterAudioProcessor::KemomileMeterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(targetIntegratedLoudness = new juce::AudioParameterFloat({ "targetIntegratedLoudenss", 1 }, "Integrated", juce::NormalisableRange<float>(-50.0f, 0.0f, 1.0f), -23.0f));
    addParameter(targetMaximumShortTermLoudness = new juce::AudioParameterFloat({ "targetMaximumShortTermLoudness", 1 }, "Maximum Short Term", juce::NormalisableRange<float>(-50.0f, 0.0f, 1.0f), -18.0f));
    addParameter(targetMaximumTruePeakLevel = new juce::AudioParameterFloat({ "targetMaximumTruePeakLevel", 1 }, "Maximum True Peak", juce::NormalisableRange<float>(-50.0f, 6.0f, 1.0f), -1.0f));

}

KemomileMeterAudioProcessor::~KemomileMeterAudioProcessor()
{
}

//==============================================================================
const juce::String KemomileMeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KemomileMeterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KemomileMeterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KemomileMeterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KemomileMeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KemomileMeterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KemomileMeterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KemomileMeterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KemomileMeterAudioProcessor::getProgramName (int index)
{
    return {};
}

void KemomileMeterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KemomileMeterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    interpolatedBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock * oversampling);

    /*
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    chainLeft.prepare(spec);
    chainRight.prepare(spec);
    */
}

void KemomileMeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


#ifndef JucePlugin_PreferredChannelConfigurations
bool KemomileMeterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void KemomileMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //star from here        >>>>>>>>>>>>>
    
    loudnessMeterProcessor.processBlock(buffer);
    measurementPaused = loudnessMeterProcessor.getMomentaryLoudness() < -70;
    if (!measurementPaused)
    {
        integratedLoudness = loudnessMeterProcessor.getIntegratedLoudness();
    }
    shortTermLoudness = loudnessMeterProcessor.getShortTermLoudness();
    momentaryLoudness = loudnessMeterProcessor.getMomentaryLoudness();
    maximumShortTermLoudness = loudnessMeterProcessor.getMaximumShortTermLoudness();
    maximumMomentaryLoudness = loudnessMeterProcessor.getMaximumMomentaryLoudness();
    loudnessRange = loudnessMeterProcessor.getLoudnessRange();
    peakLevel = juce::Decibels::gainToDecibels(buffer.getMagnitude(0, buffer.getNumSamples()), -INFINITY);
    maximumPeakLevel = peakLevel > maximumPeakLevel ? peakLevel : maximumPeakLevel;
    truePeakLevel = juce::Decibels::gainToDecibels(truePeakProcessor.process(buffer).getMax(), -INFINITY);
    maximumTruePeakLevel = truePeakLevel > maximumTruePeakLevel ? truePeakLevel : maximumTruePeakLevel;


}

void KemomileMeterAudioProcessor::resetIntegratedLoudness()
{
    loudnessMeterProcessor.reset();
    integratedLoudness = -INFINITY;
    maximumShortTermLoudness = -INFINITY;
    maximumMomentaryLoudness = -INFINITY;
    peakLevel = -INFINITY;
    maximumPeakLevel = -INFINITY;
    maximumTruePeakLevel = -INFINITY;
    truePeakLevel = -INFINITY;
}

//==============================================================================
bool KemomileMeterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KemomileMeterAudioProcessor::createEditor()
{
    return new KemomileMeterAudioProcessorEditor (*this);
    //generic GUI
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void KemomileMeterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("LoudnessMeterParams"));
    xml->setAttribute("targetIntegratedLoudness", (double)*targetIntegratedLoudness);
    xml->setAttribute("targetMaximumShortTermLoudness", (double)*targetMaximumShortTermLoudness);
    xml->setAttribute("targetMaximumTruePeakLevel,", (double)*targetMaximumTruePeakLevel);
    copyXmlToBinary(*xml, destData);
}

void KemomileMeterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("LoudnessMeterparams"))
        {
            *targetIntegratedLoudness = (float)xmlState->getDoubleAttribute("targetIntegratedLoudness", *targetIntegratedLoudness);
            *targetMaximumShortTermLoudness = (float)xmlState->getDoubleAttribute("targetMaximumShortTermLoudness", *targetMaximumShortTermLoudness);
            *targetMaximumTruePeakLevel = (float)xmlState->getDoubleAttribute("targetmaximumTruePeakLevel", *targetMaximumTruePeakLevel);
        }
    }

}


/*
//for apvts      >>>>>>>>>>>>>>>>>
MeterSettings getMeterSettings(juce::AudioProcessorValueTreeState& apvts)
{
    MeterSettings settings;
    settings.autoCalibrate = apvts.getRawParameterValue("AutoCalibrate")->load();
    settings.inputTrim = apvts.getRawParameterValue("InputTrim")->load();
    settings.outputTrim = apvts.getRawParameterValue("OutputTrim")->load();
    settings.targetLevel = apvts.getRawParameterValue("TargetLevel")->load();
    settings.referenceLevel = apvts.getRawParameterValue("ReferenceLevel")->load();
    settings.meterType = static_cast<MeterType>(apvts.getRawParameterValue("MeterType")->load());
    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout KemomileMeterAudioProcessor::createParameterLayout()
{
    //these are for controllable GUIs
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique <juce::AudioParameterFloat>("InputTrim", "Input Trim", juce::NormalisableRange<float>(-20.f, +20.f, 1.f, 1.f), 0.0f));
    layout.add(std::make_unique <juce::AudioParameterFloat> ("OutputTrim", "Output Trim", juce::NormalisableRange<float>(-20.f, +20.f, 1.f, 1.f), 0.0f));
    layout.add(std::make_unique <juce::AudioParameterFloat>("TargetLevel", "Target Level", juce::NormalisableRange<float>(-20.f, +20.f, 1.f, 1.f), 0.0f));
    layout.add(std::make_unique <juce::AudioParameterFloat>("ReferenceLevel", "Reference Level", juce::NormalisableRange<float>(-48.f, +10.f, 1.f, 1.f), -18.0f));
    layout.add(std::make_unique <juce::AudioParameterFloat>("AutoCalibrate", "Auto Calibrate", juce::NormalisableRange<float>(0.f, 1.f, 1.f, 1.f), 0.0f));

    juce::StringArray stringArray;
    stringArray = { "Peak", "RMS", "VU", "LU" };
    layout.add(std::make_unique<juce::AudioParameterChoice>("MeterType", "MeterType", stringArray, 0));

    return layout;
}
*/


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KemomileMeterAudioProcessor();
}

