/*
  ==============================================================================

    HorizontalBarMeter.cpp
    Created: 19 Mar 2024 12:21:19am
    Author:  Decca

  ==============================================================================
*/

#include "HorizontalBarMeter.h"


void Gui::HorizontalBarMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colours::black);
    g.fillRect(bounds);
    g.setColour(juce::Colours::aliceblue);
    //map level from -60.f ~ +3.f to 0 ~ width
    const auto scaledWidthOfBar = juce::jmap(level, -60.f, +3.f, 0.f, static_cast<float>(getWidth()));
    g.fillRect(bounds.removeFromLeft(scaledWidthOfBar));
}

void Gui::HorizontalBarMeter::setLevel(const float value)
{
    level = value;
}
