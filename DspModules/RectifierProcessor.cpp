/*
  ==============================================================================

    RectifierProcessor.cpp
    Created: 14 Mar 2024 12:11:30am
    Author:  Decca (https://github.com/deccauvw)

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RectifierProcessor.h"
using namespace DspModules;

//==============================================================================
RectifierProcessor::RectifierProcessor()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

RectifierProcessor::~RectifierProcessor()
{
}

RectifierProcessor::LinearValue DspModules::RectifierProcessor::process(const juce::AudioBuffer<float>& buffer)
{
    int inputBufferNumSamples = buffer.getNumSamples();
    int inputBufferNumChannels = buffer.getNumChannels();

    for (int ch = 0; inputBufferNumChannels; ++ch)
    {
        RectifierProcessor::m_inputs.copyFrom(ch, 0, buffer.getArrayOfReadPointers()[ch], RectifierProcessor::m_inputs.getNumSamples());
    }
    //isResizing necessary here? NO for now

    return DspModules::RectifierProcessor::RectifyAndSmoothout(RectifierProcessor::m_inputs);
}

void DspModules::RectifierProcessor::reset()
{
    m_inputs.setSize(0, 0); //sAR process this buffer
    prev_inputs.setSize(0, 0); //holds previousBuffer
}



RectifierProcessor::LinearValue DspModules::RectifierProcessor::RectifyAndSmoothout(const juce::AudioBuffer<float>& buffer)
{
    LinearValue value;
    int sampleSize = buffer.getNumSamples();

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const float* input = buffer.getArrayOfReadPointers()[ch];
        for (int i = 0; i < sampleSize; ++i)
        {
            float absSample = value.m_channelArray[ch];
            value.m_channelArray[ch] = absSample;
        }
    }
    return value;
}


void DspModules::drawProcessedBuffer(const juce::AudioBuffer<float>& buffer)
{
    DspModules::prev_inputs.makeCopyOf(buffer);

    return;
    //replace input buffer with processed buffer

}





//======================================================================

void RectifierProcessor::paint(juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);   // draw an outline around the component

    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText("RectifierProcessor", getLocalBounds(),
        juce::Justification::centred, true);   // draw some placeholder text
}

void RectifierProcessor::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}