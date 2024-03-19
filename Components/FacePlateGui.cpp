/*
  ==============================================================================

    FacePlateGui.cpp
    Created: 19 Mar 2024 2:04:36am
    Author:  Decca

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FacePlateGui.h"

#include <vector>

//==============================================================================
Gui::FacePlateGui::FacePlateGui()
{
}

Gui::FacePlateGui::~FacePlateGui()
{
}

void Gui::FacePlateGui::loadAssets()
{
     imageFaceplateCanvas = juce::ImageCache::getFromMemory(BinaryData::FaceplateCanvas_png, BinaryData::FaceplateCanvas_pngSize);
     imageGlassSheenReflection = juce::ImageCache::getFromMemory(BinaryData::glassSheen_png, BinaryData::glassSheen_pngSize);
     imageNumericPrintedValuesVu = juce::ImageCache::getFromMemory(BinaryData::text_Numerics_png, BinaryData::text_Numerics_pngSize);
     imageNumericPrintedLr = juce::ImageCache::getFromMemory(BinaryData::text_LR_png, BinaryData::text_LR_pngSize);
     imageNumericPrintedVu = juce::ImageCache::getFromMemory(BinaryData::text_VU_png, BinaryData::text_VU_pngSize);
}



void Gui::FacePlateGui::paint (juce::Graphics& g)
{
}

void Gui::FacePlateGui::paintOverChildren(juce::Graphics& g)
{
}

void Gui::FacePlateGui::resized()
{
}

juce::Image Gui::FacePlateGui::getFaceplate() const
{
    return imageFaceplateCanvas; 
}

juce::Image Gui::FacePlateGui::getChannelInfos() const
{
    return imageNumericPrintedLr;
}

juce::Image Gui::FacePlateGui::getMetricsVu() const
{
    return imageNumericPrintedValuesVu;
}

juce::Image Gui::FacePlateGui::getIconVu() const
{
    return imageNumericPrintedVu;
}
