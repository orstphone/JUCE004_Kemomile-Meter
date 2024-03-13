/*
  ==============================================================================

    ComputeLevels.h
    Created: 13 Mar 2024 12:15:23pm
    Author:  prmir

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ComputeLevels  : public juce::Component
{
public:
    ComputeLevels();
    ~ComputeLevels() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComputeLevels)
};
