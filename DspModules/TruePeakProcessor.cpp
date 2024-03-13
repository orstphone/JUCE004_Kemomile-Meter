/*
  ==============================================================================

    TruePeakProcessor.cpp
    Created: 13 Mar 2024 3:54:33pm
    Author:  prmir

  ==============================================================================
*/

#include <JuceHeader.h>
#include "TruePeakProcessor.h"


//==============================================================================
AudioProcessing::TruePeak::TruePeak()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

AudioProcessing::TruePeak::LinearValue AudioProcessing::TruePeak::process(const juce::AudioBuffer<float>& buffer)
{
    return LinearValue();
}

AudioProcessing::TruePeak::~TruePeak()
{
}

void AudioProcessing::TruePeak::paint (juce::Graphics& g)
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
    g.drawText ("TruePeakProcessor", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void AudioProcessing::TruePeak::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

//======================================================================

float AudioProcessing::processTruePeak(const juce::AudioBuffer<float>& buffer, const int bufferSize)
{
    float truePeakValue = 0.0f;

    if (&buffer == nullptr)
    {
        jassert("&audiobuffer is nullptr");
    }

    AudioProcessing::TruePeak truePeak;
    TruePeak::truePeak::LinearValue value = truePeak.process(buffer);


    return 0.0f;
}
