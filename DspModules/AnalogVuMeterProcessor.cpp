/*
  ==============================================================================

    AnalogVuMeterProcessor.cpp
    Created: 19 Mar 2024 4:06:42pm
    Author:  prmir

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AnalogVuMeterProcessor.h"

//static member constants
const float AnalogVuMeterProcessor::minimalReturnValue = -300.0f;



//==============================================================================
AnalogVuMeterProcessor::AnalogVuMeterProcessor() :
    bufferForMeasurement(2, 2048),
    numberOfBins(0),
    numberOfSamplesPerBin(0),
    numberOfSamplesInAllBins(0),
    measurementDuration(0),
    currentBin(0),
    vuLevels {minimalReturnValue, minimalReturnValue},
    currentBlockIsSilent(false)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    DBG("AnalogVuMeterProcessor Spawned");

    // If this class is used without caution and processBlock
    // is called before prepareToPlay, divisions by zero
    // might occure. E.g. if numberOfSamplesInAllBins = 0.
    //
    // To prevent this, prepareToPlay is called here with
    // some arbitrary arguments.
    prepareToPlay(48000.0, 2, 512, 20);
}

AnalogVuMeterProcessor::~AnalogVuMeterProcessor()
{
}

void AnalogVuMeterProcessor::prepareToPlay(double sampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock, int expectedRequestRate)
{
    spec.sampleRate =sampleRate;
    spec.numChannels = numberOfInputChannels;
    spec.maximumBlockSize = estimatedSamplesPerBlock;
    this->expectedRequestRate = expectedRequestRate;


    bufferForMeasurement.setSize(numberOfInputChannels, estimatedSamplesPerBlock);
    bufferRectifier.prepareToPlay(sampleRate, numberOfInputChannels);

    //determine bins' stuff : omitted




}

void AnalogVuMeterProcessor::processBlock(juce::AudioSampleBuffer& buffer)
{
    bufferForMeasurement = buffer;

    if (currentBlockIsSilent)
    {
        //if the block is silence
        const float silenceThreshold = std::pow(10, 0.1 * -90);
        const float magnitude = buffer.getMagnitude(0, buffer.getNumSamples());

        if (magnitude < silenceThreshold)
        {
            currentBlockIsSilent = true;
            DBG("silence detected");
        }
        else {
            currentBlockIsSilent = false;
        }
    }

    //STEP 1:: rectify and damp the signal transient.
    bufferRectifier.processBlock(bufferForMeasurement);

    //STEP 2:: set the number of channels to prevent EXC_BAD_ACCESS
    //when the number of channels in the buffer suddenly changes w/o calling
    //prepareToPlay() in advance.

    const int numberOfChannels = juce::jmin(
        bufferForMeasurement.getNumChannels(), numberOfChannels);
    
    //STEP 3:: accum. the samples and use the steady state equation.
}

void AnalogVuMeterProcessor::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("AnalogVuMeterProcessor", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void AnalogVuMeterProcessor::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////


AnalogVuMeterRectifier::AnalogVuMeterRectifier() :
    numberOfChannels{ 0 },
    sampleRate{ 48000 }
{
    //determine the values if exists
}

AnalogVuMeterRectifier::~AnalogVuMeterRectifier()
{
}

void AnalogVuMeterRectifier::prepareToPlay(double sampleRate, int numberOfChannels)
{
    DBG("prepareToPlay @ Rectifier called");
    this->sampleRate = sampleRate;
    this->numberOfChannels = numberOfChannels;

    //init. z1 and z2
    z1.calloc(numberOfChannels);

    //determine the filter coeffs if E : there is none unless Tustin.


}

void AnalogVuMeterRectifier::releaseResources()
{
}

void AnalogVuMeterRectifier::processBlock(juce::AudioSampleBuffer& buffer)
{
    const int numOfChannels = juce::jmin(numberOfChannels, buffer.getNumChannels());
    for (int channel = 0; channel < numOfChannels; ++channel)
    {
        float* samples = buffer.getWritePointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float x_ = samples[i];
            float absx_ = abs(x_);
            double weight = capacitance / sampleRate;
            double difference = absx_ - 1 * z1[channel];
            double x_next = weight * difference;

            z1[channel] = difference;
            samples[i] = x_next;
        }
    }
}

void AnalogVuMeterRectifier::reset()
{
    z1.clear(numberOfChannels);
}
