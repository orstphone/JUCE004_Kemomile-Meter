/*
  ==============================================================================

    HorizontalBarMeter.h
    Created: 19 Mar 2024 12:21:19am
    Author:  Decca

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace Gui
{
    class HorizontalBarMeter : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;

        void setLevel(const float value);

    private:
        float level = -60.f;
    };
}